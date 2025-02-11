﻿/*
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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Graph/Base/NodeStamp.h>
#include <imgui.h>

NodeStamp::NodeStamp() = default;

NodeStamp::~NodeStamp() = default;

void NodeStamp::DrawImGui() {
    ImGui::Text("type stamp : %s", typeStamp.c_str());  // float(vec3)
    ImGui::Text("name stamp : %s", nameStamp.c_str());  // float map(vec3)
    ImGui::Text("full stamp : %s", fullStamp.c_str());  // float map(vec3 a)
}