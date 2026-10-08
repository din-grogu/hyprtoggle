#include "edge_action.hpp"
#include "globals.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/state/MonitorState.hpp>
#if __has_include(<hyprland/src/desktop/view/Window.hpp>)
#include <hyprland/src/desktop/view/Window.hpp>
#elif __has_include(<hyprland/src/desktop/view/window/Window.hpp>)
#include <hyprland/src/desktop/view/window/Window.hpp>
#endif
#include <hyprland/src/layout/LayoutManager.hpp>

eScreenEdge detectScreenEdge(const Hyprutils::Math::Vector2D& mouseCoords, double threshold) {
    const auto pMonitor = State::monitorState()->query().vec(mouseCoords).run();
    if (!pMonitor)
        return eScreenEdge::NONE;

    // Borda superior (Topo)
    if (mouseCoords.y <= pMonitor->m_position.y + threshold) {
        const auto pMonitorAbove = State::monitorState()->query().vec({mouseCoords.x, pMonitor->m_position.y - 1}).run();
        // Se a busca retornar nullptr ou o próprio monitor, significa que não há outro monitor acima
        if (!pMonitorAbove || pMonitorAbove == pMonitor)
            return eScreenEdge::TOP;
    }

    // Borda inferior (Rodapé)
    if (mouseCoords.y >= (pMonitor->m_position.y + pMonitor->m_size.y) - threshold) {
        const auto pMonitorBelow = State::monitorState()->query().vec({mouseCoords.x, pMonitor->m_position.y + pMonitor->m_size.y + 1}).run();
        if (!pMonitorBelow || pMonitorBelow == pMonitor)
            return eScreenEdge::BOTTOM;
    }

    // Borda esquerda
    if (mouseCoords.x <= pMonitor->m_position.x + threshold) {
        const auto pMonitorLeft = State::monitorState()->query().vec({pMonitor->m_position.x - 1, mouseCoords.y}).run();
        if (!pMonitorLeft || pMonitorLeft == pMonitor)
            return eScreenEdge::LEFT;
    }

    // Borda direita
    if (mouseCoords.x >= (pMonitor->m_position.x + pMonitor->m_size.x) - threshold) {
        const auto pMonitorRight = State::monitorState()->query().vec({pMonitor->m_position.x + pMonitor->m_size.x + 1, mouseCoords.y}).run();
        if (!pMonitorRight || pMonitorRight == pMonitor)
            return eScreenEdge::RIGHT;
    }

    return eScreenEdge::NONE;
}

void handleTopEdgeAction(PHLWINDOW window) {
    if (!Desktop::View::validMapped(window))
        return;

    const auto targetMode = g_config.mode ? (g_config.mode->value() == 1 ? Fullscreen::FSMODE_MAXIMIZED : Fullscreen::FSMODE_FULLSCREEN) : Fullscreen::FSMODE_FULLSCREEN;

    // Agenda a transição no event loop para permitir que o drop finalize o layout antes
    g_pEventLoopManager->doLater([window, targetMode]() {
        if (!Desktop::View::validMapped(window))
            return;
        Fullscreen::controller()->setFullscreenMode(window, targetMode);
        g_pHyprRenderer->damageWindow(window);
    });
}

void handleBottomEdgeAction(PHLWINDOW window) {
    if (!Desktop::View::validMapped(window))
        return;

    // Modular: pronto para implementar toggle de floating window posteriormente
    // g_layoutManager->changeFloatingMode(window->layoutTarget());
}

bool dispatchEdgeDropAction(PHLWINDOW window, const Hyprutils::Math::Vector2D& mouseCoords) {
    if (!window || !isPluginEnabled())
        return false;

    const double threshold = g_config.threshold ? sc<double>(g_config.threshold->value()) : 25.0;
    const auto   edge      = detectScreenEdge(mouseCoords, threshold);

    switch (edge) {
        case eScreenEdge::TOP:
            handleTopEdgeAction(window);
            return true;
        case eScreenEdge::BOTTOM:
            handleBottomEdgeAction(window);
            return true;
        default:
            return false;
    }
}
