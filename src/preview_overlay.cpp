#include "preview_overlay.hpp"
#include "globals.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/render/Renderer.hpp>

using namespace Render::GL;

static PHLMONITOR s_currentRenderMonitor  = nullptr;
static bool       s_hasRenderedThisFrame = false;

void initPreviewOverlay() {
    static auto P_PRE = Event::bus()->m_events.render.pre.listen([](PHLMONITOR pMonitor) {
        s_currentRenderMonitor  = pMonitor;
        s_hasRenderedThisFrame = false;
    });

    static auto P_STAGE = Event::bus()->m_events.render.stage.listen([](eRenderStage stage) {
        if (!s_hasRenderedThisFrame && (stage == eRenderStage::RENDER_POST_WINDOWS || stage == eRenderStage::RENDER_LAST_MOMENT) && s_currentRenderMonitor) {
            renderPreview(s_currentRenderMonitor);
            s_hasRenderedThisFrame = true;
        }
    });
}

void updatePreview(eScreenEdge edge, const CBox& targetBox, PHLMONITOR pMonitor) {
    if (!isPreviewEnabled() || !isPluginEnabled()) {
        clearPreview();
        return;
    }

    if (g_previewState.active && g_previewState.edge == edge && g_previewState.monitor == pMonitor && g_previewState.targetBox == targetBox) {
        return;
    }

    PHLMONITOR oldMonitor = g_previewState.monitor;
    CBox       oldBox     = g_previewState.targetBox;

    g_previewState.active    = true;
    g_previewState.edge      = edge;
    g_previewState.targetBox = targetBox;
    g_previewState.monitor   = pMonitor;

    if (oldMonitor && oldBox.width > 0 && oldBox.height > 0) {
        g_pHyprRenderer->damageBox(oldBox);
        g_pHyprRenderer->damageMonitor(oldMonitor);
    }

    if (pMonitor && targetBox.width > 0 && targetBox.height > 0) {
        g_pHyprRenderer->damageBox(targetBox);
        g_pHyprRenderer->damageMonitor(pMonitor);
    }
}

void clearPreview() {
    if (!g_previewState.active)
        return;

    g_previewState.active = false;
    g_previewState.edge   = eScreenEdge::NONE;

    if (g_previewState.monitor) {
        g_pHyprRenderer->damageBox(g_previewState.targetBox);
        g_pHyprRenderer->damageMonitor(g_previewState.monitor);
        g_previewState.monitor = nullptr;
    }
}

void renderPreview(PHLMONITOR pMonitor) {
    if (!g_previewState.active || !g_previewState.monitor || g_previewState.monitor != pMonitor)
        return;

    if (g_previewState.targetBox.width < 1 || g_previewState.targetBox.height < 1)
        return;

    CBox box = g_previewState.targetBox;
    box.translate(-pMonitor->m_position);
    box.scale(pMonitor->m_scale).round();

    if (box.width < 1 || box.height < 1)
        return;

    // Cor do fundo translúcido suave (azul acento moderno)
    const CHyprColor fillColor{0.15f, 0.45f, 0.90f, 0.25f};
    const CHyprColor borderColor{0.30f, 0.65f, 1.00f, 0.85f};

    const int round = g_config.preview_rounding ? g_config.preview_rounding->value() : 10;
    const int borderSize = g_config.preview_border_size ? g_config.preview_border_size->value() : 2;

    CRegion previewDamage{box};

    CHyprOpenGLImpl::SRectRenderData rectData;
    rectData.round         = round * pMonitor->m_scale;
    rectData.roundingPower = 2.0f;
    rectData.damage        = &previewDamage;

    g_pHyprOpenGL->renderRect(box, fillColor, rectData);

    CHyprOpenGLImpl::SBorderRenderData borderData;
    borderData.round         = round * pMonitor->m_scale;
    borderData.borderSize    = borderSize;
    borderData.roundingPower = 2.0f;

    g_pHyprOpenGL->renderBorder(box, Config::CGradientValueData{borderColor}, borderData);
}
