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
