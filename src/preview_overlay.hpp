#pragma once

#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/output/Monitor.hpp>
#include "edge_action.hpp"

struct SPreviewState {
    bool        active = false;
    eScreenEdge edge   = eScreenEdge::NONE;
    CBox        targetBox;
    PHLMONITOR  monitor = nullptr;
};

inline SPreviewState g_previewState;

void initPreviewOverlay();
void updatePreview(eScreenEdge edge, const CBox& targetBox, PHLMONITOR pMonitor);
void clearPreview();
void renderPreview(PHLMONITOR pMonitor);
