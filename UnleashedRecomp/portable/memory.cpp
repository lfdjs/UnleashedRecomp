#include "memory.h"
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif !defined(__SWITCH__) && !defined(UNLEASHED_RECOMP_SWITCH)
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace portable::memory {
size_t HostPageSize() noexcept {
#if defined(_WIN32)
    SYSTEM_INFO info{};
    GetSystemInfo(&info);
    return info.dwPageSize;
#elif defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    // EN: Host-page data is supplied by libnx for platform-specific mapping.
    // PT-BR: O libnx fornece o tamanho das paginas ao mapeador do Switch.
    return 0;
#else
    const long size = sysconf(_SC_PAGESIZE);
    return size > 0 ? static_cast<size_t>(size) : 0;
#endif
}
Region AllocateDesktop(size_t size, uintptr_t preferred) noexcept {
    if (!size) return {};
#if defined(_WIN32)
    // EN: Preserve the previous Windows commit semantics (no behavior change).
    // PT-BR: Mantem o comportamento anterior de commit no Windows.
    void* p = VirtualAlloc(reinterpret_cast<void*>(preferred), size,
                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!p) p = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return {p, p ? size : 0};
#elif defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    // EN: libnx owns the Switch mapping; this desktop API must not be used there.
    // PT-BR: O libnx gerencia o Switch; esta API desktop nao deve ser usada nele.
    (void)preferred;
    return {};
#else
    void* p = mmap(reinterpret_cast<void*>(preferred), size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        p = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // EN: MAP_FAILED is NOT nullptr; check both attempts before dereferencing.
    // PT-BR: MAP_FAILED NAO e nullptr; valide ambas as tentativas.
    if (p == MAP_FAILED) return {};
    return {p, size};
#endif
}
bool ProtectNull(Region region, size_t pageBytes) noexcept {
    // EN: Never round up a guest guard over valid low guest addresses.
    // PT-BR: Nunca arredonde a protecao sobre enderecos baixos validos do jogo.
    const size_t hostPage = HostPageSize();
    if (!region.base || !hostPage || pageBytes == 0 ||
        pageBytes > region.size || (pageBytes % hostPage) != 0)
        return false;
#if defined(_WIN32)
    DWORD oldProtection = 0;
    return VirtualProtect(region.base, pageBytes, PAGE_NOACCESS, &oldProtection) != 0;
#elif defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    return false;
#else
    return mprotect(region.base, pageBytes, PROT_NONE) == 0;
#endif
}
void ReleaseDesktop(Region region) noexcept {
    if (!region.base) return;
#if defined(_WIN32)
    VirtualFree(region.base, 0, MEM_RELEASE);
#elif defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    (void)region;
#else
    munmap(region.base, region.size);
#endif
}
} // namespace portable::memory
