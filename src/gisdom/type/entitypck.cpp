// Copyright 2026 Sergei Pikin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "gcprec.h"
#include "entitypck.h"
#include "entity.h"
#include "io/wkbio.h"

constexpr size_t gceEntityBufferHeaderSize = 4;


namespace
{

struct get_packed_size2
{
    const gceColumnValue &m_val;

    template <class T> size_t operator()()
    {
        if (std::holds_alternative<T>(m_val))
        {
            if constexpr (gce::is_any_v<T, int16_t, int32_t, int64_t, float, double>)
            {
                return sizeof(T);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return std::get<T>(m_val).size();
            }
            else if constexpr (std::is_same_v<T, std::unique_ptr<geom::Geometry>>)
            {
                return geom::WKBwriter::sizeGeometry(std::get<T>(m_val).get());
            }
        }
        return 0;
    }
};

struct pack_unchecked2
{
    std::byte *dest = nullptr;
    const gceColumnValue &m_val;

    template <class T> void operator()()
    {
        if (std::holds_alternative<T>(m_val))
        {
            auto &arg = std::get<T>(m_val);
            if constexpr (is_column_arithmetic_v<T>)
            {
                gce::write_be(dest, arg);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                std::memcpy(dest, arg.data(), arg.size());
            }
            else if constexpr (std::is_same_v<T, std::unique_ptr<geom::Geometry>>)
            {
                if (arg)
                {
                    geom::WKBwriter().write((char *)dest, *arg);
                }
            }
        }
    }
};
struct get_value2
{
    gce::span<const std::byte> src;
    geom::CoordinateType cooType;

    template <class T> gceColumnValue operator()()
    {
        gceColumnValue val;
        if constexpr (is_column_arithmetic_v<T>)
        {
            if (src.size() == sizeof(T))
            {
                val = gce::read_be<T>(src.data());
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            val = std::string(reinterpret_cast<const char *>(src.data()), src.size());
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<geom::Geometry>>)
        {
            if (!src.empty())
            {
                val = geom::WKBreader().readGeometry(cooType, reinterpret_cast<const uint8_t *>(src.data()), src.size());
            }
        }
        return val;
    }
};

struct get_string2
{
    gce::span<const std::byte> src;

    template <class T> std::string operator()()
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return std::string(reinterpret_cast<const char *>(src.data()), src.size());
        }
        else
        {
            if constexpr (is_column_arithmetic_v<T>)
            {
                if (src.size() == sizeof(T))
                {
                    return fmt::format("{}", gce::read_be<T>(src.data()));
                }
            }
            else if constexpr (std::is_same_v<T, std::unique_ptr<geom::Geometry>>)
            {
                if (!src.empty())
                {
                    return std::string("geometry");
                }
            }
            return std::string("null");
        }
    }
};

void write_ep_offsetsize_and_cols(std::byte **ptr, uint8_t offsetSize, uint8_t numCol)
{
    *((*ptr)++) = static_cast<std::byte>('e');
    *((*ptr)++) = static_cast<std::byte>('p');
    *((*ptr)++) = static_cast<std::byte>(offsetSize);
    *((*ptr)++) = static_cast<std::byte>(numCol);
}

uint8_t calculateIndexBytes(size_t ncol, size_t dataSize)
{
    uint8_t res = 1;
    while ((gceEntityBufferHeaderSize + ncol * res + dataSize) > (size_t(1) << (8 * res)))
    {
        res++;
    }
    return res;
}

void write_offset_be1(std::byte *ptr, size_t val, uint8_t size)
{
    auto *ptr2 = ptr + size;
    do
    {
        *(--ptr2) = static_cast<std::byte>(val & 0xFF);
        val = val >> 8;
    }
    while (ptr != ptr2);
}

size_t read_offset_be1(const std::byte *ptr, uint8_t size)
{
    size_t res = static_cast<size_t>(*ptr);
    auto *ptr2 = ptr + size;
    while (++ptr != ptr2)
    {
        res = (res << 8) + static_cast<size_t>(*ptr);
    }
    return res;
}

template <class T> GCE_WITH_REQUIRES(is_column_arithmetic_v<T>)
static bool less_val1(const gceEntityPacked &A, const gceEntityPacked &B, uint8_t index)
{
    static_assert(is_column_arithmetic_v<T>, "");
    return A.get_arithmetic_unchecked<T>(index) < B.get_arithmetic_unchecked<T>(index);
}

} // end of anonimous namespace

gceEntityPacked::gceEntityPacked(const gceEntityVar &other) : m_schema(other.schema)
{
    const size_t numCol = m_schema->getColumns().size();
    // allocation size
    std::vector<uint32_t> sizes(numCol);
    size_t dataSize = 0;
    for (auto &col : m_schema->getColumns())
    {
        uint8_t index = col.getIndex();
        size_t sz = gce::visit_column(get_packed_size2{other[index]}, col.getType());
        sizes[index] = sz;
        dataSize += sz;
    }

    uint8_t nbytes = calculateIndexBytes(numCol + 1, dataSize);
    const size_t headerSize = gceEntityBufferHeaderSize + (numCol + 1) * nbytes;

    m_buffer.resize(headerSize + dataSize);

    std::byte *start = m_buffer.data();

    write_ep_offsetsize_and_cols(&start, nbytes, static_cast<uint8_t>(numCol));
    uint32_t offset = headerSize;
    for (auto &val : sizes)
    {
        write_offset_be1(start, offset, nbytes);
        offset += val;
        start += nbytes;
    }
    write_offset_be1(start, offset, nbytes);
    start += nbytes;

    for (auto &col : m_schema->getColumns())
    {
        uint8_t index = col.getIndex();
        if (sizes[index] > 0)
        {
            gce::visit_column(pack_unchecked2{start, other[index]}, col.getType());
            start += sizes[index];
        }
    }
}

