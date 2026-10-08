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

#include <algorithm>
#include <cctype>

static bool isWindowFloating(PHLWINDOW window) {
#if __has_include(<hyprland/src/desktop/view/Window.hpp>)
    return window->m_isFloating;
#else
    return window->isFloating();
#endif
}

eScreenEdge detectScreenEdge(const Vector2D& mouseCoords, double edgeThreshold, double cornerThreshold) {
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

std::string getActionForEdge(eScreenEdge edge) {
    switch (edge) {
        case eScreenEdge::TOP:
            return g_config.action_top ? g_config.action_top->value() : "fullscreen";
        case eScreenEdge::BOTTOM:
            return g_config.action_bottom ? g_config.action_bottom->value() : "toggle_float";
        case eScreenEdge::LEFT:
            return g_config.action_left ? g_config.action_left->value() : "snap_left";
        case eScreenEdge::RIGHT:
            return g_config.action_right ? g_config.action_right->value() : "snap_right";
        case eScreenEdge::TOP_LEFT:
            return g_config.action_top_left ? g_config.action_top_left->value() : "snap_top_left";
        case eScreenEdge::TOP_RIGHT:
            return g_config.action_top_right ? g_config.action_top_right->value() : "snap_top_right";
        case eScreenEdge::BOTTOM_LEFT:
            return g_config.action_bottom_left ? g_config.action_bottom_left->value() : "snap_bottom_left";
        case eScreenEdge::BOTTOM_RIGHT:
            return g_config.action_bottom_right ? g_config.action_bottom_right->value() : "snap_bottom_right";
        default:
            return "none";
    }
}

CBox getTargetBoxForAction(const std::string& action, eScreenEdge edge, PHLMONITOR pMonitor) {
    if (!pMonitor)
        return {};

    const CBox workArea = pMonitor->logicalBoxMinusReserved();
    const double halfW  = workArea.width / 2.0;
    const double halfH  = workArea.height / 2.0;

    std::string act = action;
    std::ranges::transform(act, act.begin(), [](unsigned char c) { return std::tolower(c); });

    if (act == "fullscreen" || act == "toggle_fullscreen" || act == "togglefullscreen" || act == "maximize" || act == "maximized") {
        return workArea;
    }
    if (act == "toggle_float" || act == "togglefloating" || act == "float") {
        const double floatW = workArea.width * 0.7;
        const double floatH = workArea.height * 0.7;
        return CBox{workArea.x + (workArea.width - floatW) / 2.0, workArea.y + (workArea.height - floatH) / 2.0, floatW, floatH};
    }
    if (act == "snap_left" || act == "left") {
        return CBox{workArea.x, workArea.y, halfW, workArea.height};
    }
    if (act == "snap_right" || act == "right") {
        return CBox{workArea.x + halfW, workArea.y, halfW, workArea.height};
    }
    if (act == "snap_top_left" || act == "top_left" || act == "top-left") {
        return CBox{workArea.x, workArea.y, halfW, halfH};
    }
    if (act == "snap_top_right" || act == "top_right" || act == "top-right") {
        return CBox{workArea.x + halfW, workArea.y, halfW, halfH};
    }
    if (act == "snap_bottom_left" || act == "bottom_left" || act == "bottom-left") {
        return CBox{workArea.x, workArea.y + halfH, halfW, halfH};
    }
    if (act == "snap_bottom_right" || act == "bottom_right" || act == "bottom-right") {
        return CBox{workArea.x + halfW, workArea.y + halfH, halfW, halfH};
    }
    if (act == "none" || act.empty()) {
        return {};
    }

    // Indicador visual de destaque caso seja um dispatcher ou comando arbitrário
    const double barThick = 12.0;
    switch (edge) {
        case eScreenEdge::TOP:
            return CBox{workArea.x, workArea.y, workArea.width, barThick};
        case eScreenEdge::BOTTOM:
            return CBox{workArea.x, workArea.y + workArea.height - barThick, workArea.width, barThick};
        case eScreenEdge::LEFT:
            return CBox{workArea.x, workArea.y, barThick, workArea.height};
        case eScreenEdge::RIGHT:
            return CBox{workArea.x + workArea.width - barThick, workArea.y, barThick, workArea.height};
        case eScreenEdge::TOP_LEFT:
            return CBox{workArea.x, workArea.y, 60.0, 60.0};
        case eScreenEdge::TOP_RIGHT:
            return CBox{workArea.x + workArea.width - 60.0, workArea.y, 60.0, 60.0};
        case eScreenEdge::BOTTOM_LEFT:
            return CBox{workArea.x, workArea.y + workArea.height - 60.0, 60.0, 60.0};
        case eScreenEdge::BOTTOM_RIGHT:
            return CBox{workArea.x + workArea.width - 60.0, workArea.y + workArea.height - 60.0, 60.0, 60.0};
        default:
            return {};
    }
}

static void applySnapBox(PHLWINDOW window, const CBox& targetBox) {
    if (!Desktop::View::validMapped(window))
        return;

    g_pEventLoopManager->doLater([window, targetBox]() {
        if (!Desktop::View::validMapped(window))
            return;

        if (Fullscreen::controller()->isFullscreen(window)) {
            Fullscreen::controller()->setFullscreenMode(window, Fullscreen::FSMODE_NONE);
        }

        if (!isWindowFloating(window)) {
            g_layoutManager->changeFloatingMode(window->layoutTarget());
        }

        window->setBox(targetBox);
        window->sendWindowSize(true);
        g_pHyprRenderer->damageWindow(window);
    });
}

bool executeAction(const std::string& action, PHLWINDOW window, PHLMONITOR pMonitor, const CBox& targetBox) {
    if (action.empty() || action == "none")
        return true;

    std::string actLower = action;
    std::ranges::transform(actLower, actLower.begin(), [](unsigned char c) { return std::tolower(c); });

    // 1. Fullscreen / maximize
    if (actLower == "fullscreen" || actLower == "toggle_fullscreen" || actLower == "togglefullscreen") {
        g_pEventLoopManager->doLater([window]() {
            if (!Desktop::View::validMapped(window))
                return;
            Fullscreen::controller()->setFullscreenMode(window, Fullscreen::FSMODE_FULLSCREEN);
            g_pHyprRenderer->damageWindow(window);
        });
        return true;
    }

    if (actLower == "maximize" || actLower == "maximized") {
        g_pEventLoopManager->doLater([window]() {
            if (!Desktop::View::validMapped(window))
                return;
            Fullscreen::controller()->setFullscreenMode(window, Fullscreen::FSMODE_MAXIMIZED);
            g_pHyprRenderer->damageWindow(window);
        });
        return true;
    }

    // 2. Toggle Float
    if (actLower == "toggle_float" || actLower == "togglefloating" || actLower == "float") {
        g_pEventLoopManager->doLater([window]() {
            if (!Desktop::View::validMapped(window))
                return;
            g_layoutManager->changeFloatingMode(window->layoutTarget());
            g_pHyprRenderer->damageWindow(window);
        });
        return true;
    }

    // 3. Geometric Snapping
    if (actLower.starts_with("snap_") || actLower == "left" || actLower == "right" ||
        actLower == "top_left" || actLower == "top_right" || actLower == "bottom_left" || actLower == "bottom_right") {
        applySnapBox(window, targetBox);
        return true;
    }

    // 4. Fechar Janela
    if (actLower == "closewindow" || actLower == "close_window" || actLower == "killactive") {
        g_pEventLoopManager->doLater([window]() {
            if (!Desktop::View::validMapped(window))
                return;
            HyprlandAPI::invokeHyprctlCommand("dispatch", "closewindow");
        });
        return true;
    }

    // 5. Dispatcher arbitrário do Hyprland ou comando exec
    std::string dispatchCmd = action;
    if (dispatchCmd.starts_with("dispatch ")) {
        dispatchCmd = dispatchCmd.substr(9);
    }

    g_pEventLoopManager->doLater([dispatchCmd]() {
        HyprlandAPI::invokeHyprctlCommand("dispatch", dispatchCmd);
    });

    return true;
}

bool dispatchEdgeDropAction(PHLWINDOW window, const Vector2D& mouseCoords) {
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

    const std::string action    = getActionForEdge(edge);
    const CBox        targetBox = getTargetBoxForAction(action, edge, pMonitor);

    return executeAction(action, window, pMonitor, targetBox);
}
