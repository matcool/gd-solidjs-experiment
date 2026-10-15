#include "script.hpp"
#include "QJS.hpp"

using namespace geode::prelude;

template <>
struct qjs::detail::JsTypeToCpp<CCNode*> {
    static Result<CCNode*> from(Context ctx, Value arg) {
        if (!arg.isClass(*ctx.getRuntime().getClassID("CCNode"))) {
            return Err("Expected CCNode, got {}", arg.getTypeName());
        }
        return Ok(arg.getOpaque<CCNode*>());
    }
    static Value to(Context ctx, CCNode* value) {
        auto ret = ctx.createObject(*ctx.getRuntime().getClassID("CCNode"));
        value->retain();
        ret.setOpaque(value);
        return ret;
    }
};
static_assert(qjs::detail::IsValidJsTypeToCpp<CCNode*>);

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
    
    ctx.createFunction("getScene", [node]() { return node; });

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