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

static bool isWindowFloating(PHLWINDOW window) {
#if __has_include(<hyprland/src/desktop/view/Window.hpp>)
    return window->m_isFloating;
#else
    return window->isFloating();
#endif
}

eScreenEdge detectScreenEdge(const Hyprutils::Math::Vector2D& mouseCoords, double edgeThreshold, double cornerThreshold) {
    const auto pMonitor = State::monitorState()->query().vec(mouseCoords).run();
    if (!pMonitor)
        return eScreenEdge::NONE;

    const double leftBound   = pMonitor->m_position.x;
    const double rightBound  = pMonitor->m_position.x + pMonitor->m_size.x;
    const double topBound    = pMonitor->m_position.y;
    const double bottomBound = pMonitor->m_position.y + pMonitor->m_size.y;

    const bool isAtTop    = (mouseCoords.y <= topBound + edgeThreshold);
    const bool isAtBottom = (mouseCoords.y >= bottomBound - edgeThreshold);
    const bool isAtLeft   = (mouseCoords.x <= leftBound + edgeThreshold);
    const bool isAtRight  = (mouseCoords.x >= rightBound - edgeThreshold);

    const bool isNearLeftCorner   = (mouseCoords.x <= leftBound + cornerThreshold);
    const bool isNearRightCorner  = (mouseCoords.x >= rightBound - cornerThreshold);
    const bool isNearTopCorner    = (mouseCoords.y <= topBound + cornerThreshold);
    const bool isNearBottomCorner = (mouseCoords.y >= bottomBound - cornerThreshold);

    // 1. Top edge and top corners
    if (isAtTop) {
        const auto pMonitorAbove = State::monitorState()->query().vec({mouseCoords.x, topBound - 1}).run();
        if (!pMonitorAbove || pMonitorAbove == pMonitor) {
            if (isNearLeftCorner)
                return eScreenEdge::TOP_LEFT;
            if (isNearRightCorner)
                return eScreenEdge::TOP_RIGHT;
            return eScreenEdge::TOP;
        }
    }

    // 2. Bottom edge and bottom corners
    if (isAtBottom) {
        const auto pMonitorBelow = State::monitorState()->query().vec({mouseCoords.x, bottomBound + 1}).run();
        if (!pMonitorBelow || pMonitorBelow == pMonitor) {
            if (isNearLeftCorner)
                return eScreenEdge::BOTTOM_LEFT;
            if (isNearRightCorner)
                return eScreenEdge::BOTTOM_RIGHT;
            return eScreenEdge::BOTTOM;
        }
    }

    // 3. Left edge and left corners
    if (isAtLeft) {
        const auto pMonitorLeft = State::monitorState()->query().vec({leftBound - 1, mouseCoords.y}).run();
        if (!pMonitorLeft || pMonitorLeft == pMonitor) {
            if (isNearTopCorner)
                return eScreenEdge::TOP_LEFT;
            if (isNearBottomCorner)
                return eScreenEdge::BOTTOM_LEFT;
            return eScreenEdge::LEFT;
        }
    }

    // 4. Right edge and right corners
    if (isAtRight) {
        const auto pMonitorRight = State::monitorState()->query().vec({rightBound + 1, mouseCoords.y}).run();
        if (!pMonitorRight || pMonitorRight == pMonitor) {
            if (isNearTopCorner)
                return eScreenEdge::TOP_RIGHT;
            if (isNearBottomCorner)
                return eScreenEdge::BOTTOM_RIGHT;
            return eScreenEdge::RIGHT;
        }
    }

    return eScreenEdge::NONE;
}

