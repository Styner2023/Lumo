// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <LumoBackend/Interfaces/TextureCubeInputInterface.h>

#include <memory>
#include <ctools/cTools.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>
#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Interfaces/TextureCubeOutputInterface.h>

void TextureCubeInputFunctions::UpdateTextureCubeInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotInputPtr>& vInputs) {
    ct::fvec2 texSize;
    for (const auto& input : vInputs) {
        if (input.second && input.second->slotType == "TEXTURE_CUBE") {
            for (auto slot : input.second->linkedSlots) {
                auto otherSLotPtr = slot.lock();
                if (otherSLotPtr) {
                    if (otherSLotPtr->slotType == "TEXTURE_CUBE") {
                        auto otherParentPtr = otherSLotPtr->parentNode.lock();
                        if (otherParentPtr) {
                            auto otherNodePtr = dynamic_pointer_cast<TextureCubeOutputInterface>(otherParentPtr);
                            if (otherNodePtr) {
                                auto descPtr = otherNodePtr->GetTextureCube(otherSLotPtr->descriptorBinding, &texSize);
                                SetTextureCube(input.second->descriptorBinding, descPtr, &texSize);
                            }
                        }
                    }
                }
            }
        }
    }
}
