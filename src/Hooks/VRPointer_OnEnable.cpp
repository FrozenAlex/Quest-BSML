#include "hooking.hpp"
#include "logging.hpp"
#include "VRPointerEnabledPatch.hpp"
#include "UnityEngine/GameObject.hpp"

DEFINE_TYPE(BSML, VRPointerEnabledPatch);

namespace BSML {
    UnorderedEventCallback<VRUIControls::VRPointer*> VRPointerEnabledPatch::OnPointerEnabled;

    void VRPointerEnabledPatch::Awake() {
        _vrPointer = GetComponent<VRUIControls::VRPointer*>();
    }

    void VRPointerEnabledPatch::OnEnable() {
        if (OnPointerEnabled.size() > 0)
            OnPointerEnabled.invoke(_vrPointer);
    }
}

MAKE_AUTO_HOOK_MATCH(VRPointer_Awake, &::VRUIControls::VRPointer::Awake, void, VRUIControls::VRPointer* self) {
    VRPointer_Awake(self);
    DEBUG("VRPointer_Awake");
    auto pointerEnabledPatch = self->GetComponent<BSML::VRPointerEnabledPatch*>();
    if (!pointerEnabledPatch) {
        self->get_gameObject()->AddComponent<BSML::VRPointerEnabledPatch*>();
    }
}
