/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

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

#include <set>
#include <array>
#include <string>
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>

class MathModule_Quad_Pass : public QuadShaderPass,

                             public Texture2DInputInterface<3U>,
                             public Texture2DOutputInterface,
                             public NodeInterface {
private:
    VulkanBufferObjectPtr m_UBOFragPtr = nullptr;
    vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

    struct UBOFrag {
        alignas(4) int32_t u_Function_index = 0;
        alignas(4) float u_use_input_0 = 0.0f;
        alignas(4) float u_use_input_1 = 0.0f;
        alignas(4) float u_use_input_2 = 0.0f;
    } m_UBOFrag;

    typedef std::tuple<std::string, uint32_t, std::string, std::string, std::string> MathModuleEntry;

    std::vector<MathModuleEntry> m_Functions;

public:
    MathModule_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    virtual ~MathModule_Quad_Pass() override;

    void ActionBeforeInit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

    bool DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr) override;
    uint32_t GetComponentCount();
    std::string GetInputName(const uint32_t& vIdx);

protected:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;

    bool DrawCombo(const float vWidth);
    void ResizeToMaxOfTexturesIfNeeded();
};