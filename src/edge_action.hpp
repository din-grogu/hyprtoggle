#pragma once

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprutils/math/Vector2D.hpp>

enum class eScreenEdge {
    NONE = 0,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

// Identifica se as coordenadas do mouse estão tocando alguma borda do monitor (considerando multi-monitores)
eScreenEdge detectScreenEdge(const Hyprutils::Math::Vector2D& mouseCoords, double threshold);

// Handlers modulares específicos para cada borda
void handleTopEdgeAction(PHLWINDOW window);
void handleBottomEdgeAction(PHLWINDOW window);

// Despachante principal chamado ao soltar o arrasto da janela
bool dispatchEdgeDropAction(PHLWINDOW window, const Hyprutils::Math::Vector2D& mouseCoords);
