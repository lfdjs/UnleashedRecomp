#pragma once
#include <filesystem>
#include <string_view>
#include <system_error>

namespace portable::filesystem {
/** EN: Resolve the existing platform-specific user-data location without creating it.
 *  PT-BR: Resolve o local original de dados do usuario sem criar diretorios. */
std::filesystem::path UserDataRoot(std::string_view application);
/** EN: Create a directory hierarchy without propagating file-system exceptions.
 *  PT-BR: Cria diretorios intermediarios sem propagar excecoes de E/S. */
bool EnsureDirectory(const std::filesystem::path& path, std::error_code& error) noexcept;
} // namespace portable::filesystem
