cmake_minimum_required(VERSION 3.20)
project(PortableRuntimeTests LANGUAGES CXX)
# EN: Test shared platform contracts without game assets, SDL, or submodules.
# PT-BR: Testa contratos multiplataforma sem dados do jogo, SDL ou submodulos.
add_executable(portable_tests portable_tests.cpp ../memory.cpp ../filesystem.cpp ../process.cpp)
target_compile_features(portable_tests PRIVATE cxx_std_20)
if(WIN32)
    target_link_libraries(portable_tests PRIVATE shell32 ole32)
endif()
enable_testing()
add_test(NAME portable_tests COMMAND portable_tests)

# EN: Run ASan/UBSan against portable code without requiring game assets.
# PT-BR: Executa ASan/UBSan na camada comum sem arquivos do jogo.
option(PORTABLE_ENABLE_SANITIZERS "Enable ASan and UBSan." OFF)
if(PORTABLE_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(portable_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(portable_tests PRIVATE -fsanitize=address,undefined)
    else()
        message(FATAL_ERROR "Sanitizers require Clang or GCC.")
    endif()
endif()
