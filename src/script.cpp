#include "script.hpp"
#include "QJS.hpp"
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;
using namespace std::string_view_literals;

template <>
struct qjs::detail::JsTypeToCpp<CCNode*> {
    static Result<CCNode*> from(Context ctx, Value arg) {
        if (!ctx.isInstanceOfClass(arg, "Node")) {
            return Err("Expected Node, got {}", arg.getTypeName());
        }
        return Ok(arg.getOpaque<CCNode*>());
    }
    static Value to(Context ctx, CCNode* value) {
        auto ret = ctx.createObject(*ctx.getRuntime().getClassID("Node"));
        value->retain();
        ret.setOpaque(value);
        return ret;
    }
};
static_assert(qjs::detail::IsValidJsTypeToCpp<CCNode*>);

template <>
struct qjs::detail::JsTypeToCpp<geode::Label*> {
    static Result<geode::Label*> from(Context ctx, Value arg) {
        if (!ctx.isInstanceOfClass(arg, "Label")) {
            return Err("Expected Label, got {}", arg.getTypeName());
        }
        return Ok(arg.getOpaque<geode::Label*>());
    }
    static Value to(Context ctx, geode::Label* value) {
        auto ret = ctx.createObject(*ctx.getRuntime().getClassID("Label"));
        value->retain();
        ret.setOpaque(value);
        return ret;
    }
};
static_assert(qjs::detail::IsValidJsTypeToCpp<geode::Label*>);

static qjs::Value js_console_log(qjs::Context ctx, qjs::Value, std::vector<qjs::Value> const& args) {
    std::string line;
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) line += ' ';
        line += args[i].toString();
    }
    log::info("[console.log] {}", line);
    return ctx.createUndefined();
}

static void install_console(qjs::Context& ctx) {
    auto console = ctx.createObject();
    console.setProperty("log", ctx.createFunctionBare("log", js_console_log));
    console.setProperty("error", ctx.createFunctionBare("error", js_console_log));
    ctx.getGlobalObject().setProperty("console", console);
}

static char* normalize_module(JSContext* ctx, const char* base_name, const char* name, void*) {
    std::filesystem::path p = std::filesystem::path(base_name).parent_path() / name;
    p = p.lexically_normal();
    return js_strdup(ctx, p.generic_string().c_str());
}

static JSModuleDef* load_module(JSContext* ctx, const char* module_name, void*) {
    auto file = utils::file::readString(module_name);
    if (!file) {
        JS_ThrowReferenceError(ctx, "could not load module '%s'", module_name);
        return nullptr;
    }
    auto src = *file;
    JSValue func = JS_Eval(ctx, src.c_str(), src.size(), module_name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(func)) return nullptr;
    auto* m = static_cast<JSModuleDef*>(JS_VALUE_GET_PTR(func));
    JS_FreeValue(ctx, func);
    return m;
}

