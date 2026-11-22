project("UnleashedRecompLib")

set(UNLEASHED_RECOMP_SWITCH_PREGENERATED_SHADER_CACHE_DEFAULT OFF)
if (UNLEASHED_RECOMP_SWITCH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp")
    set(UNLEASHED_RECOMP_SWITCH_PREGENERATED_SHADER_CACHE_DEFAULT ON)
endif()

option(
    UNLEASHED_RECOMP_SWITCH_USE_PREGENERATED_SHADERS
    "Use an existing shader/shader_cache.cpp during Switch builds instead of running XenosRecomp from the target build."
    ${UNLEASHED_RECOMP_SWITCH_PREGENERATED_SHADER_CACHE_DEFAULT}
)

# EN: When a user has already generated the PPC C++ and shader cache from
#     their own licensed game copy, permit later UI/runtime-only rebuilds
#     without rerunning the proprietary-data extractors. This is strictly
#     opt-in and never creates or downloads game content.
# PT-BR: Permite recompilar apenas interface/runtime usando PPC e shaders
#     gerados previamente a partir da propria copia licenciada do jogo.
#     O modo e opcional e nunca cria ou baixa arquivos do jogo.
option(UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES
    "Reuse existing generated PPC C++ and shader cache without requiring original Xbox 360 build inputs."
    OFF)

if(UNLEASHED_RECOMP_SWITCH AND UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES)
    message(FATAL_ERROR
        "UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES is for host desktop builds only. "
        "For Switch use UNLEASHED_RECOMP_SWITCH_USE_PREGENERATED_SHADERS and the Switch build toolchain.")
endif()

add_compile_options(
    -fno-strict-aliasing
)

if (WIN32)
    add_compile_options(/fp:strict)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-ffp-model=strict>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-frounding-math>)
endif()

if (UNLEASHED_RECOMP_SWITCH)
    set(UNLEASHED_RECOMP_HOST_TOOLS_DIR "${CMAKE_SOURCE_DIR}/build/host-tools" CACHE PATH "Directory containing host-side generation tools.")
    set(UNLEASHED_RECOMP_HOST_X_DECOMPRESS "${UNLEASHED_RECOMP_HOST_TOOLS_DIR}/tools/x_decompress/x_decompress" CACHE FILEPATH "Host x_decompress executable.")
    set(UNLEASHED_RECOMP_HOST_XENON_RECOMP "${UNLEASHED_RECOMP_HOST_TOOLS_DIR}/tools/XenonRecomp/XenonRecomp/XenonRecomp" CACHE FILEPATH "Host XenonRecomp executable.")
    set(UNLEASHED_RECOMP_HOST_XENOS_RECOMP "${UNLEASHED_RECOMP_HOST_TOOLS_DIR}/tools/XenosRecomp/XenosRecomp/XenosRecomp" CACHE FILEPATH "Host XenosRecomp executable.")
    set(UNLEASHED_RECOMP_XENON_RECOMP_COMMAND
        "${UNLEASHED_RECOMP_HOST_XENON_RECOMP}"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml"
        "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h"
    )
    set(UNLEASHED_RECOMP_XENON_RECOMP_DEPENDS "${UNLEASHED_RECOMP_HOST_XENON_RECOMP}")
    set(UNLEASHED_RECOMP_X_DECOMPRESS_COMMAND "${UNLEASHED_RECOMP_HOST_X_DECOMPRESS}")
    set(UNLEASHED_RECOMP_X_DECOMPRESS_DEPENDS "${UNLEASHED_RECOMP_HOST_X_DECOMPRESS}")
    set(UNLEASHED_RECOMP_XENOS_RECOMP_COMMAND
        "${UNLEASHED_RECOMP_HOST_XENOS_RECOMP}"
        "${CMAKE_CURRENT_SOURCE_DIR}/private"
        "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
        "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenosRecomp/XenosRecomp/shader_common.h"
    )
    set(UNLEASHED_RECOMP_XENOS_RECOMP_DEPENDS "${UNLEASHED_RECOMP_HOST_XENOS_RECOMP}")
