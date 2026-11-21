#pragma once
#include <cstdint>

namespace portable::audio {
/** EN: Count complete PCM blocks queued by a platform-specific audio backend.
 *  PT-BR: Conta blocos PCM completos presentes na fila do driver de audio. */
constexpr uint64_t QueuedBlocks(uint64_t bytes, uint32_t channels,
                                uint32_t samplesPerBlock, uint32_t bytesPerSample) noexcept {
    const uint64_t blockBytes = uint64_t(channels) * samplesPerBlock * bytesPerSample;
    return blockBytes == 0 ? 0 : bytes / blockBytes;
}
/** EN: Prevent runaway refill when a device reports an invalid format.
 *  PT-BR: Evita reposicao infinita quando o dispositivo informa formato invalido. */
constexpr bool NeedsRefill(uint64_t bytes, uint32_t channels, uint32_t samplesPerBlock,
                           uint32_t bytesPerSample, uint64_t maxBlocks) noexcept {
    return channels != 0 && samplesPerBlock != 0 && bytesPerSample != 0 &&
           QueuedBlocks(bytes, channels, samplesPerBlock, bytesPerSample) <= maxBlocks;
}
} // namespace portable::audio
