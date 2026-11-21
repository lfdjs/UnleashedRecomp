#pragma once

#include <portable/guest_address.h>
#include <cstdint>

#if defined(__SWITCH__)
#include <os/switch_atomic_wait.h>
#endif

#ifndef _WIN32
#define MEM_COMMIT  0x00001000  
#define MEM_RESERVE 0x00002000  
#endif

struct Memory
{
    uint8_t* base{};

    Memory();

#if defined(__SWITCH__)
    bool CommitRange(size_t offset, size_t size) noexcept;
    bool CommitHostRange(const void* host, size_t size) noexcept;
#endif

    /**
     * EN: Check host-pointer membership using integer offsets rather than
     *     comparing pointers to unrelated host allocations.
     * PT-BR: Verifica se o ponteiro pertence ao espaco do jogo por offsets
     *     inteiros, sem comparar ponteiros de alocacoes independentes.
     */
    bool IsInMemoryRange(const void* host) const noexcept
    {
        return portable::memory::ContainsHostRange(
            reinterpret_cast<uintptr_t>(base), PPC_MEMORY_SIZE,
            reinterpret_cast<uintptr_t>(host));
    }

    /**
     * EN: Translate a 32-bit guest address into the host reservation.
     *     Offset zero deliberately maps to the guarded first guest page.
     * PT-BR: Traduz um endereco do jogo para a reserva do host.
     *     O offset zero aponta intencionalmente para a pagina protegida.
     */
    void* Translate(size_t offset) const noexcept
    {
        assert(base != nullptr && offset < PPC_MEMORY_SIZE);
        if (base == nullptr || offset >= PPC_MEMORY_SIZE)
            return nullptr;

        return base + offset;
    }

    /**
     * EN: Convert a valid host pointer back to an Xbox guest offset;
     *     null and invalid pointers return zero (with a debug assertion
     *     for invalid non-null addresses).
     * PT-BR: Converte um ponteiro valido para o offset Xbox; ponteiros
     *     nulos ou invalidos retornam zero, com assert no modo debug.
     */
    uint32_t MapVirtual(const void* host) const noexcept
    {
        if (host == nullptr)
            return 0;

        uint32_t offset = 0;
        const bool valid = portable::memory::TryGuestOffset(
            reinterpret_cast<uintptr_t>(base), PPC_MEMORY_SIZE,
            reinterpret_cast<uintptr_t>(host), offset);
        assert(valid);
        return valid ? offset : 0;
    }

    PPCFunc* FindFunction(uint32_t guest) const noexcept
    {
        return PPC_LOOKUP_FUNC(base, guest);
    }

    void InsertFunction(uint32_t guest, PPCFunc* host)
    {
        PPC_LOOKUP_FUNC(base, guest) = host;
    }

#if defined(__SWITCH__)
    struct SwitchCommitChunk
    {
        size_t offset{};
        size_t size{};
        void* backing{};
        void* codeAlias{};
    };

    void* reservation{};
    std::vector<uint8_t> committedPages;
    std::vector<SwitchCommitChunk> switchCommitChunks;
    mutable std::mutex commitMutex;
    uint64_t switchAliasBase{};
    uint64_t switchAliasSize{};
    uint64_t switchAslrBase{};
    uint64_t switchAslrSize{};
    uint64_t switchHeapBase{};
    uint64_t switchHeapSize{};
    uint64_t switchSelectedBase{};
    uint64_t switchCommitFailureOffset{};
    uint64_t switchCommitFailureAddress{};
    uint64_t switchCommitFailureMemoryBase{};
    uint64_t switchCommitFailureMemorySize{};
    uint32_t switchCommitFailureMemoryType{};
    uint32_t switchCommitFailureMemoryAttr{};
    uint32_t switchCommitFailureMemoryPerm{};
    uint32_t switchCommitFailurePageInfo{};
    uint32_t switchInitResult{};
    const char* switchInitFailureReason{"not started"};
    bool switchUsingAliasBaseFallback{};
#endif
};

extern "C" void* MmGetHostAddress(uint32_t ptr);
extern Memory g_memory;
