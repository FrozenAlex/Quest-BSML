#include "BSML/Tags/TextTag.hpp"
#include "Helpers/getters.hpp"
#include "Helpers/creation.hpp"
#include "logging.hpp"
#include "internal_macros.hpp"

#include "HMUI/CurvedTextMeshPro.hpp"
#include "UnityEngine/GameObject.hpp"

using namespace UnityEngine;

namespace BSML {
    static BSMLTagParser<TextTag> textTagParser({"text", "label"});
    void TextTag::Construct(UnityEngine::Transform* parent, Il2CppObject* host) const {
        auto go = CreateObject(parent);
        auto text = go->GetComponent<TMPro::TextMeshProUGUI*>();
        SetHostField(host, text);

        textMeshProUGUIData.Apply(text);
        rectTransformData.Apply(text->get_rectTransform());
        
        CreateChildren(go->get_transform(), host);
    }

    UnityEngine::GameObject* TextTag::CreateObject(UnityEngine::Transform* parent) const {
        DEBUG("Creating Text");
        auto gameObject = GameObject::New_ctor("BSMLText");
        gameObject->get_transform()->SetParent(parent, false);

        // on PC this is a FormattableText, but we're skipping that for now at least
        auto textMesh = gameObject->AddComponent<HMUI::CurvedTextMeshPro*>();
        textMesh->set_font(Helpers::GetMainTextFont());
        textMesh->set_fontSharedMaterial(Helpers::GetMainUIFontMaterial());
        textMesh->set_fontSize(4);
        textMesh->set_color({1.0f, 1.0f, 1.0f, 1.0f});
        textMesh->set_text("BSMLText");

        auto rectTransform = textMesh->get_rectTransform();
        rectTransform->set_anchorMin({0.5f, 0.5f});
        rectTransform->set_anchorMax({0.5f, 0.5f});
        
        return gameObject;
    }

    void TextTag::parse(const tinyxml2::XMLElement& elem) {
        DEBUG("Parsing text tag");
        this->::BSML::BSMLTag::parse(elem);
        
        textMeshProUGUIData = TextMeshProUGUIData(elem);
        rectTransformData = RectTransformData(elem);
    }
}