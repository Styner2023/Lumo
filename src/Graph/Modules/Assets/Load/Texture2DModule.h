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

#pragma once

#include <functional>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Gaia/Resources/Texture2D.h>
#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class Texture2DModule : public conf::ConfigAbstract, public GuiInterface, public NodeInterface, public Texture2DOutputInterface {
public:
    static std::shared_ptr<Texture2DModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<Texture2DModule> m_This;
    std::string m_FilePathName;
    std::string m_FilePath;
    std::string m_FileName;
    std::string unique_OpenPictureFileDialog_id;
    Texture2DPtr m_Texture2DPtr = nullptr;
    ImGuiTexture m_ImGuiTexture;
    GaiApi::VulkanCoreWeak m_VulkanCore;

public:
    Texture2DModule(GaiApi::VulkanCoreWeak vVulkanCore);
    virtual ~Texture2DModule();

    bool Init();
    void Unit();

    std::string GetFileName() {
        return m_FileName;
    }

    void NeedResize(ct::ivec2* vNewSize);

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void DrawTexture(ct::ivec2 vMaxRect);

    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;

private:
    void LoadTexture2D(const std::string& vFilePathName);

public:
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};