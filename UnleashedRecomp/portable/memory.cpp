#include "memory.h"
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif !defined(__SWITCH__) && !defined(UNLEASHED_RECOMP_SWITCH)
#include <sys/mman.h>
#endif

namespace portable::memory {
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
    if (!region.base || pageBytes == 0 || pageBytes > region.size) return false;
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
