cmake_minimum_required (VERSION 3.20)

option(UNLEASHED_RECOMP_HOST_TOOLS_ONLY "Only build host-side generation tools." OFF)

# EN: CMake presets supply VCPKG_ROOT only for the initial configure.
#     Ninja regenerates build.ninja by invoking CMake without the preset
#     environment. Recover the same root from the cached toolchain or the
#     bundled submodule on every configure, before calling project().
# PT-BR: O preset define VCPKG_ROOT apenas na configuracao inicial.
#     O Ninja regenera build.ninja chamando CMake sem o ambiente do preset.
#     Recupera o mesmo vcpkg do toolchain em cache ou do submodulo antes
#     de executar project(), inclusive em builds incrementais.
if(NOT UNLEASHED_RECOMP_SWITCH AND NOT UNLEASHED_RECOMP_HOST_TOOLS_ONLY)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ResolveVcpkgRoot.cmake")
endif()

set(UNLEASHED_RECOMP_THIRDPARTY_ROOT ${CMAKE_SOURCE_DIR}/thirdparty)
set(UNLEASHED_RECOMP_TOOLS_ROOT ${CMAKE_SOURCE_DIR}/tools)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

# EN: Keep CMake option() semantics consistent across third-party subprojects.
#     Older nested projects may reset CMP0077 to unset when calling
#     cmake_minimum_required(); the policy default is inherited without
#     editing any vendored or submodule source files.
# PT-BR: Mantem a semantica de option() consistente nos subprojetos.
#     Bibliotecas antigas podem redefinir CMP0077 ao executar
#     cmake_minimum_required(); este padrao evita alterar o codigo externo.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project("UnleashedRecomp-ALL")

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
if (CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT UNLEASHED_RECOMP_SWITCH)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _IPO_OK OUTPUT _IPO_ERROR)
    if (_IPO_OK)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
        message(STATUS "LTO enabled for Release build")
    else()
        message(STATUS "LTO not available: ${_IPO_ERROR}")
    endif()
endif()

if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

if (CMAKE_OSX_ARCHITECTURES)
    set(UNLEASHED_RECOMP_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
elseif(CMAKE_SYSTEM_PROCESSOR)
    set(UNLEASHED_RECOMP_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
else()
    set(UNLEASHED_RECOMP_ARCHITECTURE ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()
string(TOLOWER "${UNLEASHED_RECOMP_ARCHITECTURE}" UNLEASHED_RECOMP_ARCHITECTURE)
message(STATUS "Detected architecture: ${UNLEASHED_RECOMP_ARCHITECTURE}")

if (UNLEASHED_RECOMP_ARCHITECTURE STREQUAL "x86_64" OR UNLEASHED_RECOMP_ARCHITECTURE STREQUAL "amd64")
    # Target Sandy Bridge for all projects
    add_compile_options(
        -march=sandybridge
    )
endif()

if (UNLEASHED_RECOMP_HOST_TOOLS_ONLY)
    add_subdirectory(${UNLEASHED_RECOMP_TOOLS_ROOT})
    return()
endif()

add_subdirectory(${UNLEASHED_RECOMP_THIRDPARTY_ROOT})
add_subdirectory(${UNLEASHED_RECOMP_TOOLS_ROOT})

# Include sub-projects.
add_subdirectory("UnleashedRecompLib")
add_subdirectory("UnleashedRecomp")
