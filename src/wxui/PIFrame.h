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

#include <sigslot.h>

class gceEditorFrame;
class gceEntityPackedRef_set;
class gceTypeSchemaPropertyGrid;

class gcePIFrame final : public wxPanel, public sigslot::has_slots
{
public:
    explicit gcePIFrame(gceEditorFrame *parent);

    /*!
     * @brief selection changes slot
     */
    void onSelectionChanged(const int);

private:

    void SetRecordProperties(gceTypeSchemaPropertyGrid *pg);

    void OnBnApply(wxCommandEvent &event);
    void OnBnApplyUUI(wxUpdateUIEvent &event);

    gceTypeSchemaPropertyGrid *m_recProp = nullptr;
    gceEditorFrame *const m_Editor;
};