CBox getTargetBoxForEdge(eScreenEdge edge, PHLMONITOR pMonitor) {
    if (!pMonitor)
        return {};

    const CBox workArea = pMonitor->logicalBoxMinusReserved();
    const double halfW  = workArea.width / 2.0;
    const double halfH  = workArea.height / 2.0;

    switch (edge) {
        case eScreenEdge::TOP:
            return workArea;
        case eScreenEdge::BOTTOM: {
            const double floatW = workArea.width * 0.7;
            const double floatH = workArea.height * 0.7;
            return CBox{workArea.x + (workArea.width - floatW) / 2.0, workArea.y + (workArea.height - floatH) / 2.0, floatW, floatH};
        }
        case eScreenEdge::LEFT:
            return CBox{workArea.x, workArea.y, halfW, workArea.height};
        case eScreenEdge::RIGHT:
            return CBox{workArea.x + halfW, workArea.y, halfW, workArea.height};
        case eScreenEdge::TOP_LEFT:
            return CBox{workArea.x, workArea.y, halfW, halfH};
        case eScreenEdge::TOP_RIGHT:
            return CBox{workArea.x + halfW, workArea.y, halfW, halfH};
        case eScreenEdge::BOTTOM_LEFT:
            return CBox{workArea.x, workArea.y + halfH, halfW, halfH};
        case eScreenEdge::BOTTOM_RIGHT:
            return CBox{workArea.x + halfW, workArea.y + halfH, halfW, halfH};
        default:
            return {};
    }
}

void handleTopEdgeAction(PHLWINDOW window) {
    if (!Desktop::View::validMapped(window))
        return;

    const auto targetMode = g_config.mode ? (g_config.mode->value() == 1 ? Fullscreen::FSMODE_MAXIMIZED : Fullscreen::FSMODE_FULLSCREEN) : Fullscreen::FSMODE_FULLSCREEN;

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

    g_pEventLoopManager->doLater([window]() {
        if (!Desktop::View::validMapped(window))
            return;
        g_layoutManager->changeFloatingMode(window->layoutTarget());
        g_pHyprRenderer->damageWindow(window);
    });
}

void handleSnapBoxAction(PHLWINDOW window, PHLMONITOR pMonitor, const CBox& targetBox) {
    if (!Desktop::View::validMapped(window))
        return;

    g_pEventLoopManager->doLater([window, targetBox]() {
        if (!Desktop::View::validMapped(window))
            return;

        // Se a janela estiver em tela cheia, remove tela cheia primeiro
        if (Fullscreen::controller()->isFullscreen(window)) {
            Fullscreen::controller()->setFullscreenMode(window, Fullscreen::FSMODE_NONE);
        }

        // Se for tiled, transforma em floating
        if (!isWindowFloating(window)) {
            g_layoutManager->changeFloatingMode(window->layoutTarget());
        }

        window->setBox(targetBox);
        window->sendWindowSize(true);
        g_pHyprRenderer->damageWindow(window);
    });
}

bool dispatchEdgeDropAction(PHLWINDOW window, const Hyprutils::Math::Vector2D& mouseCoords) {
    if (!window || !isPluginEnabled())
        return false;

    const double edgeThresh   = g_config.threshold ? sc<double>(g_config.threshold->value()) : 20.0;
    const double cornerThresh = g_config.corner_threshold ? sc<double>(g_config.corner_threshold->value()) : 60.0;

    const auto edge = detectScreenEdge(mouseCoords, edgeThresh, cornerThresh);
    if (edge == eScreenEdge::NONE)
        return false;

    const auto pMonitor = State::monitorState()->query().vec(mouseCoords).run();
    if (!pMonitor)
        return false;

    switch (edge) {
        case eScreenEdge::TOP:
            handleTopEdgeAction(window);
            return true;
        case eScreenEdge::BOTTOM:
            handleBottomEdgeAction(window);
            return true;
        case eScreenEdge::LEFT:
        case eScreenEdge::RIGHT:
        case eScreenEdge::TOP_LEFT:
        case eScreenEdge::TOP_RIGHT:
        case eScreenEdge::BOTTOM_LEFT:
        case eScreenEdge::BOTTOM_RIGHT: {
            const CBox targetBox = getTargetBoxForEdge(edge, pMonitor);
            handleSnapBoxAction(window, pMonitor, targetBox);
            return true;
        }
        default:
            return false;
    }
}
