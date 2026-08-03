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
#pragma once
#include <memory>
#include <limits>

namespace gce
{

/*!
 * @brief index only allocator
 * @tparam I index type
 */
template <class I = uint32_t> class idxpool
{
public:
    using size_type = I;
    static constexpr size_type index_max = std::numeric_limits<size_type>::max();

    /*!
    * constructor. allocates memory for index only
    *
    * @param size maximum elements of type X
    */
    explicit idxpool(size_type size) : m_size(size)
    {
#if has_cpp20
        m_freeI = std::make_unique_for_overwrite<I[]>(size);
#else
        m_freeI = std::make_unique<I[]>(size);
#endif

        for (size_type i = 0; i < size; i++)
        {
            m_freeI[i] = i;
        }
    }

    /*!
     * @brief check if index is valid
     * @param id index to validate
     * @return true if id is within valid range [0, size), false otherwise
     */
    bool isId(size_type id) const
    {
        return id < m_size;
    }

    /*!
     * @brief get number of free indices available for allocation
     * @return count of unallocated indices
     */
    size_type freeCount() const
    {
        return m_size - m_ptr;
    }

    size_type useCount() const
    {
        return m_ptr;
    }

    /*!
     * @brief allocate element
     * @param ind [out] allocated index or idxpool::index_max
     * @return true if new index allocated or false overwise
     */
    bool newX(size_type &ind)
    {
        if (m_ptr != m_size)
        {
            ind = m_freeI[m_ptr];
            m_ptr++;
            return true;
        }
        ind = index_max;
        return false;
    }

    /*!
    * remove element, set index to invalid value
    *
    * @param[inout] ind  element index
    */
    void delX(size_type &ind)
    {
        if (this->isId(ind))
        {
            m_ptr--;
            m_freeI[m_ptr] = ind;
            ind = index_max;
        }
    }
    /*!
     * @brief get total pool size
     * @return maximum number of elements that can be allocated
     */
    I size() const
    {
        return m_size;
    }
private:
    std::unique_ptr<I[]> m_freeI;
    I m_ptr = 0;
    I m_size;
};

/*!
 * @brief indexed block allocator, zero index/element is reserved
 * @tparam X stored data type
 * @tparam I index type
 */
template <class T, class I = uint32_t, class Deleter = std::default_delete<T[]>>
class idxalloca : public idxpool<I>
{
    static constexpr bool is_default_deleter = std::is_same_v<Deleter, std::default_delete<T[]>>;
public:
    using size_type = I;

    /*!
    * constructor. allocates memory for data
    *
    * @param size maximum elements of type X
    */
    explicit idxalloca(size_type size) GCE_WITH_REQUIRES(is_default_deleter) : idxpool<I>(size),
        array(std::make_unique<T[]>(size))
    {
        static_assert(is_default_deleter, "idxalloca requires default deleter for automatic memory management");
    }

    /*!
    * constructor. uses existing memory for data
    *
    * @param size maximum elements of type X
    * @param Xptr   data pointer
    */
    idxalloca(size_type size, T *ptr) GCE_WITH_REQUIRES(!is_default_deleter) : idxpool<I>(size),
        array(ptr)
    {
        static_assert(!is_default_deleter, "idxalloca requires custom deleter for external memory management");
    }

    ~idxalloca() = default;

    /*!
     * @brief get pointer to element at allocated index
     * @param ind element index
     * @return pointer to element at index if valid, nullptr otherwise
     */
    T *getX(size_type ind)
    {
        return (ind < this->size()) ? array.get() + ind : nullptr;
    }
private:
    std::unique_ptr<T[], Deleter> array;
};


}
