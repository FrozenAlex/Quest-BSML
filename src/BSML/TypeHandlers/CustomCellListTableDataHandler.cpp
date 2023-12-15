#include "BSML/TypeHandlers/CustomCellListTableDataHandler.hpp"
#include "Helpers/getters.hpp"
#include "Helpers/delegates.hpp"

#include "HMUI/ScrollView.hpp"
#include "HMUI/TextPageScrollView.hpp"
#include "HMUI/VerticalScrollIndicator.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

using namespace UnityEngine;

extern HMUI::TableView::TableType stringToTableType(const std::string& str);

namespace BSML {
    static CustomCellListTableDataHandler customCellListTableDataHandler{};
    HMUI::TextPageScrollView* get_textPageTemplate();

    CustomCellListTableDataHandler::Base::PropMap CustomCellListTableDataHandler::get_props() const {
        return {
            { "selectCell", {"select-cell"}},
            { "visibleCells", {"visible-cells"}},
            { "cellSize", {"cell-size"}},
            { "id", {"id"}},
            { "listWidth", {"list-width"}},
            { "listHeight", {"list-height"}},
            { "listDirection", {"list-direction"}},
            { "data", {"contents", "data"}},
            { "cellClickable", {"clickable-cells"}},
            { "cellTemplate", {"_children"}},
            { "alignCenter", {"align-to-center"}},
            { "stickScrolling", {"stick-scrolling"}},
            { "showScrollbar", {"show-scrollbar"}}
        };
    }

    CustomCellListTableDataHandler::Base::SetterMap CustomCellListTableDataHandler::get_setters() const {
        return {
            {"cellClickable",     [](auto component, auto value){ component->clickableCells = value.tryParseBool().value_or(true); }},
            {"alignCenter",     [](auto component, auto value){ component->tableView->_alignToCenter = value; }},
            {"stickScrolling",  [](auto component, auto value){ if (static_cast<bool>(value)) component->tableView->_scrollView->_platformHelper = Helpers::GetIVRPlatformHelper(); }}
        };
    }

