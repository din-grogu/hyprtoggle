#pragma once

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/output/Monitor.hpp>
#include <hyprutils/math/Vector2D.hpp>

enum class eScreenEdge {
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

// Identifica se as coordenadas do mouse estão tocando alguma borda ou canto do monitor (considerando multi-monitores)
eScreenEdge detectScreenEdge(const Hyprutils::Math::Vector2D& mouseCoords, double edgeThreshold, double cornerThreshold);

// Calcula o retângulo de destino (CBox) para uma determinada borda/canto no monitor
CBox getTargetBoxForEdge(eScreenEdge edge, PHLMONITOR pMonitor);

// Handlers modulares específicos para cada borda
void handleTopEdgeAction(PHLWINDOW window);
void handleBottomEdgeAction(PHLWINDOW window);
void handleSnapBoxAction(PHLWINDOW window, PHLMONITOR pMonitor, const CBox& targetBox);

// Despachante principal chamado ao soltar o arrasto da janela
bool dispatchEdgeDropAction(PHLWINDOW window, const Hyprutils::Math::Vector2D& mouseCoords);
