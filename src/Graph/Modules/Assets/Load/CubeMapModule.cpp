/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CubeMapModule.h"

#include <cinttypes>
#include <filesystem>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Buffer/FrameBuffer.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>

using namespace GaiApi;

#define MAX_THUMBNAIL_HEIGHT 50

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<CubeMapModule> CubeMapModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<CubeMapModule>(vVulkanCore);
    res->SetParentNode(vParentNode);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

CubeMapModule::CubeMapModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;

    unique_OpenPictureFileDialog_id = ct::toStr("OpenCubeMapFacesFileDialog%u", (uintptr_t)this);
}

CubeMapModule::~CubeMapModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool CubeMapModule::Init() {
    ZoneScoped;

    return true;
}

void CubeMapModule::Unit() {
    ZoneScoped;

    ClearTextures();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool CubeMapModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::ContrastedButton("Load Textures", nullptr, nullptr, -1.0f)) {
        IGFD::FileDialogConfig config;
        config.path = m_SelectedFilePath;
        config.filePathName = m_SelectedFilePathName;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog(unique_OpenPictureFileDialog_id, "Open CubeMap Face Texture File",
            "CubeMap Faces Textures 2D Files{([a-zA-Z0-9]+_[0-9]+.((png)|(jpg)|(jpeg)))}",  // pattern file_09.png (png, jpg, jpeg)
            config);
    }

    if (ImGui::ContrastedButton("Reset Textures", nullptr, nullptr, -1.0f)) {
        ClearTextures();
    }

    if (m_TextureCubePtr) {
        ImGui::Text("File path : %s", m_SelectedFilePath.c_str());

        ImGui::Text("File X+ : %s", m_FileNames[0U].c_str());
        ImGui::Text("File X- : %s", m_FileNames[1U].c_str());
        ImGui::Text("File Y+ : %s", m_FileNames[2U].c_str());
        ImGui::Text("File Y- : %s", m_FileNames[3U].c_str());
        ImGui::Text("File Z+ : %s", m_FileNames[4U].c_str());
        ImGui::Text("File Z- : %s", m_FileNames[5U].c_str());

        const float aw = ImGui::GetContentRegionAvail().x;
        DrawTextures(ct::ivec2((int32_t)aw, (int32_t)(aw * 3.0f / 4.0f)));
    }

    return false;
}

bool CubeMapModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool CubeMapModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ImVec2 max = vMaxRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::Instance()->Display(unique_OpenPictureFileDialog_id, ImGuiWindowFlags_NoCollapse, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_SelectedFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            m_SelectedFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            LoadTextures(m_SelectedFilePathName);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE CUBE SLOT OUTPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* CubeMapModule::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    if (m_TextureCubePtr) {
        return &m_TextureCubePtr->m_DescriptorImageInfo;
    }

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
    return corePtr->getEmptyTextureCubeDescriptorImageInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVTE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CubeMapModule::LoadTextures(const std::string& vFilePathName) {
    namespace fs = std::filesystem;

    if (fs::exists(vFilePathName)) {
        bool is_ok = false;
        m_FilePathNames = {};

        auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
        if (ps.isOk) {
            // the last number of first file must be 0
            // the last number of last file must be 5

            if (ps.name.size() > 1) {
                std::string name = ps.name.substr(0U, ps.name.size() - 1U);
                uint32_t num = ct::uvariant(ps.name.substr(ps.name.size() - 1U)).GetU();

                if (num >= 0U && num <= 5U) {
                    is_ok = true;
                    for (uint32_t idx = 0U; idx < 6U; ++idx) {
                        m_FilePathNames[idx] = ct::toStr("%s/%s%u.%s", ps.path.c_str(), name.c_str(), idx, ps.ext.c_str());
                        if (!fs::exists(m_FilePathNames[idx])) {
                            is_ok = false;
                            m_FilePathNames = {};
                            break;
                        }
                    }
                }
            }
        }

        if (is_ok) {
            m_TextureCubePtr = TextureCube::CreateFromFiles(m_VulkanCore, m_FilePathNames);
            if (m_TextureCubePtr) {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);
                for (uint32_t idx = 0U; idx < 6U; ++idx) {
                    m_Texture2Ds[idx] = Texture2D::CreateFromFile(m_VulkanCore, m_FilePathNames[idx], MAX_THUMBNAIL_HEIGHT);
                    if (m_Texture2Ds[idx]) {
                        ps = FileHelper::Instance()->ParsePathFileName(m_FilePathNames[idx]);
                        if (ps.isOk) {
                            m_FileNames[idx] = ps.name;
                        }

                        auto imguiRendererPtr = corePtr->GetVulkanImGuiRenderer().lock();
                        if (imguiRendererPtr) {
                            m_ImGuiTextures[idx].SetDescriptor(
                                imguiRendererPtr, &m_Texture2Ds[idx]->m_DescriptorImageInfo, m_Texture2Ds[idx]->m_Ratio);
                        }
                    }
                }

                auto parentNodePtr = GetParentNode().lock();
                if (parentNodePtr) {
                    parentNodePtr->SendFrontNotification(TextureUpdateDone);
                }
            }
        }
    }
}

