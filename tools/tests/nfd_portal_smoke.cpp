#include <nfd.h>
#include <cstdio>

/**
 * EN: Confirm that the pinned Linux xdg-desktop-portal implementation can
 *     initialize and shut down over a temporary D-Bus session. No dialog is
 *     displayed and no copyrighted game assets are required.
 * PT-BR: Confirma a inicializacao e encerramento do backend xdg-desktop-portal
 *     usando sessao D-Bus temporaria. Nao abre janelas nem usa dados do jogo.
 */
int main()
{
    const nfdresult_t status = NFD_Init();
    if (status != NFD_OKAY)
    {
        const char* error = NFD_GetError();
        std::fprintf(stderr, "PORTAL_INIT|FAIL|%s\n", error ? error : "unknown error");
        return 1;
    }

    std::puts("PORTAL_INIT|PASS|D-Bus session initialized");
    NFD_Quit();
    return 0;
}
