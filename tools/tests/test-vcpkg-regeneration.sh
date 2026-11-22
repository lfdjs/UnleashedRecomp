#!/usr/bin/env bash
set -euo pipefail

# EN: Reproduce the Ninja regeneration error in a disposable, asset-free
#     CMake project. Both configuration and regeneration run without
#     VCPKG_ROOT; the resolver must restore it each time.
# PT-BR: Reproduz a falha do Ninja em projeto CMake descartavel e sem
#     arquivos do jogo. Configuracao e regeneracao ocorrem sem VCPKG_ROOT.
root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
sandbox="$(mktemp -d)"
trap 'rm -rf -- "$sandbox"' EXIT

# EN: A mock vcpkg.cmake is enough to test root resolution, without
#     downloading or bootstrapping any third-party dependencies.
# PT-BR: Um vcpkg.cmake simulado basta para testar os caminhos, sem
#     baixar dependencias ou executar o bootstrap.
mkdir -p "$sandbox/source/thirdparty/vcpkg/scripts/buildsystems"
mkdir -p "$sandbox/external-vcpkg/scripts/buildsystems"
touch "$sandbox/source/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake"
touch "$sandbox/external-vcpkg/scripts/buildsystems/vcpkg.cmake"

cat > "$sandbox/source/CMakeLists.txt" <<'CMAKE'
cmake_minimum_required(VERSION 3.20)
project(VcpkgRootRegeneration LANGUAGES NONE)
include("${RESOLVER_MODULE}")
file(WRITE "${CMAKE_BINARY_DIR}/resolved-vcpkg-root.txt" "$ENV{VCPKG_ROOT}")
add_custom_target(verify ALL
    COMMAND "${CMAKE_COMMAND}" -E echo "VCPKG_ROOT restored")
CMAKE

# EN: Verify that a preset-style bundled toolchain survives two CMake runs.
# PT-BR: Confere o submodulo embutido em duas configuracoes sem ambiente.
env -u VCPKG_ROOT cmake -S "$sandbox/source" -B "$sandbox/bundled-build" -G Ninja \
    -DRESOLVER_MODULE="$root_dir/cmake/ResolveVcpkgRoot.cmake"
grep -Fxq "$sandbox/source/thirdparty/vcpkg" \
    "$sandbox/bundled-build/resolved-vcpkg-root.txt"

# EN: Force Ninja to regenerate CMake, reproducing the actual failure.
# PT-BR: Forca o Ninja a regenerar, reproduzindo o problema informado.
echo "# EN/PT-BR: force an incremental reconfigure" >> "$sandbox/source/CMakeLists.txt"
env -u VCPKG_ROOT cmake --build "$sandbox/bundled-build"
grep -Fxq "$sandbox/source/thirdparty/vcpkg" \
    "$sandbox/bundled-build/resolved-vcpkg-root.txt"

# EN: A custom cached vcpkg toolchain takes precedence over the submodule.
# PT-BR: Um toolchain externo em cache tem prioridade sobre o submodulo.
env -u VCPKG_ROOT cmake -S "$sandbox/source" -B "$sandbox/external-build" -G Ninja \
    -DRESOLVER_MODULE="$root_dir/cmake/ResolveVcpkgRoot.cmake" \
    -DCMAKE_TOOLCHAIN_FILE="$sandbox/external-vcpkg/scripts/buildsystems/vcpkg.cmake"
grep -Fxq "$sandbox/external-vcpkg" \
    "$sandbox/external-build/resolved-vcpkg-root.txt"

# EN: Respect a caller-supplied environment value, even when a bundled
#     toolchain exists, to avoid overriding user configurations.
# PT-BR: Preserva a variavel fornecida pelo usuario, sem substitui-la.
env VCPKG_ROOT="$sandbox/external-vcpkg" cmake -S "$sandbox/source" \
    -B "$sandbox/explicit-build" -G Ninja \
    -DRESOLVER_MODULE="$root_dir/cmake/ResolveVcpkgRoot.cmake"
grep -Fxq "$sandbox/external-vcpkg" \
    "$sandbox/explicit-build/resolved-vcpkg-root.txt"

echo "PASS: preset-less Ninja regeneration restores VCPKG_ROOT."
echo "APROVADO: o Ninja recupera VCPKG_ROOT sem repetir o preset."