static qjs::Value createAnyNode(qjs::Context ctx, qjs::Value, std::vector<qjs::Value> const& args) {
    if (args.size() < 1) return ctx.createNull();
    auto name = args[0].toString();
    std::unordered_map<std::string, qjs::Value> props;
    if (args.size() > 1) {
        props = args[1].getProperties();
    }

    log::info("Tried to create {}", name);
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

std::unique_ptr<Script> loadScript(CCNode* node, std::string_view path) {
    auto file = utils::file::readString(path);
    if (!file) {
        log::error("Failed to read file");
        return {};
    }

    auto rt = qjs::Runtime::create();
    JS_SetModuleLoaderFunc(rt.getRaw(), normalize_module, load_module, nullptr);
    auto ctx = qjs::Context::create(rt);

    install_console(ctx);
    
    auto global = ctx.getGlobalObject();
    
    global.setProperty("getScene", ctx.createFunction("getScene", [node](qjs::Context, qjs::Value) { return node; }));
    global.setProperty("createLabelNode",
        ctx.createFunction("createLabelNode", +[](qjs::Context, qjs::Value, std::string text) {
            auto* label = geode::Label::create(text, "bigFont.fnt");
            return label;
        })
    );

    global.setProperty("createAnyNode",
        ctx.createFunctionBare("createAnyNode", createAnyNode)
    );

    {
        const auto nodeFinalizer = [](qjs::Runtime const&, qjs::Value const& value) {
            auto* ptr = value.getOpaque<CCNode*>();
            // log::info("node finalizer called! {}", ptr);
            if (ptr) ptr->release();
        };

        auto node = rt.createClass("Node", nodeFinalizer).unwrap();
        auto nodeProto = ctx.createObject();
        nodeProto.setProperty("addChild",
            ctx.createFunction("<Node>.addChild", [](qjs::Context, qjs::Value self, CCNode* child) {
                self.getOpaque<CCNode*>()->addChild(child);
                return JS_UNDEFINED;
            })
        );
        nodeProto.setProperty("setPosition",
            ctx.createFunction("<Node>.setPosition", [](qjs::Context, qjs::Value self, float x, float y) {
                self.getOpaque<CCNode*>()->setPosition(x, y);
                return JS_UNDEFINED;
            })
        );
        nodeProto.setProperty("insertAfter",
            ctx.createFunction("<Node>.insertAfter", [](qjs::Context, qjs::Value self, CCNode* child, CCNode* after) {
                self.getOpaque<CCNode*>()->insertAfter(child, after);
                return JS_UNDEFINED;
            })
        );
        nodeProto.setProperty("getParent",
            ctx.createFunction("<Node>.getParent", [](qjs::Context, qjs::Value self) {
                return self.getOpaque<CCNode*>()->getParent();
            })
        );
        nodeProto.setProperty("getChildren",
            ctx.createFunction("<Node>.getChildren", [](qjs::Context, qjs::Value self) {
                return self.getOpaque<CCNode*>()->getChildrenExt().toVector();
            })
        );
        nodeProto.setProperty("removeChild",
            ctx.createFunction("<Node>.removeChild", [](qjs::Context, qjs::Value self, CCNode* child) {
                self.getOpaque<CCNode*>()->removeChild(child);
                return JS_UNDEFINED;
            })
        );
        nodeProto.setProperty("updateLayout",
            ctx.createFunction("<Node>.updateLayout", [](qjs::Context, qjs::Value self) {
                self.getOpaque<CCNode*>()->updateLayout();
                return JS_UNDEFINED;
            })
        );
        nodeProto.setProperty("setProperty",
            ctx.createFunctionBare("<Node>.setProperty", [](qjs::Context ctx, qjs::Value self, std::vector<qjs::Value> const& args) {
                if (args.size() < 2) return ctx.createUndefined();
                auto name = args[0].toString();
                auto const& value = args[1];
                auto* node = self.getOpaque<CCNode*>();
                log::info("Tried to set property {} on {}", name, node);
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
            })
        );
        ctx.setClassProto(node, nodeProto);

        auto label = rt.createClass("Label", nodeFinalizer).unwrap();
        auto labelProto = ctx.createObject();
        labelProto.setPrototype(nodeProto);
        labelProto.setProperty("setText", ctx.createFunction("<Label>.setText", [](qjs::Context, qjs::Value self, std::string const& text) {
            self.getOpaque<geode::Label*>()->setText(text);
            return JS_UNDEFINED;
        }));
        ctx.setClassProto(label, labelProto);
    }

    auto mod = ctx.eval(file.unwrap(), path);
    if (!mod) {
        log::error("eval failed: {}", mod.unwrapErr());
        return {};
    }
    
    auto module = mod.unwrap();
    return std::make_unique<Script>(Script { .rt = std::move(rt), .ctx = std::move(ctx), .module = std::move(module) });
}

void Script::tick() {
    if (finished) return;
    auto step = module.tick();
    if (!step) {
        log::error("module error: {}", step.unwrapErr());
        finished = true;
        return;
    }
    auto value = step.unwrap();
    if (value) {
        if (!value->isUndefined())
            log::debug("Finished running module with return value {}", value->toString());
        finished = true;
    }
}