#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>

inline HANDLE PHANDLE = nullptr;
inline CFunctionHook* g_pDragEndHook   = nullptr;
inline CFunctionHook* g_pMouseMoveHook = nullptr;

struct SPluginConfig {
    SP<Config::Values::CIntValue>    enabled;
    SP<Config::Values::CIntValue>    threshold;
    SP<Config::Values::CIntValue>    corner_threshold;

    // 8 Zonas / Eixos Customizáveis
    SP<Config::Values::CStringValue> action_top;
    SP<Config::Values::CStringValue> action_bottom;
    SP<Config::Values::CStringValue> action_left;
    SP<Config::Values::CStringValue> action_right;
    SP<Config::Values::CStringValue> action_top_left;
    SP<Config::Values::CStringValue> action_top_right;
    SP<Config::Values::CStringValue> action_bottom_left;
    SP<Config::Values::CStringValue> action_bottom_right;

    // Preview Visual
    SP<Config::Values::CIntValue>    preview;
    SP<Config::Values::CIntValue>    preview_rounding;
    SP<Config::Values::CIntValue>    preview_border_size;
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