void CubeMapModule::ClearTextures() {
    for (auto& tex : m_ImGuiTextures) {
        tex.ClearDescriptor();
    }

    m_TextureCubePtr.reset();

    auto parentNodePtr = GetParentNode().lock();
    if (parentNodePtr) {
        parentNodePtr->SendFrontNotification(TextureUpdateDone);
    }
}

void CubeMapModule::DrawTextures(const ct::ivec2& vMaxRect, const float& vRounding) {
    if (m_TextureCubePtr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        static const uint32_t _x_pos = 0U;
        static const uint32_t _x_neg = 1U;
        static const uint32_t _y_pos = 2U;
        static const uint32_t _y_neg = 3U;
        static const uint32_t _z_pos = 4U;
        static const uint32_t _z_neg = 5U;
        static int32_t faces[3U][4U] = {{-1, _y_pos, -1, -1}, {_x_neg, _z_pos, _x_pos, _z_neg}, {-1, _y_neg, -1, -1}};
        static int32_t roundedCorners[3U][4U] = {{0, ImDrawFlags_RoundCornersTop, 0, 0},
            {ImDrawFlags_RoundCornersLeft, 0, 0, ImDrawFlags_RoundCornersRight}, {0, ImDrawFlags_RoundCornersBottom, 0, 0}};

        const ImVec2 size = ImVec2((float)vMaxRect.x / 4.0f, (float)vMaxRect.y / 3.0f);
        const ImVec2 spos = window->DC.CursorPos;
        const ImRect bb(spos, spos + ImVec2((float)vMaxRect.x, (float)vMaxRect.y));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, 0))
            return;

        const auto borders_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        for (size_t row = 0U; row < 3U; ++row) {
            for (size_t col = 0U; col < 4U; ++col) {
                const auto& idx = faces[row][col];
                if (idx > -1) {
                    if (m_ImGuiTextures[idx].canDisplayPreview) {
                        const ImVec2 pos = ImVec2(spos.x + size.x * col, spos.y + size.y * row);
                        if (vRounding > 0.0f) {
                            window->DrawList->AddImageRounded((ImTextureID)&m_ImGuiTextures[idx].descriptor, pos, pos + size, ImVec2(0, 0),
                                ImVec2(1, 1), borders_color, vRounding, roundedCorners[row][col]);
                        } else {
                            window->DrawList->AddImage((ImTextureID)&m_ImGuiTextures[idx].descriptor, pos, pos + size);
                        }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CubeMapModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<cubemap_module>\n";

    str += vOffset + "\t<file_path_name>" + m_SelectedFilePathName + "</file_path_name>\n";
    str += vOffset + "\t<file_path>" + m_SelectedFilePath + "</file_path>\n";

    str += vOffset + "</cubemap_module>\n";

    return str;
}

bool CubeMapModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "cubemap_module") {
        if (strName == "file_path")
            m_SelectedFilePath = strValue;
        else if (strName == "file_path_name") {
            m_SelectedFilePathName = strValue;
            LoadTextures(m_SelectedFilePathName);
        }
    }

    return true;
}
