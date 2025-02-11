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

#include "DebugPane.h"
#include <Frontend/MainFrontend.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <Project/ProjectFile.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <imgui_internal.h>
#include <ImGuiFileDialog.h>

#include <cinttypes>  // printf zu

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

DebugPane::DebugPane() = default;
DebugPane::~DebugPane() {
    Unit();
}

bool DebugPane::Init() {
    return true;
}

void DebugPane::Unit() {
    m_NodeToDebug.reset();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool DebugPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);

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
                auto nodePtr = m_NodeToDebug.lock();
                if (nodePtr) {
                    nodePtr->DrawDebugInfos(nullptr);
                }
            }
        }

        ImGui::End();
    }

    return false;
}

bool DebugPane::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    if (ProjectFile::Instance()->IsProjectLoaded()) {
        /*ImVec2 maxSize = MainFrame::Instance()->m_DisplaySize;
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
        }*/
    }
    return false;
}

bool DebugPane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    return false;
}

bool DebugPane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

void DebugPane::Select(BaseNodeWeak vNode) {
    m_NodeToDebug = vNode;
}
