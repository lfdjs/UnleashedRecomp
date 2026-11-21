#include <stdafx.h>
#include "guest_thread.h"
#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/function.h>
#include <os/logger.h>
#include "ppc_context.h"
#include <cstring>
#include <new>
#if defined(__APPLE__)
#include <sys/resource.h>
#endif

constexpr size_t PCR_SIZE = 0xAB0;
constexpr size_t TLS_SIZE = 0x100;
constexpr size_t TEB_SIZE = 0x2E0;
constexpr size_t STACK_SIZE = 0x40000;
constexpr size_t TOTAL_SIZE = PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE;

constexpr size_t TEB_OFFSET = PCR_SIZE + TLS_SIZE;

GuestThreadContext::GuestThreadContext(uint32_t cpuNumber)
{
    assert(thread == nullptr);

    // EN: Guest thread state must exist before the context and its stack are initialized.
    // PT-BR: A estrutura e a pilha do jogo devem existir antes de inicializar o contexto.
    thread = (uint8_t*)g_userHeap.Alloc(TOTAL_SIZE);
    if (thread == nullptr)
        throw std::bad_alloc();

    memset(thread, 0, TOTAL_SIZE);

    *(uint32_t*)thread = ByteSwap(g_memory.MapVirtual(thread + PCR_SIZE)); // tls pointer
    *(uint32_t*)(thread + 0x100) = ByteSwap(g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE)); // teb pointer
    *(thread + 0x10C) = cpuNumber;

    *(uint32_t*)(thread + PCR_SIZE + 0x10) = 0xFFFFFFFF; // that one TLS entry that felt quirky
    *(uint32_t*)(thread + PCR_SIZE + TLS_SIZE + 0x14C) = ByteSwap(GuestThread::GetCurrentThreadId()); // thread id

    ppcContext.r1.u64 = g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE); // stack pointer
    ppcContext.r13.u64 = g_memory.MapVirtual(thread);
    ppcContext.fpscr.loadFromHost();

    assert(GetPPCContext() == nullptr);
    SetPPCContext(ppcContext);
}

GuestThreadContext::~GuestThreadContext()
{
    g_userHeap.Free(thread);
}

#ifdef USE_PTHREAD
static size_t GetStackSize()
{
#if defined(__SWITCH__)
    return 8 * 1024 * 1024;
#else
    // EN: Thread-safe function-local initialization avoids a data race when
    //     multiple guest threads are created concurrently on macOS.
    // PT-BR: A inicializacao estatica thread-safe evita data race na criacao
    //     simultanea de threads do jogo no macOS.
    static const size_t stackSize = [] {
        constexpr size_t defaultSize = 8 * 1024 * 1024;
        struct rlimit limit{};
        if (getrlimit(RLIMIT_STACK, &limit) == 0 &&
            limit.rlim_cur != RLIM_INFINITY &&
            limit.rlim_cur >= PTHREAD_STACK_MIN &&
            limit.rlim_cur < defaultSize)
        {
            return static_cast<size_t>(limit.rlim_cur);
        }
        return defaultSize;
    }();

    return stackSize;
#endif
}

