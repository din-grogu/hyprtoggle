#pragma once

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/output/Monitor.hpp>
#include <hyprutils/math/Vector2D.hpp>
#include <hyprutils/math/Box.hpp>
#include <string>

using Hyprutils::Math::Vector2D;
using Hyprutils::Math::CBox;

enum class eScreenEdge : uint8_t {
    NONE = 0,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

eScreenEdge detectScreenEdge(const Vector2D& mouseCoords, double edgeThreshold, double cornerThreshold);
std::string getActionForEdge(eScreenEdge edge);
CBox        getTargetBoxForAction(const std::string& action, eScreenEdge edge, PHLMONITOR pMonitor);
bool        executeAction(const std::string& action, PHLWINDOW window, PHLMONITOR pMonitor, const CBox& targetBox);
bool        dispatchEdgeDropAction(PHLWINDOW window, const Vector2D& mouseCoords);
