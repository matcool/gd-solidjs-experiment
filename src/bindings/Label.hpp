#pragma once

#include "shared.hpp"

DEFINE_NODE_CONVERSION(geode::Label*, "Label");

namespace binds {

static void setupLabel(qjs::Runtime& rt, qjs::Context& ctx) {
    auto proto = ctx.createObject();
    proto.setPrototype(ctx.getClassProto(*rt.getClassID("Node")));
    proto.setCreateFunction("<Label>.setText", [](qjs::Context, qjs::Value self, std::string const& text) {
        self.getOpaque<geode::Label*>()->setText(text);
        return JS_UNDEFINED;
    });

    auto label = rt.createClass("Label", nodeFinalizer).unwrap();
    ctx.setClassProto(label, proto);
}

}