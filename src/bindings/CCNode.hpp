#pragma once

#include "shared.hpp"
#include <Geode/platform/platform.hpp>

using namespace cocos2d;
using geode::cast::typeinfo_cast;

DEFINE_NODE_CONVERSION(cocos2d::CCNode*, "Node");

namespace binds {

static void setupCCNode(qjs::Runtime& rt, qjs::Context& ctx) {
    auto proto = ctx.createObject();
    proto.setProperty("debug", [](qjs::Context, qjs::Value self) {
        return geode::format_as(self.getOpaque<CCNode*>());
    });

    proto.setProperty("addrId", [](qjs::Context, qjs::Value self) {
        return addrToId(self.getOpaque<CCNode*>());
    });

    proto.setCreateFunction("<Node>.addChild", [](qjs::Context, qjs::Value self, CCNode* child) {
        self.getOpaque<CCNode*>()->addChild(child);
        return JS_UNDEFINED;
    });

    proto.setCreateFunction("<Node>.setPosition", [](qjs::Context, qjs::Value self, float x, float y) {
        self.getOpaque<CCNode*>()->setPosition(x, y);
        return JS_UNDEFINED;
    });

    proto.setCreateFunction("<Node>.insertAfter", [](qjs::Context, qjs::Value self, CCNode* child, CCNode* after) {
        self.getOpaque<CCNode*>()->insertAfter(child, after);
        return JS_UNDEFINED;
    });

    proto.setCreateFunction("<Node>.getParent", [](qjs::Context ctx, qjs::Value self) -> CCNode* {
        if (!ctx.isInstanceOfClass(self, "Node")) {
            ctx.throwTypeError("Expected Node, got {}", self.getTypeName());
            return nullptr;
        }
        return self.getOpaque<CCNode*>()->getParent();
    });

    proto.setCreateFunction("<Node>.getChildren", [](qjs::Context, qjs::Value self) {
        return self.getOpaque<CCNode*>()->getChildrenExt().toVector();
    });

    proto.setCreateFunction("<Node>.removeChild", [](qjs::Context, qjs::Value self, CCNode* child) {
        self.getOpaque<CCNode*>()->removeChild(child);
        return JS_UNDEFINED;
    });

    proto.setCreateFunction("<Node>.updateLayout", [](qjs::Context, qjs::Value self) {
        self.getOpaque<CCNode*>()->updateLayout();
        return JS_UNDEFINED;
    });

    proto.setCreateFunction("<Node>.setProperty", [](qjs::Context ctx, qjs::Value self, std::string name, JSValue value_) {
        auto* node = self.getOpaque<CCNode*>();
        auto value = qjs::Value::copy(ctx, value_);
        if (auto* button = typeinfo_cast<geode::Button*>(node)) {
            if (name == "onClick") {
                if (!value.isFunction()) {
                    return ctx.throwTypeError("onClick must be a function");
                }
                button->setActivateCallback([ctx, callback = value](geode::Button*) mutable {
                    callback.call(ctx.createUndefined(), {});
                });
            }
        }
        return ctx.createUndefined();
    });

    auto node = rt.createClass("Node", nodeFinalizer).unwrap();
    ctx.setClassProto(node, proto);
}

}