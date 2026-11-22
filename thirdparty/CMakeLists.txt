cmake_policy(SET CMP0077 NEW)

set(MSDF_ATLAS_BUILD_STANDALONE OFF)
set(MSDF_ATLAS_USE_SKIA OFF)
set(MSDF_ATLAS_NO_ARTERY_FONT ON)
set(MSDFGEN_DISABLE_PNG ON)

set(SDL2MIXER_DEPS_SHARED OFF)
set(SDL2MIXER_VENDORED ON)
set(SDL2MIXER_FLAC OFF)
set(SDL2MIXER_MOD OFF)
set(SDL2MIXER_MP3 OFF)
set(SDL2MIXER_MIDI OFF)
set(SDL2MIXER_OPUS OFF)
set(SDL2MIXER_VORBIS "VORBISFILE")
set(SDL2MIXER_WAVPACK OFF)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SDL_VULKAN_ENABLED ON CACHE BOOL "")
endif()

if (WIN32)
    set(D3D12_AGILITY_SDK_ENABLED ON CACHE BOOL "")
endif()

add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/msdf-atlas-gen")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/o1heap")

if (UNLEASHED_RECOMP_SWITCH)
    if (NOT DEFINED DEVKITPRO)
        set(DEVKITPRO "/opt/devkitpro")
    endif()

    set(SWITCH_PORTLIBS_ROOT "${DEVKITPRO}/portlibs/switch")
    set(SWITCH_PORTLIBS_INCLUDE "${SWITCH_PORTLIBS_ROOT}/include")
    set(SWITCH_PORTLIBS_LIB "${SWITCH_PORTLIBS_ROOT}/lib")

    if (NOT TARGET SDL2::SDL2-static)
        add_library(SDL2::SDL2-static STATIC IMPORTED GLOBAL)
        set_target_properties(SDL2::SDL2-static PROPERTIES
            IMPORTED_LOCATION "${SWITCH_PORTLIBS_LIB}/libSDL2.a"
            INTERFACE_INCLUDE_DIRECTORIES "${SWITCH_PORTLIBS_INCLUDE}/SDL2"
            INTERFACE_LINK_LIBRARIES "nx;pthread"
        )
    endif()

    if (NOT TARGET SDL2_mixer)
        add_library(SDL2_mixer STATIC IMPORTED GLOBAL)
        set_target_properties(SDL2_mixer PROPERTIES
            IMPORTED_LOCATION "${SWITCH_PORTLIBS_LIB}/libSDL2_mixer.a"
            INTERFACE_INCLUDE_DIRECTORIES "${SWITCH_PORTLIBS_INCLUDE}/SDL2"
            INTERFACE_LINK_LIBRARIES "SDL2::SDL2-static;vorbisidec;modplug;mpg123;opusfile;opus;ogg"
        )
    endif()

    if (NOT TARGET nfd::nfd)
        add_library(nfd_switch_stub STATIC
            "${CMAKE_SOURCE_DIR}/UnleashedRecomp/os/switch/nfd_switch_stub.cpp"
        )
        target_include_directories(nfd_switch_stub PUBLIC
            "${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/nativefiledialog-extended/src/include"
        )
        add_library(nfd::nfd ALIAS nfd_switch_stub)
    endif()
else()
    # EN: Use the desktop portal for Linux file/folder selection by default.
    #     This keeps GTK/GSettings out of the game process; a newer GNOME
    #     schema combined with a GTK library may otherwise abort at NFD_Init.
    #     Set the backend to GTK explicitly if portal support is unavailable.
    # PT-BR: Usa xdg-desktop-portal por padrao no Linux, sem inicializar GTK
    #     diretamente dentro do jogo. GTK permanece uma opcao explicita para
    #     distribuicoes sem suporte ao portal.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(UNLEASHED_RECOMP_LINUX_NFD_BACKEND "PORTAL" CACHE STRING
            "Linux native file dialog backend: PORTAL (recommended) or GTK")
        set_property(CACHE UNLEASHED_RECOMP_LINUX_NFD_BACKEND PROPERTY
            STRINGS PORTAL GTK)
        if(UNLEASHED_RECOMP_LINUX_NFD_BACKEND STREQUAL "PORTAL")
            set(NFD_PORTAL ON CACHE BOOL "Use xdg-desktop-portal" FORCE)
            message(STATUS "UnleashedRecomp native file dialog: xdg-desktop-portal (D-Bus)")
        elseif(UNLEASHED_RECOMP_LINUX_NFD_BACKEND STREQUAL "GTK")
            set(NFD_PORTAL OFF CACHE BOOL "Use xdg-desktop-portal" FORCE)
            message(WARNING "GTK dialog backend selected; may be affected by GNOME GSettings schema mismatch")
        else()
            message(FATAL_ERROR
                "Invalid UNLEASHED_RECOMP_LINUX_NFD_BACKEND='\${UNLEASHED_RECOMP_LINUX_NFD_BACKEND}'. Use PORTAL or GTK.")
        endif()
    endif()
    add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/nativefiledialog-extended")
    add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/SDL")
    add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/SDL_mixer")
endif()

add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/plume")

if (APPLE)
    add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/MoltenVK")
endif()
