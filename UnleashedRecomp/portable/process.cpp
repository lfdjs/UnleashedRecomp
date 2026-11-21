#include "process.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace portable::process
{
#if defined(__linux__) && !defined(__ANDROID__)
    namespace
    {
        /**
         * EN: Send a failure code over a close-on-exec pipe and terminate the
         *     forked child without invoking C++ cleanup after fork().
         * PT-BR: Envia o codigo de erro ao processo pai e encerra o filho sem
         *     executar destruidores C++ depois de fork().
         */
        [[noreturn]] void ReportAndExit(int descriptor, int error) noexcept
        {
            // EN: A pipe write of sizeof(int) is atomic, and may be interrupted.
            // PT-BR: A escrita de sizeof(int) e atomica e pode ser interrompida.
            while (write(descriptor, &error, sizeof(error)) < 0 && errno == EINTR) {}
            _exit(127);
        }

        /**
         * EN: Reap the intermediate child, preventing it from becoming a zombie.
         * PT-BR: Recolhe o filho intermediario, evitando um processo zumbi.
         */
        bool ReapIntermediate(pid_t child) noexcept
        {
            int status = 0;
            pid_t result;
            do
            {
                result = waitpid(child, &status, 0);
            }
            while (result < 0 && errno == EINTR);

            return result == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
    }
#endif

    bool StartDetachedLinux(const std::filesystem::path& executable,
        const std::vector<std::string>& arguments,
        const std::filesystem::path& workingDirectory)
    {
#if defined(__linux__) && !defined(__ANDROID__)
        if (executable.empty())
            return false;

        // EN: Build every argument before forking; the child only uses
        //     async-signal-safe POSIX calls and preallocated data.
        // PT-BR: Prepara todos os argumentos antes do fork; o filho usa
        //     chamadas POSIX e dados previamente alocados.
        const std::string executableText = executable.native();
        const std::string workingText = workingDirectory.native();
        std::vector<char*> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(const_cast<char*>(executableText.c_str()));

        for (const auto& argument : arguments)
            argv.push_back(const_cast<char*>(argument.c_str()));
        argv.push_back(nullptr);

        int errors[2] = {-1, -1};
        if (pipe2(errors, O_CLOEXEC) != 0)
            return false;

        const pid_t intermediate = fork();
        if (intermediate < 0)
        {
            close(errors[0]);
            close(errors[1]);
            return false;
        }

        if (intermediate == 0)
        {
            close(errors[0]);
            if (setsid() < 0)
                ReportAndExit(errors[1], errno);

            const pid_t detached = fork();
            if (detached < 0)
                ReportAndExit(errors[1], errno);

            if (detached > 0)
                _exit(0);

            if (!workingText.empty() && chdir(workingText.c_str()) != 0)
                ReportAndExit(errors[1], errno);

            execvp(executableText.c_str(), argv.data());
            ReportAndExit(errors[1], errno);
        }

        // EN: The second fork creates an orphan reaped by the OS init process;
        //     O_CLOEXEC closes the pipe only after a successful execvp.
        // PT-BR: O segundo fork evita zumbis persistentes; O_CLOEXEC fecha
        //     o canal somente quando execvp inicia o executavel corretamente.
        close(errors[1]);
        const bool intermediarySucceeded = ReapIntermediate(intermediate);

        int childError = 0;
        ssize_t readSize;
        do
        {
            readSize = read(errors[0], &childError, sizeof(childError));
        }
        while (readSize < 0 && errno == EINTR);

        close(errors[0]);
        return intermediarySucceeded && readSize == 0;
#else
        // EN: This helper is Linux-specific; use the platform launcher elsewhere.
        // PT-BR: Este auxiliar atende apenas Linux; outros SOs usam seus drivers.
        (void)executable;
        (void)arguments;
        (void)workingDirectory;
        return false;
#endif
    }
}
