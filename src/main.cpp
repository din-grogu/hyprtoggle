#include <hyprland/src/plugins/PluginAPI.hpp>
#include <unistd.h>
#include <vector>
#include <string_view>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/layout/supplementary/DragController.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#if __has_include(<hyprland/src/desktop/view/Window.hpp>)
#include <hyprland/src/desktop/view/Window.hpp>
#elif __has_include(<hyprland/src/desktop/view/window/Window.hpp>)
#include <hyprland/src/desktop/view/window/Window.hpp>
#endif

#include "globals.hpp"
#include "edge_action.hpp"

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

using DragEndReturn_t = decltype(std::declval<Layout::Supplementary::CDragStateController>().dragEnd());
typedef DragEndReturn_t (*origDragEnd)(void* thisptr);

template <typename T>
T callOriginalAndHandle(void* thisptr, bool isMoveDrag, PHLWINDOW draggingWindow, const Hyprutils::Math::Vector2D& mouseCoords) {
    typedef T (*origFn)(void* thisptr);
    if constexpr (std::is_same_v<T, void>) {
        (*(origFn)g_pDragEndHook->m_original)(thisptr);
        if (isMoveDrag && Desktop::View::validMapped(draggingWindow)) {
            dispatchEdgeDropAction(draggingWindow, mouseCoords);
        }
    } else {
        T res = (*(origFn)g_pDragEndHook->m_original)(thisptr);
        if (isMoveDrag && Desktop::View::validMapped(draggingWindow)) {
            dispatchEdgeDropAction(draggingWindow, mouseCoords);
        }
        return res;
    }
}

DragEndReturn_t hkDragEnd(void* thisptr) {
    if (!isPluginEnabled())
        return (*(origDragEnd)g_pDragEndHook->m_original)(thisptr);

    // 1. Pré-Drop: inspeciona o alvo de arrasto antes do dragEnd resetar os ponteiros internos
    auto target = g_layoutManager->dragController()->target();
    auto mode   = g_layoutManager->dragController()->mode();

    PHLWINDOW  draggingWindow = target ? target->window() : nullptr;
    const auto mouseCoords    = g_pInputManager->getMouseCoordsInternal();

    const bool isMoveDrag = (mode == MBIND_MOVE && draggingWindow != nullptr);

    // 2. Executa o dragEnd original do Hyprland e despacha ação
    return callOriginalAndHandle<DragEndReturn_t>(thisptr, isMoveDrag, draggingWindow, mouseCoords);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string COMPOSITOR_HASH = __hyprland_api_get_hash();
    const std::string CLIENT_HASH     = __hyprland_api_get_client_hash();

    if (COMPOSITOR_HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprtoggle] Mismatched headers! Can't proceed.",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprtoggle] Version mismatch: compositor (" + COMPOSITOR_HASH + ") != client (" + CLIENT_HASH + ")");
    }

    // Registra variáveis de configuração estruturadas compatíveis com Lua (hl.config)
    g_config.enabled   = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:enabled", "Ativa ou desativa o plugin (1 = ativado, 0 = desativado)", 1,
                                                               Config::Values::SIntValueOptions{.min = 0, .max = 1});
    g_config.mode      = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:mode", "Modo ao soltar no topo: 1 = maximizar, 2 = tela cheia real", 2,
                                                               Config::Values::SIntValueOptions{.min = 1, .max = 2});
    g_config.threshold = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:threshold", "Distância em pixels da borda do monitor para acionar", 25,
                                                               Config::Values::SIntValueOptions{.min = 1, .max = 100});

    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.enabled);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.mode);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.threshold);

    // Localiza e instala o hook em CDragStateController::dragEnd
    auto FNS = HyprlandAPI::findFunctionsByName(PHANDLE, "dragEnd");
    for (auto& fn : FNS) {
        if (!fn.demangled.contains("CDragStateController"))
            continue;

        g_pDragEndHook = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void*)::hkDragEnd);
        break;
    }

    if (!g_pDragEndHook || !g_pDragEndHook->hook()) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprtoggle] Falha ao instalar o hook em CDragStateController::dragEnd!", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprtoggle] Falha na instalação do hook");
    }

    HyprlandAPI::addNotification(PHANDLE, "[hyprtoggle] Plugin carregado com sucesso!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 4000);

    return {"hyprtoggle", "Drag windows to screen edges to toggle fullscreen or floating", "Ian Dieb", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_pDragEndHook = nullptr;
}
