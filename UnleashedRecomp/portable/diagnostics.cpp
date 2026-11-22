#include "diagnostics.h"

#include "memory.h"
#include "platform.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>

namespace portable::diagnostics
{
    namespace
    {
        /**
         * EN: Convert a possibly null SDL name or error string into printable
         *     text, so diagnostics remain safe when initialization fails.
         * PT-BR: Converte nomes e erros SDL possivelmente nulos em texto
         *     seguro, inclusive quando a inicializacao falha.
         */
        const char* SafeText(const char* text) noexcept
        {
            return text && text[0] != '\0' ? text : "(unavailable)";
        }

        /**
         * EN: Print one machine-searchable check record without relying on
         *     the game's logger, configuration files or graphics pipeline.
         * PT-BR: Registra cada verificacao em formato pesquisavel, sem depender
         *     do logger do jogo, configuracoes ou renderizador.
         */
        void Check(const char* component, bool success, const char* explanation) noexcept
        {
            std::printf("PORTABLE_CHECK|%s|%s|%s\n",
                component, success ? "PASS" : "FAIL", SafeText(explanation));
        }

        /**
         * EN: Print an informational record that does not change exit status;
         *     zero connected controllers is not an error.
         * PT-BR: Registra informacao sem alterar o codigo de retorno;
         *     nenhum controle conectado nao constitui erro.
         */
        void Info(const char* component, const char* detail) noexcept
        {
            std::printf("PORTABLE_INFO|%s|%s\n", component, SafeText(detail));
        }
    }

    int RunHostPreflight(bool guestMemoryReserved, const std::filesystem::path& userDataRoot)
    {
        // EN: Keep preflight isolated from the game's installer, updates and saves.
        // PT-BR: Nao acessa instalador, atualizacoes nem saves durante o diagnostico.
        std::puts("PORTABLE_PREFLIGHT_BEGIN|version=1");
        std::printf("PORTABLE_INFO|target|os=%s cpu=%s host_page=%zu\n",
            portable::Name(portable::CurrentOS()),
            portable::Name(portable::CurrentCPU()),
            portable::memory::HostPageSize());

        int result = 0;
        Check("guest-memory", guestMemoryReserved,
            guestMemoryReserved ? "guest address space is reserved" : "guest reservation unavailable");
        if (!guestMemoryReserved)
            result |= 1;

        // EN: A non-empty path is sufficient for the read-only preflight;
        //     creation/write permissions are tested during actual save operations.
        // PT-BR: Verifica somente se o caminho esta configurado; permissoes
        //     de escrita devem ser verificadas ao salvar dados reais do jogo.
        const bool validRoot = !userDataRoot.empty();
        Check("user-data", validRoot,
            validRoot ? "user data root configured (not written)" : "user data root missing");
        if (!validRoot)
            result |= 16;

        SDL_version runtime{};
        SDL_GetVersion(&runtime);
        std::printf("PORTABLE_INFO|sdl|runtime=%u.%u.%u\n",
            runtime.major, runtime.minor, runtime.patch);

        if (SDL_Init(0) != 0)
        {
            Info("sdl-init", SDL_GetError());
            result |= 2 | 4 | 8;
            std::printf("PORTABLE_PREFLIGHT_END|exit_code=%d\n", result);
            std::fflush(stdout);
            return result;
        }

        // EN: Test SDL video initialization before asking for the Vulkan loader.
        // PT-BR: Inicializa o video SDL antes de consultar o carregador Vulkan.
        const bool videoReady = SDL_InitSubSystem(SDL_INIT_VIDEO) == 0;
        Check("sdl-video", videoReady,
            videoReady ? SafeText(SDL_GetCurrentVideoDriver()) : SDL_GetError());
        if (!videoReady)
            result |= 2;

        if (videoReady)
        {
            // EN: SDL loading libvulkan proves the loader is accessible, but
            //     does not establish Vulkan device features or Plume rendering.
            // PT-BR: Carregar libvulkan prova acesso ao loader, mas nao valida
            //     a GPU, seus recursos nem a renderizacao do Plume.
            const bool vulkanReady = SDL_Vulkan_LoadLibrary(nullptr) == 0;
            Check("vulkan-loader", vulkanReady,
                vulkanReady ? "SDL Vulkan loader available; GPU unverified" : SDL_GetError());
            if (!vulkanReady)
                result |= 4;
            else
                SDL_Vulkan_UnloadLibrary();

            std::printf("PORTABLE_INFO|video|displays=%d\n", SDL_GetNumVideoDisplays());
        }
        else
        {
            Check("vulkan-loader", false, "skipped because SDL video failed");
            result |= 4;
        }

        // EN: This checks SDL's audio subsystem, not speaker output or latency.
        // PT-BR: Testa o subsistema SDL, nao o som nos alto-falantes nem latencia.
        const bool audioReady = SDL_InitSubSystem(SDL_INIT_AUDIO) == 0;
        Check("sdl-audio", audioReady,
            audioReady ? SafeText(SDL_GetCurrentAudioDriver()) : SDL_GetError());
        if (!audioReady)
            result |= 8;
        else
            std::printf("PORTABLE_INFO|audio|playback_devices=%d\n",
                SDL_GetNumAudioDevices(0));

        // EN: A controller is optional; warn on SDL input failure but never
        //     fail solely because no physical gamepad is connected.
        // PT-BR: Controles sao opcionais; a ausencia de gamepad fisico nao
        //     reprova o diagnostico. Falhas do SDL ficam registradas.
        const bool inputReady = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0;
        if (inputReady)
        {
            const int count = SDL_NumJoysticks();
            std::printf("PORTABLE_INFO|controllers|connected=%d\n", count);
            for (int i = 0; i < std::min(count, 8); ++i)
                std::printf("PORTABLE_INFO|controller_%d|%s\n", i,
                    SafeText(SDL_JoystickNameForIndex(i)));
        }
        else
        {
            Info("controllers-unavailable", SDL_GetError());
        }

        // EN: All SDL subsystems started here are released before returning.
        // PT-BR: Libera os subsistemas SDL antes de encerrar o diagnostico.
        SDL_Quit();
        std::printf("PORTABLE_PREFLIGHT_END|exit_code=%d\n", result);
        std::fflush(stdout);
        return result;
    }
}
