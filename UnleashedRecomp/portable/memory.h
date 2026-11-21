#pragma once
#include <cstddef>
#include <cstdint>

namespace portable::memory {
/** EN: Query native VM page granularity for safe mprotect/VirtualProtect calls.
 *  PT-BR: Consulta o tamanho nativo de pagina para protecoes seguras de memoria. */
size_t HostPageSize() noexcept;

struct Region {
    void* base = nullptr;
    size_t size = 0;
};
/** EN: Reserve a contiguous virtual guest-memory region on desktop targets.
 *  EN: Switch retains its existing libnx reserve/commit implementation.
 *  PT-BR: Reserva memoria virtual continua em sistemas desktop.
 *  PT-BR: O Switch mantem seu proprio mecanismo de reserva/commit via libnx. */
Region AllocateDesktop(size_t size, uintptr_t preferred = 0x100000000ULL) noexcept;
/** EN: Make guest address zero inaccessible, catching invalid guest pointers.
 *  PT-BR: Torna o endereco zero do jogo inacessivel para detectar ponteiros invalidos. */
bool ProtectNull(Region region, size_t pageBytes) noexcept;
/** EN: Release the virtual region; call only on regions from AllocateDesktop.
 *  PT-BR: Libera a regiao virtual criada por AllocateDesktop. */
void ReleaseDesktop(Region region) noexcept;
} // namespace portable::memory
