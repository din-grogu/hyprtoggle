#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>

inline HANDLE PHANDLE = nullptr;
inline CFunctionHook* g_pDragEndHook   = nullptr;
inline CFunctionHook* g_pMouseMoveHook = nullptr;

struct SPluginConfig {
    SP<Config::Values::CIntValue> enabled;
    SP<Config::Values::CIntValue> mode;
    SP<Config::Values::CIntValue> threshold;
    SP<Config::Values::CIntValue> corner_threshold;
    SP<Config::Values::CIntValue> preview;
    SP<Config::Values::CIntValue> preview_rounding;
    SP<Config::Values::CIntValue> preview_border_size;
};

inline SPluginConfig g_config;

inline bool isPluginEnabled() {
    if (!g_config.enabled)
        return true;
    return g_config.enabled->value() != 0;
}

inline bool isPreviewEnabled() {
    if (!g_config.preview)
        return true;
    return g_config.preview->value() != 0;
}