    void CustomCellListTableDataHandler::HandleType(const ComponentTypeWithData& componentType, BSMLParserParams& parserParams) {
        Base::HandleType(componentType, parserParams);
        auto tableData = reinterpret_cast<CustomCellListTableData*>(componentType.component);
        auto tableView = tableData->tableView;
        auto scrollView = tableView->scrollView;

        auto& data = componentType.data;

        auto selectCellItr = data.find("selectCell");
        if (selectCellItr != data.end() && !selectCellItr->second.empty()) {
            auto action = parserParams.TryGetAction(selectCellItr->second);
            if (action) tableView->add_didSelectCellWithIdxEvent(action->GetSystemAction<HMUI::TableView *, int>());
            else ERROR("Action '{}' could not be found", selectCellItr->second);
        }

        bool verticalList = true;
        auto listDirectionItr = data.find("listDirection");
        if (listDirectionItr != data.end() && !listDirectionItr->second.empty()) {
            auto arg = StringParseHelper(listDirectionItr->second);
            tableView->_tableType = stringToTableType(arg);
            scrollView->_scrollViewDirection = tableView->get_tableType() == HMUI::TableView::TableType::Vertical ? HMUI::ScrollView::ScrollViewDirection::Vertical : HMUI::ScrollView::ScrollViewDirection::Horizontal;
            verticalList = tableView->get_tableType() == HMUI::TableView::TableType::Vertical;
        }

        auto cellSizeItr = data.find("cellSize");
        if (cellSizeItr != data.end() && !cellSizeItr->second.empty()) {
            tableData->cellSize = StringParseHelper(cellSizeItr->second);
        }

        auto showScrollBarItr = data.find("showScrollbar");
        // We can't show on a horizontal list
        if (verticalList && showScrollBarItr != data.end() && !showScrollBarItr->second.empty()) {
            auto arg = StringParseHelper(showScrollBarItr->second);
            if (static_cast<bool>(arg)) {
                auto textScrollView = Object::Instantiate(get_textPageTemplate(), tableData->get_transform(), false);

                auto pageUpButton = textScrollView->_pageUpButton;
                auto pageDownButton = textScrollView->_pageDownButton;

                auto scrollIndicator = textScrollView->_verticalScrollIndicator;
                auto scrollBar = reinterpret_cast<RectTransform*>(scrollIndicator->get_transform()->get_parent());

                scrollView->_pageUpButton = pageUpButton;
                scrollView->_pageDownButton = pageDownButton;
                scrollView->_verticalScrollIndicator = scrollIndicator;
                scrollBar->SetParent(tableData->get_transform());

                Object::Destroy(textScrollView->get_gameObject());

                scrollBar->set_anchorMin({1, 0});
                scrollBar->set_anchorMax({1, 1});
                scrollBar->set_offsetMin({0, 0});
                scrollBar->set_offsetMax({8, 0});
            }
        }

        auto dataItr = data.find("data");
        if (dataItr != data.end() && !dataItr->second.empty()) {
            auto val = parserParams.TryGetValue(dataItr->second);
            List<Il2CppObject*>* cellData = val ? val->GetValue<List<Il2CppObject*>*>() : nullptr;
            static auto dataKlass = classof(List<Il2CppObject*>*);
            if (cellData && il2cpp_functions::class_is_assignable_from(cellData->klass, dataKlass)) {
                tableData->data = cellData;
                tableView->ReloadData();
            } else if (cellData && !il2cpp_functions::class_is_assignable_from(cellData->klass, dataKlass)){
                ERROR("The class of the data field was not Correct! this should be a 'List<Il2CppObject*>*' or equivalent!");
                ERROR("Class {}::{} is not assignable from {}::{}", cellData->klass->namespaze, cellData->klass->name, dataKlass->namespaze, dataKlass->name);
            } else {
                ERROR("IconSegmentedControl needs to have at least 1 value!");
                ERROR("This means BSML could not find value '{}'", dataItr->second);
            }
        }

        INFO("set sizeDelta");
        auto transform = reinterpret_cast<RectTransform*>(tableData->get_transform());
        switch(tableView->get_tableType()) {
            case HMUI::TableView::TableType::Vertical: {
                auto listWidthItr = data.find("listWidth");
                auto listWidth = listWidthItr != data.end() ? static_cast<float>(StringParseHelper(listWidthItr->second)) : 60.0f;
                auto visibleCellsItr = data.find("visibleCells");
                auto visibleCells = visibleCellsItr != data.end() ? static_cast<int>(StringParseHelper(visibleCellsItr->second)) : 7;
                float listHeight = visibleCells * tableData->cellSize;
                transform->set_sizeDelta({listWidth, listHeight});
                INFO("listWidth: {}, listHeight: {}", listWidth, listHeight);
                break;
            }
            case HMUI::TableView::TableType::Horizontal: {
                auto listHeightItr = data.find("listHeight");
                auto listHeight = listHeightItr != data.end() ? static_cast<float>(StringParseHelper(listHeightItr->second)) : 40.0f;
                auto visibleCellsItr = data.find("visibleCells");
                auto visibleCells = visibleCellsItr != data.end() ? static_cast<int>(StringParseHelper(visibleCellsItr->second)) : 4;
                float listWidth = visibleCells * tableData->cellSize;
                INFO("listWidth: {}, listHeight: {}", listWidth, listHeight);
                transform->set_sizeDelta({listWidth, listHeight});
                break;
            }
        }

        auto layoutElement = tableData->GetComponent<UnityEngine::UI::LayoutElement*>();
        if (layoutElement) {
            auto sizeDelta = transform->get_sizeDelta();
            INFO("Size: {}, {}", sizeDelta.x, sizeDelta.y);
            layoutElement->set_preferredHeight(sizeDelta.y);
            layoutElement->set_flexibleHeight(sizeDelta.y);
            layoutElement->set_minHeight(sizeDelta.y);
            layoutElement->set_preferredWidth(sizeDelta.x);
            layoutElement->set_flexibleWidth(sizeDelta.x);
            layoutElement->set_minWidth(sizeDelta.x);
        }

        tableView->get_gameObject()->SetActive(true);
        tableView->LazyInit();

        auto idItr = data.find("id");
        if (idItr != data.end() && !idItr->second.empty()) {
            std::string id = idItr->second;
            auto pageUpMinfo = il2cpp_utils::FindMethodUnsafe(scrollView, "PageUpButtonPressed", 0);
            auto pageDownMinfo = il2cpp_utils::FindMethodUnsafe(scrollView, "PageDownButtonPressed", 0);

            if (pageUpMinfo) parserParams.AddAction(id + "#PageUp", new BSMLAction(scrollView, pageUpMinfo));
            if (pageDownMinfo) parserParams.AddAction(id + "#PageDown", new BSMLAction(scrollView, pageDownMinfo));
        }
    }
}
