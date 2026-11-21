#pragma once
#include <chrono>
#include <thread>

namespace portable::threading {
/** EN: Yield the current task to the platform scheduler.
 *  PT-BR: Cede a execucao da tarefa atual ao escalonador do sistema. */
inline void Yield() noexcept {
    std::this_thread::yield();
}
/** EN: Sleep for a positive duration; zero and negative values are ignored.
 *  PT-BR: Suspende a thread apenas para duracoes positivas. */
inline void SleepFor(std::chrono::nanoseconds duration) {
    if (duration > std::chrono::nanoseconds::zero())
        std::this_thread::sleep_for(duration);
}
} // namespace portable::threading
