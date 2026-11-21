#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace portable::memory
{
    /**
     * EN: Check that a host address and all requested bytes belong to a
     *     contiguous guest-memory reservation. Integer offsets avoid undefined
     *     relational comparisons between unrelated C++ pointers.
     * PT-BR: Verifica se o endereco host e todos os bytes solicitados pertencem
     *     a reserva contigua do jogo. Deslocamentos inteiros evitam comparacoes
     *     indefinidas entre ponteiros C++ de objetos distintos.
     */
    constexpr bool ContainsHostRange(std::uintptr_t base, std::size_t regionBytes,
        std::uintptr_t address, std::size_t requestedBytes = 1) noexcept
    {
        constexpr auto maxAddress = std::numeric_limits<std::uintptr_t>::max();

        if (!base || !regionBytes || !address || !requestedBytes ||
            regionBytes > maxAddress - base || address < base)
            return false;

        const auto offset = address - base;
        return offset < regionBytes && requestedBytes <= regionBytes - offset;
    }

    /**
     * EN: Translate an address inside the guest reservation to a 32-bit Xbox
     *     guest offset. Return false for missing pointers, invalid ranges,
     *     or offsets larger than the emulated 32-bit address space.
     * PT-BR: Traduz um endereco da reserva para o deslocamento Xbox de 32 bits.
     *     Retorna falso para ponteiros ausentes, intervalos invalidos ou
     *     deslocamentos fora do espaco de enderecamento do console.
     */
    constexpr bool TryGuestOffset(std::uintptr_t base, std::size_t regionBytes,
        std::uintptr_t address, std::uint32_t& result) noexcept
    {
        if (!ContainsHostRange(base, regionBytes, address))
            return false;

        const auto offset = address - base;
        if (offset > std::numeric_limits<std::uint32_t>::max())
            return false;

        result = static_cast<std::uint32_t>(offset);
        return true;
    }
}
