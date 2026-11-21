#pragma once
#include <cstdint>
#include <limits>

namespace portable::audio
{
    /**
     * EN: Compute the byte size of one interleaved PCM block without overflowing
     *     uint64_t. Return zero for empty formats or impossible block sizes.
     * PT-BR: Calcula os bytes de um bloco PCM intercalado sem overflow de
     *     uint64_t. Retorna zero para formatos vazios ou tamanhos impossiveis.
     */
    constexpr std::uint64_t BytesPerBlock(std::uint32_t channels,
        std::uint32_t samplesPerBlock, std::uint32_t bytesPerSample) noexcept
    {
        if (!channels || !samplesPerBlock || !bytesPerSample)
            return 0;

        const std::uint64_t samplesBytes =
            std::uint64_t(samplesPerBlock) * bytesPerSample;

        if (samplesBytes > std::numeric_limits<std::uint64_t>::max() / channels)
            return 0;

        return samplesBytes * channels;
    }

    /**
     * EN: Count only complete PCM blocks in the platform audio queue.
     *     Ignore incomplete trailing bytes and invalid formats.
     * PT-BR: Conta apenas blocos PCM completos na fila de audio.
     *     Ignora bytes incompletos e formatos invalidos.
     */
    constexpr std::uint64_t QueuedBlocks(std::uint64_t queuedBytes,
        std::uint32_t channels, std::uint32_t samplesPerBlock,
        std::uint32_t bytesPerSample) noexcept
    {
        const auto blockBytes = BytesPerBlock(channels, samplesPerBlock, bytesPerSample);
        return blockBytes ? queuedBytes / blockBytes : 0;
    }

    /**
     * EN: Request more PCM when queued blocks are below/equal to the limit;
     *     never refill for invalid formats or arithmetic overflow.
     * PT-BR: Solicita mais PCM quando a fila esta no limite ou abaixo dele;
     *     nunca repoe audio se o formato e invalido ou houve overflow.
     */
    constexpr bool NeedsRefill(std::uint64_t queuedBytes,
        std::uint32_t channels, std::uint32_t samplesPerBlock,
        std::uint32_t bytesPerSample, std::uint64_t maximumBlocks) noexcept
    {
        const auto blockBytes = BytesPerBlock(channels, samplesPerBlock, bytesPerSample);
        return blockBytes != 0 && queuedBytes / blockBytes <= maximumBlocks;
    }
}
