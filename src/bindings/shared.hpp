#pragma once

#include "../QJS.hpp"
#include <cocos2d.h>

namespace binds {

static void nodeFinalizer(qjs::Runtime const&, qjs::Value const& value) {
    auto* ptr = value.getOpaque<cocos2d::CCObject*>();
    if (ptr) ptr->release();
}

static double addrToId(void* ptr) {
    auto bits = std::bit_cast<uintptr_t>(ptr);
    return static_cast<double>(bits);
}

}

#define DEFINE_NODE_CONVERSION(type, name) \
template <> \
struct qjs::detail::JsTypeToCpp<type> { \
    static Result<type> from(Context ctx, Value arg) { \
        if (!ctx.isInstanceOfClass(arg, name)) { \
            return Err("Expected " name ", got {}", arg.getTypeName()); \
        } \
        return Ok(arg.getOpaque<type>()); \
    } \
    static Value to(Context ctx, type value) { \
        if (!value) return ctx.createNull(); \
        auto ret = ctx.createObject(*ctx.getRuntime().getClassID(name)); \
        value->retain(); \
        ret.setOpaque(value); \
        return ret; \
    } \
}; \
static_assert(qjs::detail::IsValidJsTypeToCpp<type>)