#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>

inline HANDLE PHANDLE = nullptr;
inline CFunctionHook* g_pDragEndHook = nullptr;

struct SPluginConfig {
    SP<Config::Values::CIntValue> enabled;
    SP<Config::Values::CIntValue> mode;
    SP<Config::Values::CIntValue> threshold;
};

inline SPluginConfig g_config;

inline bool isPluginEnabled() {
    if (!g_config.enabled)
        return true;
    return g_config.enabled->value() != 0;
}
