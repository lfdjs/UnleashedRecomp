#!/usr/bin/env bash
set -euo pipefail
# EN: Run reproducible portable tests without downloading copyrighted game data.
# PT-BR: Executa testes reprodutiveis sem baixar dados proprietarios do jogo.
root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
jobs="${PORTABLE_VALIDATE_JOBS:-2}"
sanitizers=false
host_tools=false
full_game=false

# EN: Explain the supported modes and prerequisites.
# PT-BR: Explica os modos de execucao e seus requisitos.
help() {
  cat <<'USAGE'
Usage / Uso: tools/validate-portable-runtime.sh [--sanitizers] [--host-tools] [--full]
  Default: Debug + Release tests independent of game files.
  Padrao: testes Debug + Release independentes dos arquivos do jogo.
  --sanitizers  ASan + UBSan (GCC/Clang).
  --host-tools  Build XenonRecomp/XenosRecomp (initialized submodules needed).
  --full        Build full Linux game (requires licensed game assets and VCPKG_ROOT).
  --help        Display this usage text / Exibe esta ajuda.
USAGE
}
for option in "$@"; do
  case "$option" in
    --sanitizers) sanitizers=true ;;
    --host-tools) host_tools=true ;;
    --full) full_game=true ;;
    --help|-h) help; exit 0 ;;
    *) echo "Invalid option / Opcao invalida: $option" >&2; help; exit 2 ;;
  esac
done

# EN: Compile and execute asset-free CTest checks in one build configuration.
# PT-BR: Compila e executa testes CTest sem arquivos do jogo.
test_units() {
  local mode="$1"
  local dir="${root_dir}/build/portable-runtime-${mode,,}"
  cmake -S "${root_dir}/UnleashedRecomp/portable/tests" -B "$dir" "-DCMAKE_BUILD_TYPE=$mode"
  cmake --build "$dir" --config "$mode" --parallel "$jobs"
  ctest --test-dir "$dir" -C "$mode" --output-on-failure
}
test_units Debug
test_units Release

if [[ "$sanitizers" == true ]]; then
  # EN: Extra checks for invalid memory access and undefined C++ behavior.
  # PT-BR: Verificacoes extras de memoria invalida e comportamento indefinido.
  san_dir="${root_dir}/build/portable-runtime-sanitizers"
  cmake -S "${root_dir}/UnleashedRecomp/portable/tests" -B "$san_dir" -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_ENABLE_SANITIZERS=ON
  cmake --build "$san_dir" --parallel "$jobs"
  ctest --test-dir "$san_dir" --output-on-failure
fi

if [[ "$host_tools" == true ]]; then
  # EN: Generators need their checked-out nested third-party submodules.
  # PT-BR: Geradores dependem dos submodulos externos inicializados.
  if [[ ! -f "${root_dir}/tools/XenosRecomp/CMakeLists.txt" || ! -f "${root_dir}/tools/XenonRecomp/CMakeLists.txt" ]]; then
    echo "BLOCKED: run git submodule update --init --recursive tools/XenonRecomp tools/XenosRecomp" >&2
    exit 2
  fi
  # EN: XenonRecomp uses MS-style anonymous aggregates; Clang compiles them
  #     without patching its upstream submodule.
  # PT-BR: Clang aceita os agregados anonimos do XenonRecomp sem alterar o upstream.
  if ! command -v clang++ >/dev/null 2>&1; then
    echo "BLOCKED: install Clang for the host generators / instale Clang." >&2
    exit 2
  fi
  cmake -S "$root_dir" -B "${root_dir}/build/portable-host-tools" -DUNLEASHED_RECOMP_HOST_TOOLS_ONLY=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
  cmake --build "${root_dir}/build/portable-host-tools" --parallel "$jobs" --target file_to_c x_decompress XenonRecomp XenosRecomp
fi

if [[ "$full_game" == true ]]; then
  # EN: Block misleading builds when licensed Xbox 360 inputs are absent.
  # PT-BR: Impede builds enganosos sem arquivos licenciados do Xbox 360.
  private="${root_dir}/UnleashedRecompLib/private"
  for name in default.xex default.xexp shader.ar; do
    if [[ ! -f "$private/$name" ]]; then
      echo "BLOCKED: missing licensed game file / arquivo ausente: $private/$name" >&2
      exit 2
    fi
  done
  if [[ -z "${VCPKG_ROOT:-}" || "$(uname -s)" != Linux ]]; then
    echo "BLOCKED: --full requires Linux and a configured VCPKG_ROOT." >&2
    exit 2
  fi
  cmake --preset linux-debug -S "$root_dir"
  cmake --build "${root_dir}/out/build/linux-debug" --target UnleashedRecomp --parallel "$jobs"
fi
echo "PASS / APROVADO: all selected validations finished."
