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

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <LumoBackend/Graph/Graph.h>
#include <ImGuiPack.h>
#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class SceneTreePane : public AbstractPane {
public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:  // singleton
    static std::shared_ptr<SceneTreePane> Instance() {
        static auto _instance = std::make_shared<SceneTreePane>();
        return _instance;
    }

public:
    SceneTreePane();                            // Prevent construction
    SceneTreePane(const SceneTreePane&) = default;  // Prevent construction by copying
    SceneTreePane& operator=(const SceneTreePane&) {
        return *this;
    };                     // Prevent assignment
    virtual ~SceneTreePane();  // Prevent unwanted destruction};
};