static void* GuestThreadFunc(void* arg)
{
    GuestThreadHandle* hThread = (GuestThreadHandle*)arg;
#else
static void GuestThreadFunc(GuestThreadHandle* hThread)
{
#endif
    hThread->WaitUntilResumed();
    GuestThread::Start(hThread->params);
#ifdef USE_PTHREAD
    return nullptr;
#endif
}

GuestThreadHandle::GuestThreadHandle(const GuestThreadParams& params)
    : params(params), suspended((params.flags & 0x1) != 0)
#ifdef USE_PTHREAD
{
    pthread_attr_t attr;
    // EN: Check pthread attribute initialization before using the structure.
    // PT-BR: Verifica a inicializacao dos atributos antes de utiliza-los.
    const auto attrResult = pthread_attr_init(&attr);
    if (attrResult != 0)
    {
        LOGFN_ERROR("pthread_attr_init failed: {}", attrResult);
        return;
    }
    const auto stackResult = pthread_attr_setstacksize(&attr, GetStackSize());
    if (stackResult != 0)
    {
#if defined(__SWITCH__)
        LOGFN_ERROR("pthread_attr_setstacksize failed with error code 0x{:X}.", stackResult);
#else
        fprintf(stderr, "pthread_attr_setstacksize failed with error code 0x%X.\n", stackResult);
#endif
    }

    const auto ret = pthread_create(&thread, &attr, GuestThreadFunc, this);
    pthread_attr_destroy(&attr);
    if (ret != 0) {
#if defined(__SWITCH__)
        LOGFN_ERROR("pthread_create failed with error code 0x{:X}.", ret);
#else
        fprintf(stderr, "pthread_create failed with error code 0x%X.\n", ret);
#endif
        return;
    }

    threadCreated = true;
}
#else
      , thread(GuestThreadFunc, this)
{
}
#endif

#ifdef USE_PTHREAD
/**
 * EN: Serialize pthread_join, avoiding the previous race where the 'joined'
 *     flag became true before the worker actually finished.
 * PT-BR: Serializa pthread_join, evitando a condicao de corrida em que 'joined'
 *     ficava verdadeiro antes da thread realmente encerrar.
 */
void GuestThreadHandle::JoinPthread() noexcept
{
    std::lock_guard lock(joinMutex);
    if (!threadCreated || joined || pthread_equal(thread, pthread_self()))
        return;

    if (pthread_join(thread, nullptr) == 0)
        joined = true;
}
#endif

/**
 * EN: Wait for a created native guest thread before releasing its kernel handle.
 * PT-BR: Aguarda a thread nativa antes de liberar seu handle do kernel.
 */
GuestThreadHandle::~GuestThreadHandle()
{
#ifdef USE_PTHREAD
    JoinPthread();
#else
    if (thread.joinable())
        thread.join();
#endif
}

template <typename ThreadType>
static uint32_t CalcThreadId(const ThreadType& id)
{
    if constexpr (sizeof(id) == 4)
    {
        // EN: memcpy avoids strict-aliasing violations when the native ID is 32-bit.
        // PT-BR: memcpy evita violar o strict aliasing com IDs nativos de 32 bits.
        uint32_t bits = 0;
        std::memcpy(&bits, &id, sizeof(bits));
        return bits;
    }
    else
        return XXH32(&id, sizeof(id), 0);
}

/**
 * EN: Return the native guest thread identifier; zero means pthread creation failed.
 * PT-BR: Retorna o ID da thread nativa; zero indica falha na criacao pthread.
 */
uint32_t GuestThreadHandle::GetThreadId() const
{
#ifdef USE_PTHREAD
    if (!threadCreated)
        return 0;
    return CalcThreadId(thread);
#else
    return CalcThreadId(thread.get_id());
#endif
}

void GuestThreadHandle::Suspend()
{
    suspended.store(true, std::memory_order_release);
}

void GuestThreadHandle::Resume()
{
    suspended.store(false, std::memory_order_release);
    suspendCv.notify_all();
}

void GuestThreadHandle::WaitUntilResumed()
{
    if (!suspended.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(suspendMutex);
    suspendCv.wait(lock, [&]
    {
        return !suspended.load(std::memory_order_acquire);
    });
}

uint32_t GuestThreadHandle::Wait(uint32_t timeout)
{
    assert(timeout == INFINITE);

#ifdef USE_PTHREAD
    JoinPthread();
#else
    if (thread.joinable())
        thread.join();
#endif

    return STATUS_WAIT_0;
}

uint32_t GuestThread::Start(const GuestThreadParams& params)
{
    const auto procMask = (uint8_t)(params.flags >> 24);
    const auto cpuNumber = procMask == 0 ? 0 : 7 - std::countl_zero(procMask);

    GuestThreadContext ctx(cpuNumber);
    ctx.ppcContext.r3.u64 = params.value;

    g_memory.FindFunction(params.function)(ctx.ppcContext, g_memory.base);

    return ctx.ppcContext.r3.u32;
}

GuestThreadHandle* GuestThread::Start(const GuestThreadParams& params, uint32_t* threadId)
{
    auto hThread = CreateKernelObject<GuestThreadHandle>(params);

    if (threadId != nullptr)
    {
        *threadId = hThread->GetThreadId();
    }

    return hThread;
}

uint32_t GuestThread::GetCurrentThreadId()
{
#ifdef USE_PTHREAD
    return CalcThreadId(pthread_self());
#else
    return CalcThreadId(std::this_thread::get_id());
#endif
}

void GuestThread::SetLastError(uint32_t error)
{
    auto* thread = (char*)g_memory.Translate(GetPPCContext()->r13.u32);
    if (*(uint32_t*)(thread + 0x150))
    {
        // Program doesn't want errors
        return;
    }

    // TEB + 0x160 : Win32LastError
    *(uint32_t*)(thread + TEB_OFFSET + 0x160) = ByteSwap(error);
}

#ifdef _WIN32
void GuestThread::SetThreadName(uint32_t threadId, const char* name)
{
#pragma pack(push,8)
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

void SetThreadNameImpl(uint32_t a1, uint32_t threadId, uint32_t* name)
{
#ifdef _WIN32
    GuestThread::SetThreadName(threadId, (const char*)g_memory.Translate(ByteSwap(*name)));
#endif
}

int GetThreadPriorityImpl(GuestThreadHandle* hThread)
{
#ifdef _WIN32
    return GetThreadPriority(hThread == GetKernelObject(CURRENT_THREAD_HANDLE) ? GetCurrentThread() : hThread->thread.native_handle());
#else 
    return 0;
#endif
}

uint32_t SetThreadIdealProcessorImpl(GuestThreadHandle* hThread, uint32_t dwIdealProcessor)
{
    return 0;
}

GUEST_FUNCTION_HOOK(sub_82DFA2E8, SetThreadNameImpl);
GUEST_FUNCTION_HOOK(sub_82BD57A8, GetThreadPriorityImpl);
GUEST_FUNCTION_HOOK(sub_82BD5910, SetThreadIdealProcessorImpl);

GUEST_FUNCTION_STUB(sub_82BD58F8); // Some function that updates the TEB, don't really care since the field is not set
