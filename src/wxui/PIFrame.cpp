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

#include "PIFrame.h"
#include <wx/propgrid/propgrid.h>

#include "editorfrm.h"
#include "type/entity.h"

namespace
{
struct gceColumnToWxVariant2
{
    gce::span<const std::byte> src;

    template <class T> wxVariant operator()()
    {
        wxVariant val;
        if constexpr (gce::is_any_v<T, int16_t, int32_t, int64_t, float, double>)
        {
            if (src.size() == sizeof(T))
            {
                T a = gce::read_be<T>(src.data());
                if constexpr (gce::is_any_v<T, int16_t, int32_t>)
                {
                    val = long(a);
                }
                else if constexpr (gce::is_any_v<T, int64_t>)
                {
                    val = wxLongLong(a);
                }
                else if constexpr (gce::is_any_v<T, float, double>)
                {
                    val = double(a);
                }
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            val = to_wxstring(std::string(reinterpret_cast<const char *>(src.data()), src.size()));
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<geom::Geometry>>)
        {
            val = wxString("Geometry");
        }
        return val;
    }
};
}

class gceTypeSchemaPropertyGrid final : public wxPropertyGrid
{
public:
    gceTypeSchemaPropertyGrid(gcePIFrame *parent, const gceTypeSchema *schema, gceEntityPackedRef_set &refa) : m_schema(schema), m_refa(refa)
    {
        wxPropertyGrid::Create(parent, -1, wxDefaultPosition, wxDefaultSize, wxPG_SPLITTER_AUTO_CENTER);
        this->CreatePropertyGrid();

        this->Bind(wxEVT_PG_CHANGED, &gceTypeSchemaPropertyGrid::OnPropertyGridChange, this, this->GetId());
    }

    bool TransferToWindow()
    {
        if (m_refa.empty())
        {
            return false;
        }
        for (const gceColumnSchema &col : m_schema->getColumns())
        {
            if (!col.hasFlags(gceColumnSchema::F_PROPERTY))
            {
                continue;
            }

            if (auto *prop = this->GetProperty(to_wxstring(col.getName())); prop)
            {
                // set property value
                wxString hint;
                if (uint8_t index = col.getIndex(); m_refa.IsSameColumnValue(index))
                {
                    auto &en = m_refa.begin()->entity;
                    auto val = gce::visit_column(gceColumnToWxVariant2{en.get_data(index)}, col.getType());
                    if (val.IsNull())
                    {
                        this->SetPropertyValueUnspecified(prop);
                        hint = "[NULL]";
                    }
                    else
                    {
                        this->SetPropertyValue(prop, val);
                    }
                }
                else
                {
                    this->SetPropertyValueUnspecified(prop);
                    hint = "[multiple attribute value]";
                }
                this->SetPropertyAttribute(prop, wxPG_ATTR_HINT, hint);
            }
        }
        this->ClearModifiedStatus();
        this->ClearSelection();
        this->RefreshGrid();

        ProcessEnableUI();

        return true;
    }

    bool TransferFromWindow(gceCmdProcessor &proc)
    {
        if (!m_refa.empty())
        {
            this->CommitChangesFromEditor();

            if (auto first = m_refa.begin(); first->isTableRow())
            {
                gceCommandGroup cmds("Update");
                for (auto &ref : m_refa)
                {
                    gceEntityVar newEntity{ref.entity};
                    DoTransferFromWindow(newEntity);
                    cmds.Update(ref.id_table, gceEntityPacked{newEntity}, ref.entity);
                }
                return proc.postCommandGroup(cmds);
            }
            else
            {
                gceEntityVar en{first->entity};
                DoTransferFromWindow(en);
                m_refa.Replace({{}, gceEntityPacked{en}}, true);
                m_refa.signalSelection();
                return true;
            }
        }
        return false;

    }

    bool IsModified() const
    {
        return this->IsAnyModified();
    }

    const gceTypeSchema *getSchema() const
    {
        return m_schema;
    }

private:
    enum { ID_PG };

    void OnPropertyGridChange(wxPropertyGridEvent &)
    {
        ProcessEnableUI();
    }
    void ProcessEnableUI()
    {
        //m_typeFactory.updateRecordPropUI(this, to_string(getCurrentTemplate()));
    }

