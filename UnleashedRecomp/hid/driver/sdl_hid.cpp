#include <stdafx.h>
#include <SDL.h>
#include <user/config.h>
#include <hid/hid.h>
#include <hid/driver/sdl_controller_types.h>
#include <os/logger.h>
#include <ui/game_window.h>
#include <kernel/xdm.h>
#include <app.h>

#define TRANSLATE_INPUT(S, X) SDL_GameControllerGetButton(controller, S) << FirstBitLow(X)
#define VIBRATION_TIMEOUT_MS 5000

class Controller
{
public:
    SDL_GameController* controller{};
    SDL_Joystick* joystick{};
    SDL_JoystickID id{ -1 };
    XAMINPUT_GAMEPAD state{};
    XAMINPUT_VIBRATION vibration{ 0, 0 };

    Controller() = default;

    explicit Controller(int index) : Controller(SDL_GameControllerOpen(index))
    {
    }

    Controller(SDL_GameController* controller) : controller(controller)
    {
        if (!controller)
            return;

        joystick = SDL_GameControllerGetJoystick(controller);
        id = SDL_JoystickInstanceID(joystick);
    }

    SDL_GameControllerType GetControllerType() const
    {
        // EN: SDL device indices change after unplugging another controller.
        //     Query the opened device handle, not the original index.
        // PT-BR: Índices SDL mudam após desconectar outros controles.
        //     Consulta o handle aberto para manter o tipo correto.
        return controller ? SDL_GameControllerGetType(controller)
                          : SDL_CONTROLLER_TYPE_UNKNOWN;
    }

    hid::EInputDevice GetInputDevice() const
    {
        return hid::sdl::PromptFamily(GetControllerType());
    }

    void Close()
    {
        if (!controller)
            return;

        SDL_GameControllerClose(controller);

        controller = nullptr;
        joystick = nullptr;
        id = -1;
    }

    bool CanPoll()
    {
        return controller;
    }

    void PollAxis()
    {
        if (!CanPoll())
            return;

        auto& pad = state;

        pad.sThumbLX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        pad.sThumbLY = ~SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        pad.sThumbRX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        pad.sThumbRY = ~SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);

        pad.bLeftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >> 7;
        pad.bRightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >> 7;
    }

    void Poll()
    {
        if (!CanPoll())
            return;

        auto& pad = state;

        pad.wButtons = 0;

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_UP, XAMINPUT_GAMEPAD_DPAD_UP);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_DOWN, XAMINPUT_GAMEPAD_DPAD_DOWN);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_LEFT, XAMINPUT_GAMEPAD_DPAD_LEFT);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, XAMINPUT_GAMEPAD_DPAD_RIGHT);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_START, XAMINPUT_GAMEPAD_START);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_BACK, XAMINPUT_GAMEPAD_BACK);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_TOUCHPAD, XAMINPUT_GAMEPAD_BACK);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSTICK, XAMINPUT_GAMEPAD_LEFT_THUMB);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSTICK, XAMINPUT_GAMEPAD_RIGHT_THUMB);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, XAMINPUT_GAMEPAD_LEFT_SHOULDER);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, XAMINPUT_GAMEPAD_RIGHT_SHOULDER);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_A, XAMINPUT_GAMEPAD_A);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_B, XAMINPUT_GAMEPAD_B);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_X, XAMINPUT_GAMEPAD_X);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_Y, XAMINPUT_GAMEPAD_Y);
    }

    void SetVibration(const XAMINPUT_VIBRATION& vibration)
    {
        if (!CanPoll())
            return;

        this->vibration = vibration;

        SDL_GameControllerRumble(controller, vibration.wLeftMotorSpeed, vibration.wRightMotorSpeed, VIBRATION_TIMEOUT_MS);
    }

    void SetLED(const uint8_t r, const uint8_t g, const uint8_t b) const
    {
        SDL_GameControllerSetLED(controller, r, g, b);
    }
};

std::array<Controller, 4> g_controllers;
Controller* g_activeController;

inline Controller* EnsureController(uint32_t dwUserIndex)
{
    if (!g_controllers[dwUserIndex].controller)
        return nullptr;

    return &g_controllers[dwUserIndex];
}

inline size_t FindFreeController()
{
    for (size_t i = 0; i < g_controllers.size(); i++)
    {
        if (!g_controllers[i].controller)
            return i;
    }

    return g_controllers.size();
}

inline Controller* FindController(int which)
{
    for (auto& controller : g_controllers)
    {
        if (controller.id == which)
            return &controller;
    }

    return nullptr;
}

