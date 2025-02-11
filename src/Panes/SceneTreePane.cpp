/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SceneTreePane.h"
#include <ImWidgets.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Project/ProjectFile.h>

#include <cinttypes>  // printf zu

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

SceneTreePane::SceneTreePane() = default;
SceneTreePane::~SceneTreePane() = default;

bool SceneTreePane::Init() {
    ZoneScoped;

    return true;
}

void SceneTreePane::Unit() {
    ZoneScoped;
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool SceneTreePane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    if (vOpened && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
            if (ProjectFile::Instance()->IsProjectLoaded()) {
                // SourcePane_WidgetId = SceneManager::Instance()->DrawSceneTree(SourcePane_WidgetId);
            }
        }

        ImGui::End();
    }

    return false;
}

bool SceneTreePane::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    if (ProjectFile::Instance()->IsProjectLoaded()) {
        /*
        ImVec2 maxSize = MainFrame::Instance()->m_DisplaySize;
        ImVec2 minSize = maxSize * 0.5f;
        if (ImGuiFileDialog::Instance()->Display("OpenShaderCode",
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking,
            minSize, maxSize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
                auto code = FileHelper::Instance()->LoadFileToString(filePathName);
                SetCode(code);
            }

            ImGuiFileDialog::Instance()->Close();
        }
        */
    }

    return false;
}

bool SceneTreePane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    return false;
}

bool SceneTreePane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}
