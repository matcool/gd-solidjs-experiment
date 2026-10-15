#pragma once

#include <string_view>
#include <cocos2d.h>
#include "QJS.hpp"
#include <memory>

struct Script {
    qjs::Runtime rt;
    qjs::Context ctx;
    qjs::Module module;
    bool finished = false;

    void tick();
};

std::unique_ptr<Script> loadScript(cocos2d::CCNode* parent, std::string_view path);