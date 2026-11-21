#pragma once

#include <kernel/xdm.h>
#include <condition_variable>

// Use pthreads directly on macOS to be able to increase default stack size.
#if defined(__APPLE__) || defined(__SWITCH__)

#define USE_PTHREAD 1
#endif

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

#define CURRENT_THREAD_HANDLE uint32_t(-2)

struct GuestThreadContext
{
    PPCContext ppcContext{};
    uint8_t* thread = nullptr;

    GuestThreadContext(uint32_t cpuNumber);
    ~GuestThreadContext();
};

struct GuestThreadParams
{
    uint32_t function;
    uint32_t value;
    uint32_t flags;
};

struct GuestThreadHandle : KernelObject
{
    GuestThreadParams params;
    std::atomic<bool> suspended;
    std::mutex suspendMutex;
    std::condition_variable suspendCv;
#ifdef USE_PTHREAD
    pthread_t thread{};
    bool threadCreated = false;
    // EN: Serialize all pthread_join calls; joined is set only after completion.
    // PT-BR: Serializa pthread_join; joined so muda apos a thread terminar.
    std::mutex joinMutex;
    bool joined = false;
#else
    std::thread thread;
#endif

    GuestThreadHandle(const GuestThreadParams& params);
    ~GuestThreadHandle() override;

#ifdef USE_PTHREAD
    /** EN: Join a native thread at most once, including concurrent Wait calls.
     *  PT-BR: Espera o encerramento da thread apenas uma vez, mesmo com Wait concorrentes. */
    void JoinPthread() noexcept;
#endif

    uint32_t GetThreadId() const;
    void Suspend();
    void Resume();
    void WaitUntilResumed();

    uint32_t Wait(uint32_t timeout) override;
};

struct GuestThread
{
    static uint32_t Start(const GuestThreadParams& params);
    static GuestThreadHandle* Start(const GuestThreadParams& params, uint32_t* threadId);

    static uint32_t GetCurrentThreadId();
    static void SetLastError(uint32_t error);

#ifdef _WIN32
    static void SetThreadName(uint32_t threadId, const char* name);
#endif
};
