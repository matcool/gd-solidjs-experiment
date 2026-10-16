#pragma once

#include "shared.hpp"

using namespace geode::prelude;
using namespace std::string_view_literals;

namespace binds {

static qjs::Value createAnyNode(qjs::Context ctx, qjs::Value, std::string name, std::unordered_map<std::string, qjs::Value> props) {
    if (name == "Row"sv) {
        auto* node = CCNode::create();
        auto* layout = geode::SimpleRowLayout::create()
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setMainAxisScaling(AxisScaling::Grow);
        node->setLayout(layout);
        return ctx.createCpp(node);
    } else if (name == "Column"sv) {
        auto* node = CCNode::create();
        auto* layout = geode::SimpleColumnLayout::create()
            ->setCrossAxisScaling(AxisScaling::Grow)
            ->setMainAxisScaling(AxisScaling::Grow);
        node->setLayout(layout);
        return ctx.createCpp(node);
    } else if (name == "Button"sv) {
        auto text = props.contains("label") ? props.at("label").toString() : std::string("");
        auto* node = geode::Button::createWithLabel(text, "bigFont.fnt");
        return ctx.createCpp((CCNode*)node);
    }
    return ctx.createNull();
}

static void setupCreates(qjs::Runtime& rt, qjs::Context& ctx) {
    auto global = ctx.getGlobalObject();

    global.setCreateFunction("createLabelNode", [](qjs::Context, qjs::Value, std::string text) {
        auto* label = geode::Label::create(text, "bigFont.fnt");
        return label;
    });

    global.setCreateFunction("createAnyNode", createAnyNode);
}

}