void gceEntityPacked::assign(const gceTypeSchema *schema, gce::span<std::byte> *data)
{
    m_schema = schema;
    const size_t numCol = m_schema->getColumns().size();
    const auto sdata = gce::span<gce::span<std::byte>>{data, numCol};
    size_t dataSize = 0;
    for (auto &val : sdata)
    {
        dataSize += val.size();
    }
    uint8_t nbytes = calculateIndexBytes(numCol + 1, dataSize);
    const size_t headerSize = gceEntityBufferHeaderSize + (numCol + 1) * nbytes;

    m_buffer.resize(headerSize + dataSize);

    std::byte *start = m_buffer.data();

    write_ep_offsetsize_and_cols(&start, nbytes, static_cast<uint8_t>(numCol));
    uint32_t offset = headerSize;
    for (auto &val : sdata)
    {
        write_offset_be1(start, offset, nbytes);
        offset += val.size();
        start += nbytes;
    }
    write_offset_be1(start, offset, nbytes);
    start += nbytes;

    for (auto &val : sdata)
    {
        std::memcpy(start, val.data(), val.size());
        start += val.size();
    }
}

std::unique_ptr<geom::Geometry> gceEntityPacked::get_geometry_unchecked(uint8_t index) const
{
    auto src = get_data(index);
    auto &col = m_schema->at(index);
    geom::WKBreader r;
    return r.readGeometry(col.getCoordinateType(), reinterpret_cast<const uint8_t *>(src.data()), src.size());
}

geom::GEOSGeom_scoped_t gceEntityPacked::get_geometry_GEOS(uint8_t index) const
{
    struct GEOSWKBReader_destroyer
    {
        void operator()(GEOSWKBReader *o) { GEOSWKBReader_destroy(o); }
    };

    auto wkb = this->get_data(index);
    if (std::unique_ptr<GEOSWKBReader, GEOSWKBReader_destroyer> wr{GEOSWKBReader_create()}; wr)
    {
        return geom::GEOSGeom_scoped_t{GEOSWKBReader_read(wr.get(), (const unsigned char *)wkb.data(), wkb.size())};
    }
    return {};
}

gce::span<const std::byte> gceEntityPacked::get_data(uint8_t index) const
{
    auto *data = m_buffer.data();
    uint8_t nbytes = static_cast<uint8_t>(data[2]);
    auto *offsetPtr = data + gceEntityBufferHeaderSize + index * nbytes;
    return gce::span<const std::byte>{data + read_offset_be1(offsetPtr, nbytes), data + read_offset_be1(offsetPtr + nbytes, nbytes)};
}

template <class T> GCE_WITH_REQUIRES(is_column_arithmetic_v<T>)
inline gceColumnValue get_or_monostate(gce::span<const std::byte> &&r)
{
    static_assert(is_column_arithmetic_v<T>, "");
    if (r.size() == sizeof(T))
    {
        return gce::read_be<T>(r.data());
    }
    return {};
}

gceColumnValue gceEntityPacked::getValue(uint8_t index) const
{
    auto &col = m_schema->at(index);
    return gce::visit_column(get_value2{get_data(index), col.getCoordinateType()}, col.getType());
}

std::string gceEntityPacked::to_string() const
{
    return gceEntityVar{*this}.to_string();
}

std::string gceEntityPacked::pkey_string() const
{
    uint8_t index = m_schema->getKeyIndex();
    return gce::visit_column(get_string2{get_data(index)}, getType(index));
}

template < typename T, typename dummy = T >
struct remap_rey
{
    remap_rey(const T &) {}
    operator gceEntityKey() const
    {
        throw std::runtime_error("unsupported key type");
    }
};

template < typename T >
struct remap_rey <T, std::enable_if_t<boost::mp11::mp_contains<gceEntityKey, T>::value, T >>
{
    remap_rey(const T &v) : v(v) {}
    operator gceEntityKey() const
    {
        return v;
    }
    T v;
};

template <class T>  //requires(is_column_arithmetic_v)
std::optional<T> getValueImpl(const gce::span<const std::byte> &src)
{
    if constexpr (std::is_same_v < T, std::string>)
    {
        return {std::string(reinterpret_cast<const char *>(src.data()), src.size())};
    }
    else
    {
        if constexpr (is_column_arithmetic_v<T>)
        {
            if (src.size() == sizeof(T))
            {
                return gce::read_be<T>(src.data());
            }
        }
        return {};
    }

}
struct get_pkey2
{
    const gceEntityPacked &m_ep;
    template <class T> gceEntityKey operator()() const
    {
        auto &col = m_ep.get_schema()->getKeyColumn();
        auto src = m_ep.get_data(col.getIndex());
        return remap_rey(getValueImpl<T>(src).value_or(T{}));
    }
};
gceEntityKey gceEntityPacked::get_pkey() const
{
    auto &col = m_schema->getKeyColumn();
    auto src = get_data(col.getIndex());
    return gce::visit_column(get_pkey2{*this}, col.getType());
}

bool gceEntityPacked::equals(const gceEntityPacked &other, uint8_t index) const
{
    if (empty() || other.empty() || m_schema->getId() != other.m_schema->getId())
    {
        return false;
    }

    auto r1 = this->get_data(index);
    auto r2 = other.get_data(index);
    return std::equal(r1.begin(), r1.end(), r2.begin(), r2.end());
}

bool gceEntityPacked::hasValue(uint8_t index) const
{
    return !empty() && !get_data(index).empty();
}