static void SetControllerInputDevice(Controller* controller)
{
    g_activeController = controller;

    if (App::s_isLoading)
        return;

    hid::g_inputDevice = controller->GetInputDevice();
    hid::g_inputDeviceController = hid::g_inputDevice;

    // EN: Explicitly map SDL controller models; do not rely on enum numbering.
    // PT-BR: Mapeia os modelos SDL sem depender da ordem das enumerações.
    const auto controllerType = hid::sdl::Identify(controller->GetControllerType());

    if (hid::g_inputDeviceExplicit != controllerType)
    {
        hid::g_inputDeviceExplicit = controllerType;

        LOGFN("Detected controller: {}", hid::GetInputDeviceName());
    }
}

static void SetControllerTimeOfDayLED(Controller& controller, bool isNight)
{
    auto r = isNight ? 22 : 0;
    auto g = isNight ? 0 : 37;
    auto b = isNight ? 101 : 184;

    controller.SetLED(r, g, b);
}

/**
 * EN: Connect an SDL game controller by device index, preserving SDL
 *     instance IDs for hotplug. Avoid duplicates when SDL already emitted
 *     the startup event and reject devices outside the 4-pad pool.
 * PT-BR: Conecta o dispositivo pelo índice SDL, mas guarda o ID de
 *     instância para reconexão. Evita duplicatas na detecção inicial.
 */
static void ConnectController(int deviceIndex)
{
    if (deviceIndex < 0 || !SDL_IsGameController(deviceIndex))
        return;

    const auto instanceId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
    if (instanceId < 0 || FindController(instanceId))
        return;

    const size_t freeIndex = FindFreeController();
    if (freeIndex >= g_controllers.size())
    {
        LOGN_WARNING("Controller detected but all 4 slots are occupied.");
        return;
    }

    Controller fresh(deviceIndex);
    if (!fresh.CanPoll())
    {
        LOGFN_WARNING("Failed to open SDL game controller: {}", SDL_GetError());
        return;
    }

    auto& connected = g_controllers[freeIndex];
    connected = fresh;
    connected.Poll();
    connected.PollAxis();
    SetControllerTimeOfDayLED(connected, App::s_isWerehog);

    // EN: SDL presents a combined left/right Joy-Con as one full controller.
    //     A single Joy-Con is still detected but has limited controls.
    // PT-BR: SDL apresenta Joy-Cons pareados como um controle completo;
    //     um Joy-Con isolado tem comandos limitados.
    LOGFN("Gamepad connected (slot {}, id {}, SDL type {}): {}{}",
        freeIndex, instanceId, static_cast<int>(connected.GetControllerType()),
        SDL_GameControllerName(connected.controller)
            ? SDL_GameControllerName(connected.controller) : "Unknown",
        hid::sdl::HasFullGamepad(connected.GetControllerType())
            ? "" : " [single Joy-Con: limited buttons/sticks]");

    if (!g_activeController)
        SetControllerInputDevice(&connected);
}

/**
 * EN: Close a disconnected controller, release the active pointer and
 *     select another connected controller when available. Never leave
 *     XInput reading a detached pad's cached input state.
 * PT-BR: Desconecta o controle, limpa o ponteiro ativo e escolhe outro
 *     dispositivo conectado, evitando comandos travados do controle antigo.
 */
static void DisconnectController(SDL_JoystickID instanceId)
{
    auto* disconnected = FindController(instanceId);
    if (!disconnected)
        return;

    const bool wasActive = (g_activeController == disconnected);
    disconnected->Close();

    if (wasActive)
    {
        g_activeController = nullptr;
        for (auto& candidate : g_controllers)
        {
            if (candidate.CanPoll())
            {
                SetControllerInputDevice(&candidate);
                break;
            }
        }
    }

    LOGFN("Gamepad disconnected (SDL instance id {}).", instanceId);
}