    /*!
     * @brief create and populate propertyGrid
     */
    void CreatePropertyGrid()
    {
        for (auto &col : m_schema->getColumns())
        {
            if (!col.hasFlags(gceColumnSchema::F_PROPERTY))
            {
                continue;
            }
            wxPGProperty *prop = nullptr;
            if (col.getControlName().empty())
            {
                if (isInteger(col.getType()))
                {
                    prop = new wxIntProperty(col.getLabel(), col.getName());
                }
                else if (isFloat(col.getType()))
                {
                    prop = new wxFloatProperty(col.getLabel(), col.getName());
                }
                else if (isString(col.getType()))
                {
                    prop = new wxStringProperty(col.getLabel(), col.getName());
                }
                if (prop)
                {
                    this->Append(prop);
                }
            }
        }
        this->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, (long)1);
    }

    bool DoTransferFromWindow(gceEntityVar &en)
    {
        for (auto &col : m_schema->getColumns())
        {
            if (!col.hasFlags(gceColumnSchema::F_PROPERTY))
            {
                continue;
            }

            wxPGProperty *const prop = this->GetProperty(to_wxstring(col.getName()));
            if (prop && !this->IsPropertyValueUnspecified(prop))
            {
                wxAny val = this->GetPropertyValue(prop);
                auto &var = en[col.getIndex()];

                switch (col.getType())
                {
                case gceColumnType::int16:
                    var = static_cast<int16_t>(val.As<long>());
                    break;
                case gceColumnType::int32:
                    var = static_cast<int32_t>(val.As<long>());
                    break;
                case gceColumnType::int64:
                    var = static_cast<int64_t>(val.As<wxLongLong>().GetValue());
                    break;
                case gceColumnType::float32:
                    var = static_cast<float>(val.As<double>());
                    break;
                case gceColumnType::float64:
                    var = val.As<double>();
                    break;
                case gceColumnType::string:
                    var = to_string(val.As<wxString>());
                    break;
                default: // geometry
                    break;
                }
            }
        }
        return true;
    }

    const gceTypeSchema *m_schema = nullptr;
    gceEntityPackedRef_set &m_refa;
};

// gcePIFrame implementation
// 
gcePIFrame::gcePIFrame(gceEditorFrame *parent) : wxPanel(parent), m_Editor(parent)
{
    SetMinSize(wxSize(300, 200));
    // set style
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS | GetExtraStyle());

    wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    // buttons
    auto *buttsizer = new wxStdDialogButtonSizer;
    buttsizer->AddButton(new wxButton(this, wxID_APPLY));
    buttsizer->Realize();

    topSizer->Add(buttsizer, 0, wxEXPAND | wxALL, 5);

    wxEvtHandler::Bind(wxEVT_COMMAND_BUTTON_CLICKED, &gcePIFrame::OnBnApply, this, wxID_APPLY);

    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gcePIFrame::OnBnApplyUUI, this, wxID_APPLY);

    m_Editor->getSelection().selectionChanged.connect(this, &gcePIFrame::onSelectionChanged);
}

void gcePIFrame::OnBnApply(wxCommandEvent &)
{
    if (!m_recProp) return;

    if (!m_recProp->TransferFromWindow(this->m_Editor->getActionProcessor()))
    {
        ::wxMessageBox("Error applying properties", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_EXCLAMATION);
    }
}

void gcePIFrame::OnBnApplyUUI(wxUpdateUIEvent &event)
{
    event.Enable(m_recProp && m_recProp->IsModified());
}

void gcePIFrame::SetRecordProperties(gceTypeSchemaPropertyGrid *pg)
{
    // remove current if any
    if (m_recProp)
    {
        GetSizer()->Detach(m_recProp);
        m_recProp->Destroy();
    }

    // set new
    if (pg)
    {
        GetSizer()->Insert(0, pg, 1, wxGROW | wxALL, 5);
        pg->Show(true);
    }


    m_recProp = pg;

    Layout();

}

// signal processors
void gcePIFrame::onSelectionChanged(const int)
{
    if (auto &sel = m_Editor->getSelection(); !sel.empty() && sel.IsSameType())
    {
        auto first = sel.begin();

        // try to use existing property grid or create new
        if (m_recProp && (m_recProp->getSchema()->getId() == first->entity.get_schema()->getId()))
        {
            m_recProp->TransferToWindow();
        }
        else
        {
            auto rp = new gceTypeSchemaPropertyGrid(this, first->entity.get_schema(), sel);
            rp->TransferToWindow();
            SetRecordProperties(rp);
        }
    }
    else if (m_recProp)
    {
        SetRecordProperties(nullptr);
    }
}
