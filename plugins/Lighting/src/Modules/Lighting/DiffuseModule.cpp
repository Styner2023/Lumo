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

#include "DiffuseModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/Lighting/Pass/DiffuseModule_Comp_Pass.h>

using namespace GaiApi;

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

std::shared_ptr<DiffuseModule> DiffuseModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<DiffuseModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

DiffuseModule::DiffuseModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

DiffuseModule::~DiffuseModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DiffuseModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = false;

    if (BaseRenderer::InitCompute2D(map_size)) {
        m_DiffuseModule_Comp_Pass_Ptr = std::make_shared<DiffuseModule_Comp_Pass>(m_VulkanCore);
        if (m_DiffuseModule_Comp_Pass_Ptr) {
            if (m_DiffuseModule_Comp_Pass_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
                AddGenericPass(m_DiffuseModule_Comp_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DiffuseModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("Diffuse", vCmd);

    return true;
}

bool DiffuseModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("Diffuse", -1.0f, true, true, &m_CanWeRender)) {
            bool change = false;

            if (m_DiffuseModule_Comp_Pass_Ptr) {
                change |= m_DiffuseModule_Comp_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
            }

            return change;
        }
    }

    return false;
}

bool DiffuseModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool DiffuseModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void DiffuseModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void DiffuseModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_DiffuseModule_Comp_Pass_Ptr) {
        m_DiffuseModule_Comp_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

vk::DescriptorImageInfo* DiffuseModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_DiffuseModule_Comp_Pass_Ptr) {
        return m_DiffuseModule_Comp_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

void DiffuseModule::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    if (m_DiffuseModule_Comp_Pass_Ptr) {
        return m_DiffuseModule_Comp_Pass_Ptr->SetLightGroup(vSceneLightGroup);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DiffuseModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<diffuse_module>\n";

    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

    if (m_DiffuseModule_Comp_Pass_Ptr) {
        str += m_DiffuseModule_Comp_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }

    str += vOffset + "</diffuse_module>\n";

    return str;
}

bool DiffuseModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "diffuse_module") {
        if (strName == "can_we_render")
            m_CanWeRender = ct::ivariant(strValue).GetB();
    }

    if (m_DiffuseModule_Comp_Pass_Ptr) {
        m_DiffuseModule_Comp_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}