else()
    target_compile_definitions(XenonRecomp PRIVATE
        XENON_RECOMP_CONFIG_FILE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml\"
        XENON_RECOMP_HEADER_FILE_PATH=\"${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h\")
    set(UNLEASHED_RECOMP_XENON_RECOMP_COMMAND "$<TARGET_FILE:XenonRecomp>")
    set(UNLEASHED_RECOMP_X_DECOMPRESS_COMMAND "$<TARGET_FILE:x_decompress>")
    set(UNLEASHED_RECOMP_XENOS_RECOMP_COMMAND "$<TARGET_FILE:XenosRecomp>")
endif()

set(UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_func_mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp_shared.h"
)

foreach(i RANGE 0 260)
    list(APPEND UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp.${i}.cpp")
endforeach()

if(UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES)
    # EN: Check every generated file before configuring the target. Merely
    #     having an old game executable is not proof that generated sources
    #     still exist. Never create placeholder guest code or shaders.
    # PT-BR: Confere TODOS os fontes gerados antes de configurar o target.
    #     Um executavel antigo nao prova que os .cpp ainda existam.
    #     Nao cria arquivos PPC/shader ficticios.
    include("${CMAKE_SOURCE_DIR}/cmake/VerifyPregeneratedGuestSources.cmake")
    unleashed_recomp_verify_pregenerated_guest_sources(
        ${UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.h"
    )
    message(STATUS
        "Reusing previously generated PPC sources and shader cache (no original Xbox 360 input files required).")
    message(WARNING
        "Reusing old PPC/shader sources: guest recompiler settings and game assets will NOT be regenerated. "
        "Disable UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES when updating SWA.toml, "
        "recompiler tools, or guest game data.")
endif()

if(NOT UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES)
    # EN: Normal build: regenerate guest C++ from the user's game assets.
    # PT-BR: Build normal: regenera C++ a partir dos arquivos do usuario.
add_custom_command(
    OUTPUT
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default_patched.xex"
        ${UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES}
    COMMAND
        ${UNLEASHED_RECOMP_XENON_RECOMP_COMMAND}
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xex"
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xexp"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml"
        "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h"
        ${UNLEASHED_RECOMP_XENON_RECOMP_DEPENDS}
    USES_TERMINAL
)
endif()

if(NOT UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES)
    # EN: Decompress source shaders only when regenerating guest artifacts.
    # PT-BR: Descomprime shaders somente ao regenerar artefatos guest.
add_custom_command(
    OUTPUT
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar"
    COMMAND
        ${UNLEASHED_RECOMP_X_DECOMPRESS_COMMAND} "${CMAKE_CURRENT_SOURCE_DIR}/private/shader.ar" "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shader.ar"
        ${UNLEASHED_RECOMP_X_DECOMPRESS_DEPENDS}
)
endif()

set(XENOS_RECOMP_ROOT "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenosRecomp/XenosRecomp")
set(XENOS_RECOMP_INCLUDE "${XENOS_RECOMP_ROOT}/shader_common.h")

file(GLOB XENOS_RECOMP_SOURCES
    "${XENOS_RECOMP_ROOT}/*.h"
    "${XENOS_RECOMP_ROOT}/*.cpp"
)

if (UNLEASHED_RECOMP_SWITCH AND UNLEASHED_RECOMP_SWITCH_USE_PREGENERATED_SHADERS)
    message(STATUS "Using pre-generated Switch shader cache: ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp")
elseif(UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES)
    # EN: Keep a previously generated shader_cache.cpp as a normal source.
    #     Do not rerun XenosRecomp or require default_patched.xex/shader.ar.
    # PT-BR: Compila shader_cache.cpp existente sem acionar XenosRecomp
    #     nem exigir default_patched.xex ou shader.ar.
    message(STATUS "Using pre-generated desktop shader cache: ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp")
else()
    target_compile_definitions(XenosRecomp PRIVATE
        XENOS_RECOMP_INPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/private\"
        XENOS_RECOMP_OUTPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp\"
        XENOS_RECOMP_INCLUDE_INPUT=\"${XENOS_RECOMP_INCLUDE}\"
        UNLEASHED_RECOMP
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
        COMMAND
            ${UNLEASHED_RECOMP_XENOS_RECOMP_COMMAND}
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/private/default_patched.xex"
            "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar"
            ${XENOS_RECOMP_SOURCES}
            ${XENOS_RECOMP_INCLUDE}
            ${UNLEASHED_RECOMP_XENOS_RECOMP_DEPENDS}
        USES_TERMINAL
    )
endif()

add_library(UnleashedRecompLib 
    ${UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES}
    "shader/shader_cache.h"
    "shader/shader_cache.cpp"
)

target_include_directories(UnleashedRecompLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(UnleashedRecompLib PRIVATE "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/thirdparty/simde")
target_precompile_headers(UnleashedRecompLib PUBLIC "ppc/ppc_recomp_shared.h")
