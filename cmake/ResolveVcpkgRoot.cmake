# EN: Restore VCPKG_ROOT when Ninja/CMake reconfigure without a preset.
#     A valid caller-provided environment variable always takes precedence.
#     No external package manager, game data or installed dependencies are
#     modified by this resolver.
# PT-BR: Restaura VCPKG_ROOT quando o Ninja/CMake regeneram sem preset.
#     Uma variavel do ambiente fornecida pelo usuario tem prioridade.
#     Este modulo nao instala programas nem modifica arquivos do jogo.
if(NOT DEFINED ENV{VCPKG_ROOT} OR "$ENV{VCPKG_ROOT}" STREQUAL "")
    set(_unleashed_vcpkg_root "")

    # EN: Prefer the vcpkg toolchain already recorded in CMakeCache.txt.
    #     This preserves custom external vcpkg installations across rebuilds.
    # PT-BR: Prioriza o toolchain vcpkg ja salvo em CMakeCache.txt, inclusive
    #     quando o usuario utiliza uma instalacao externa ao repositorio.
    if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT "${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        get_filename_component(_unleashed_toolchain_file
            "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
        get_filename_component(_unleashed_buildsystems_dir
            "${_unleashed_toolchain_file}" DIRECTORY)
        get_filename_component(_unleashed_scripts_dir
            "${_unleashed_buildsystems_dir}" DIRECTORY)
        get_filename_component(_unleashed_candidate
            "${_unleashed_scripts_dir}" DIRECTORY)
        if(_unleashed_toolchain_file STREQUAL
           "${_unleashed_candidate}/scripts/buildsystems/vcpkg.cmake"
           AND EXISTS "${_unleashed_toolchain_file}")
            set(_unleashed_vcpkg_root "${_unleashed_candidate}")
        endif()
    endif()

    # EN: Without a cached vcpkg toolchain, match the bundled preset default.
    # PT-BR: Sem toolchain vcpkg em cache, usa o submodulo previsto no preset.
    if(_unleashed_vcpkg_root STREQUAL "")
        set(_unleashed_candidate "${CMAKE_SOURCE_DIR}/thirdparty/vcpkg")
        if(EXISTS "${_unleashed_candidate}/scripts/buildsystems/vcpkg.cmake")
            set(_unleashed_vcpkg_root "${_unleashed_candidate}")
        endif()
    endif()

    # EN: Fail with actionable guidance only if neither root is usable.
    # PT-BR: Falha com orientacao apenas se nenhum caminho valido existir.
    if(_unleashed_vcpkg_root STREQUAL "")
        message(FATAL_ERROR
            "VCPKG_ROOT is unset and no vcpkg toolchain was found. "
            "Initialize the bundled submodule with "
            "'git submodule update --init --recursive thirdparty/vcpkg' "
            "or configure with a valid VCPKG_ROOT/CMAKE_TOOLCHAIN_FILE.")
    endif()

    # EN: Export for this CMake process and its child commands, including
    #     the vcpkg manifest installation performed during regeneration.
    # PT-BR: Exporta para este processo CMake e seus subprocessos, inclusive
    #     instalacao por manifesto executada na regeneracao do Ninja.
    set(ENV{VCPKG_ROOT} "${_unleashed_vcpkg_root}")
    message(STATUS "Recovered VCPKG_ROOT for incremental CMake configure: $ENV{VCPKG_ROOT}")

    unset(_unleashed_candidate)
    unset(_unleashed_vcpkg_root)
    unset(_unleashed_toolchain_file)
    unset(_unleashed_buildsystems_dir)
    unset(_unleashed_scripts_dir)
endif()
