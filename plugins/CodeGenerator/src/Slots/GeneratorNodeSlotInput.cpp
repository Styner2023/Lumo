#include "GeneratorNodeSlotInput.h"

#include <LumoBackend/Graph/Base/BaseNode.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

GeneratorNodeSlotInputPtr GeneratorNodeSlotInput::Create() {
    auto res = std::make_shared<GeneratorNodeSlotInput>();
    res->m_This = res;
    return res;
}

GeneratorNodeSlotInputPtr GeneratorNodeSlotInput::Create(const std::string& vName) {
    auto res = std::make_shared<GeneratorNodeSlotInput>(vName);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

GeneratorNodeSlotInput::GeneratorNodeSlotInput() : NodeSlotInput() {
    pinID = sGetNewSlotId();
    slotPlace = NodeSlot::PlaceEnum::INPUT;
}

GeneratorNodeSlotInput::GeneratorNodeSlotInput(const std::string& vName) : NodeSlotInput(vName) {
    pinID = sGetNewSlotId();
    slotPlace = NodeSlot::PlaceEnum::INPUT;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNodeSlotInput::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string res;

    res += vOffset + ct::toStr(
                         "<slot index=\"%u\" name=\"%s\" type=\"%s\" place=\"%s\" id=\"%u\" hideName=\"%s\" typeIndex=\"%u\" subTypeIndex=\"%u\" "
                         "bindingIndex=\"%u\" showWidget=\"%s\"/>\n",
                         index, name.c_str(), slotType.c_str(), NodeSlot::sGetStringFromNodeSlotPlaceEnum(slotPlace).c_str(), (uint32_t)GetSlotID(),
                         hideName ? "true" : "false", editorSlotTypeIndex, editorSlotSubTypeIndex, descriptorBinding, showWidget ? "true" : "false");

    return res;
}

bool GeneratorNodeSlotInput::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strName == "slot" && strParentName == "node") {
        uint32_t _index = 0U;
        std::string _name;
        std::string _type = "NONE";
        auto _place = NodeSlot::PlaceEnum::NONE;
        uint32_t _pinId = 0U;
        bool _hideName = false;

        for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
            std::string attName = attr->Name();
            std::string attValue = attr->Value();

            if (attName == "index")
                _index = ct::ivariant(attValue).GetU();
            else if (attName == "name")
                _name = attValue;
            else if (attName == "type")
                _type = attValue;
            else if (attName == "place")
                _place = NodeSlot::sGetNodeSlotPlaceEnumFromString(attValue);
            else if (attName == "id")
                _pinId = ct::ivariant(attValue).GetU();
            else if (attName == "hideName")
                _hideName = ct::ivariant(attValue).GetB();
        }

        if (index == _index && slotType == _type && slotPlace == _place && !idAlreadySetbyXml) {
            pinID = _pinId;
            idAlreadySetbyXml = true;

            // pour eviter que des slots aient le meme id qu'un nodePtr
            BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, (uint32_t)GetSlotID());

            return false;
        }
    }

    return true;
}