int HID_OnSDLEvent(void*, SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_CONTROLLERDEVICEADDED:
            ConnectController(event->cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            DisconnectController(event->cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
        {
            // EN: Some drivers change mappings when a gamepad connects
            //     through Bluetooth instead of USB.
            // PT-BR: Drivers podem alterar o mapeamento entre USB e Bluetooth.
            if (auto* controller = FindController(event->cdevice.which))
            {
                controller->Poll();
                controller->PollAxis();
                if (g_activeController == controller)
                    SetControllerInputDevice(controller);
            }
            break;
        }

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERTOUCHPADDOWN:
        case SDL_CONTROLLERTOUCHPADMOTION:
        case SDL_CONTROLLERTOUCHPADUP:
        {
            auto* controller = FindController(event->cdevice.which);

            if (!controller)
                break;

            if (event->type == SDL_CONTROLLERAXISMOTION)
            {
                if (abs(event->caxis.value) > 8000)
                {
                    SDL_ShowCursor(SDL_DISABLE);
                    SetControllerInputDevice(controller);
                }

            }
            else
            {
                SDL_ShowCursor(SDL_DISABLE);
                SetControllerInputDevice(controller);
            }

            // EN: Refresh axes, triggers and buttons together. Touchpads
            //     remain available to the separate world-map listener.
            // PT-BR: Atualiza analógicos, gatilhos e botões em conjunto.
            //     O touchpad também chega ao listener do mapa-múndi.
            controller->Poll();
            controller->PollAxis();

            break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            hid::g_inputDevice = hid::EInputDevice::Keyboard;
            break;

        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            if (!GameWindow::IsFullscreen() || GameWindow::s_isFullscreenCursorVisible)
                SDL_ShowCursor(SDL_ENABLE);

            hid::g_inputDevice = hid::EInputDevice::Mouse;

            break;
        }

        case SDL_WINDOWEVENT:
        {
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                // Stop vibrating controllers on focus lost.
                for (auto& controller : g_controllers)
                    controller.SetVibration({ 0, 0 });
            }

            break;
        }

        case SDL_USER_EVILSONIC:
        {
            for (auto& controller : g_controllers)
                SetControllerTimeOfDayLED(controller, event->user.code);

            break;
        }
    }

    return 0;
}

void hid::Init()
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    // EN: Enable Nintendo HIDAPI (Switch Pro and Joy-Cons) before SDL
    //     initializes the game-controller subsystem. Two Joy-Cons become
    //     a single logical gamepad when both are available.
    // PT-BR: Ativa HIDAPI Nintendo antes do SDL iniciar. Dois Joy-Cons
    //     conectados são combinados em um único controle lógico.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_COMBINE_JOY_CONS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "1");

    // EN: Install the hotplug watcher before SDL enumerates controllers.
    //     The post-init enumeration also covers devices connected at start.
    // PT-BR: Registra eventos antes da detecção; depois procura controles
    //     já conectados ao iniciar o jogo, sem duplicar os IDs.
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
    {
        LOGFN_ERROR("SDL events initialization failed: {}", SDL_GetError());
        return;
    }
    SDL_AddEventWatch(HID_OnSDLEvent, nullptr);
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
    {
        LOGFN_ERROR("SDL game controller initialization failed: {}", SDL_GetError());
        return;
    }

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
        ConnectController(i);
}

uint32_t hid::GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState)
{
    static uint32_t packet;

    if (!pState)
        return ERROR_BAD_ARGUMENTS;

    memset(pState, 0, sizeof(*pState));

    pState->dwPacketNumber = packet++;

    if (dwUserIndex != 0 || !g_activeController ||
        !SDL_GameControllerGetAttached(g_activeController->controller))
        return ERROR_DEVICE_NOT_CONNECTED;

    pState->Gamepad = g_activeController->state;

    return ERROR_SUCCESS;
}

uint32_t hid::SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration)
{
    if (!pVibration)
        return ERROR_BAD_ARGUMENTS;

    if (dwUserIndex != 0 || !g_activeController ||
        !SDL_GameControllerGetAttached(g_activeController->controller))
        return ERROR_DEVICE_NOT_CONNECTED;

    g_activeController->SetVibration(*pVibration);

    return ERROR_SUCCESS;
}

uint32_t hid::GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps)
{
    if (!pCaps)
        return ERROR_BAD_ARGUMENTS;

    if (dwUserIndex != 0 || !g_activeController ||
        !SDL_GameControllerGetAttached(g_activeController->controller))
        return ERROR_DEVICE_NOT_CONNECTED;

    memset(pCaps, 0, sizeof(*pCaps));

    pCaps->Type = XAMINPUT_DEVTYPE_GAMEPAD;
    pCaps->SubType = XAMINPUT_DEVSUBTYPE_GAMEPAD; // TODO: other types?
    pCaps->Flags = 0;
    pCaps->Gamepad = g_activeController->state;
    pCaps->Vibration = g_activeController->vibration;

    return ERROR_SUCCESS;
}
