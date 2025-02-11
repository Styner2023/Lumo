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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Interfaces/VariableInputInterface.h>

class ModelExporterModule;
class ModelExporterNode : public ModelInputInterface, public ModelOutputInterface, public VariableInputInterface<0U>, public BaseNode {
public:
    static std::shared_ptr<ModelExporterNode> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::shared_ptr<ModelExporterModule> m_ModelExporterModulePtr = nullptr;

public:
    ModelExporterNode();
    virtual ~ModelExporterNode() override;

    // Init / Unit
    bool Init(GaiApi::VulkanCoreWeak vVulkanCore) override;

    //  Task
    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;

    // Draw Widgets
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;

    // Interfaces Setters
    void SetModel(SceneModelWeak vSceneModel) override;
    void SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable, void* vUserDatas) override;

    // Interfaces Getters
    SceneModelWeak GetModel() override;

    // Configuration
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void AfterNodeXmlLoading() override;
};
