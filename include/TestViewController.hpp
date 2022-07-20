#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"

#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/Button.hpp"

DECLARE_CLASS_CODEGEN(BSML, TestViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::HorizontalLayoutGroup*, layout);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, goodButton);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::ViewController::DidActivate>::get(), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    DECLARE_INSTANCE_METHOD(void, ButtonPressed);
)