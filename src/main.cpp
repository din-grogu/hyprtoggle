#include <hyprland/src/plugins/PluginAPI.hpp>
#include <unistd.h>
#include <vector>
#include <string_view>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/layout/supplementary/DragController.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#if __has_include(<hyprland/src/desktop/view/Window.hpp>)
#include <hyprland/src/desktop/view/Window.hpp>
#elif __has_include(<hyprland/src/desktop/view/window/Window.hpp>)
#include <hyprland/src/desktop/view/window/Window.hpp>
#endif
#include <hyprland/src/state/MonitorState.hpp>

#include "globals.hpp"
#include "edge_action.hpp"
#include "preview_overlay.hpp"

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
    clearPreview();

    if (!isPluginEnabled())
        return (*(origDragEnd)g_pDragEndHook->m_original)(thisptr);

    // 1. Inspeciona o alvo de arrasto antes do dragEnd resetar os ponteiros internos
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

    // Registra variáveis de configuração estruturadas compatíveis tanto com hyprland.conf quanto com Lua (hl.config)
    g_config.enabled             = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:enabled", "Ativa ou desativa o plugin (1 = ativado, 0 = desativado)", 1,
                                                                         Config::Values::SIntValueOptions{.min = 0, .max = 1});
    g_config.threshold           = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:threshold", "Distância em pixels da borda do monitor para acionar", 20,
                                                                         Config::Values::SIntValueOptions{.min = 1, .max = 100});
    g_config.corner_threshold    = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:corner_threshold", "Tamanho da zona de canto em pixels para acionar", 60,
                                                                         Config::Values::SIntValueOptions{.min = 10, .max = 300});

    // 8 Ações Customizáveis para cada zona da tela
    g_config.action_top          = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_top", "Ação ao soltar no topo", "fullscreen");
    g_config.action_bottom       = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_bottom", "Ação ao soltar na base", "toggle_float");
    g_config.action_left         = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_left", "Ação ao soltar na esquerda", "snap_left");
    g_config.action_right        = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_right", "Ação ao soltar na direita", "snap_right");
    g_config.action_top_left     = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_top_left", "Ação ao soltar no canto superior esquerdo", "snap_top_left");
    g_config.action_top_right    = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_top_right", "Ação ao soltar no canto superior direito", "snap_top_right");
    g_config.action_bottom_left  = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_bottom_left", "Ação ao soltar no canto inferior esquerdo", "snap_bottom_left");
    g_config.action_bottom_right = makeShared<Config::Values::CStringValue>("plugin:hyprtoggle:action_bottom_right", "Ação ao soltar no canto inferior direito", "snap_bottom_right");

    // Preview Visual
    g_config.preview             = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:preview", "Ativa preview visual da área de encaixe (1 = ativado, 0 = desativado)", 1,
                                                                         Config::Values::SIntValueOptions{.min = 0, .max = 1});
    g_config.preview_rounding    = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:preview_rounding", "Arredondamento das bordas do preview", 10,
                                                                         Config::Values::SIntValueOptions{.min = 0, .max = 50});
    g_config.preview_border_size = makeShared<Config::Values::CIntValue>("plugin:hyprtoggle:preview_border_size", "Espessura da borda do preview", 2,
                                                                         Config::Values::SIntValueOptions{.min = 0, .max = 10});

    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.enabled);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.threshold);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.corner_threshold);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_top);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_bottom);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_left);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_right);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_top_left);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_top_right);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_bottom_left);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.action_bottom_right);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.preview);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.preview_rounding);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_config.preview_border_size);

    // Inicializa o listener de renderização do preview overlay
    initPreviewOverlay();

    // Rastreia movimento do cursor durante o arrasto via barramento oficial do Hyprland
    static auto P_MOUSE = Event::bus()->m_events.input.mouse.move.listen([](Vector2D mousePos, Event::SCallbackInfo&) {
        if (!isPluginEnabled())
            return;

        auto mode   = g_layoutManager->dragController()->mode();
        auto target = g_layoutManager->dragController()->target();

        if (mode == MBIND_MOVE || target) {
            const double edgeThresh   = g_config.threshold ? sc<double>(g_config.threshold->value()) : 20.0;
            const double cornerThresh = g_config.corner_threshold ? sc<double>(g_config.corner_threshold->value()) : 60.0;

            const auto edge = detectScreenEdge(mousePos, edgeThresh, cornerThresh);
            if (edge != eScreenEdge::NONE) {
                const auto pMonitor = State::monitorState()->query().vec(mousePos).run();
                if (pMonitor) {
                    const std::string action    = getActionForEdge(edge);
                    const CBox        targetBox = getTargetBoxForAction(action, edge, pMonitor);
                    updatePreview(edge, targetBox, pMonitor);
                    return;
                }
            }
        }

        clearPreview();
    });

    // Hook em CDragStateController::dragEnd para executar a ação ao soltar a janela
    auto FNS_END = HyprlandAPI::findFunctionsByName(PHANDLE, "dragEnd");
    for (auto& fn : FNS_END) {
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

    return {"hyprtoggle", "Drag windows to 8 customizable screen zones to toggle fullscreen, halves, quarters or dispatchers", "Ian Dieb", "0.3"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    clearPreview();
    g_pDragEndHook = nullptr;
}
