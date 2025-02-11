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
#pragma warning(disable : 4251)

#include <map>
#include <LumoBackend/Graph/Graph.h>
#include <Gaia/gaia.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

//todo : split the interface in one input interface and one output interface
// can clarify some code who jsut need output and not input 
class LUMO_BACKEND_API TextureCubeOutputInterface
{
public:
	virtual vk::DescriptorImageInfo* GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) = 0;
};