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
  --full        Build Linux game (requires licensed assets and bundled vcpkg).
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


# EN: Validate the package required by the configured native file dialog.
#     Linux uses xdg-desktop-portal (D-Bus) by default, not in-process GTK.
# PT-BR: Verifica o pacote exigido pelo seletor nativo configurado.
#     No Linux, usa portal (D-Bus) por padrao, sem GTK dentro do jogo.
check_linux_dialog_backend() {
  if ! command -v pkg-config >/dev/null 2>&1; then
    echo "BLOCKED: pkg-config missing / instale pkg-config (pkgconf)." >&2
    return 2
  fi

  # EN: Respect the backend in CMakeCache.txt, including the GTK fallback.
  # PT-BR: Respeita o backend salvo no CMakeCache, inclusive o fallback GTK.
  local backend="PORTAL"
  local cache="${root_dir}/out/build/linux-debug/CMakeCache.txt"
  if [[ -f "$cache" ]]; then
    local cached
    cached="$(sed -n 's/^UNLEASHED_RECOMP_LINUX_NFD_BACKEND:STRING=//p' "$cache" | head -n 1)"
    if [[ -n "$cached" ]]; then
      backend="$cached"
    fi
  fi

  if [[ "$backend" == "PORTAL" ]]; then
    if ! pkg-config --exists dbus-1; then
      echo "BLOCKED: D-Bus development dependency missing (dbus-1)." >&2
      echo "Ubuntu/Debian/Mint: sudo apt install libdbus-1-dev xdg-desktop-portal xdg-desktop-portal-gtk" >&2
      echo "Fedora: sudo dnf install dbus-devel xdg-desktop-portal xdg-desktop-portal-gtk" >&2
      echo "Arch/Manjaro: sudo pacman -S dbus xdg-desktop-portal xdg-desktop-portal-gtk" >&2
      echo "Check / Confira: pkg-config --modversion dbus-1" >&2
      return 2
    fi
    echo "INFO: NFD portal (dbus-1) version: $(pkg-config --modversion dbus-1)"
  elif [[ "$backend" == "GTK" ]]; then
    if ! pkg-config --exists 'gtk+-3.0'; then
      echo "BLOCKED: GTK fallback needs GTK 3 development files." >&2
      echo "Ubuntu/Debian/Mint: sudo apt install libgtk-3-dev" >&2
      echo "Fedora: sudo dnf install gtk3-devel" >&2
      echo "Arch/Manjaro: sudo pacman -S gtk3" >&2
      return 2
    fi
    echo "INFO: NFD GTK fallback version: $(pkg-config --modversion gtk+-3.0)"
  else
    echo "BLOCKED: invalid Linux NFD backend: $backend (use PORTAL or GTK)." >&2
    return 2
  fi
}
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
  # EN: This preset is Linux-only. Reject other hosts before invoking CMake.
  # PT-BR: O preset atende apenas Linux; rejeita outros sistemas antes do CMake.
  if [[ "$(uname -s)" != Linux ]]; then
    echo "BLOCKED: --full with linux-debug requires Linux / exige Linux." >&2
    exit 2
  fi

  # EN: Fail early with actionable package instructions instead of
  #     a long CMake pkg_check_modules stack trace.
  # PT-BR: Falha antecipadamente com instrucoes claras, antes de
  #     aparecer a pilha de erros do pkg_check_modules do CMake.
  check_linux_dialog_backend

  # EN: CMakePresets.json already sets VCPKG_ROOT to thirdparty/vcpkg.
  #     The old shell precheck ran before CMake applied that preset and
  #     incorrectly rejected otherwise correctly configured Linux builds.
  # PT-BR: CMakePresets.json ja define VCPKG_ROOT como thirdparty/vcpkg.
  #     A verificacao anterior ocorria antes de o CMake ler o preset e
  #     impedia indevidamente a compilacao no Linux.
  local_vcpkg_root="${root_dir}/thirdparty/vcpkg"
  if [[ ! -f "${local_vcpkg_root}/scripts/buildsystems/vcpkg.cmake" ]]; then
    echo "BLOCKED: vcpkg submodule missing / submodulo vcpkg ausente." >&2
    echo "Run / Execute: git submodule update --init --recursive thirdparty/vcpkg" >&2
    exit 2
  fi
  if [[ ! -x "${local_vcpkg_root}/vcpkg" ]]; then
    echo "BLOCKED: bundled vcpkg needs bootstrapping / vcpkg precisa ser inicializado." >&2
    echo "Run / Execute: ./thirdparty/vcpkg/bootstrap-vcpkg.sh" >&2
    exit 2
  fi

  # EN: Export the same vcpkg location already used by the Linux preset.
  # PT-BR: Exporta o mesmo diretorio do vcpkg ja usado no preset Linux.
  export VCPKG_ROOT="${local_vcpkg_root}"
  echo "INFO: VCPKG_ROOT=${VCPKG_ROOT} (linux-debug preset)"
  cmake --preset linux-debug -S "$root_dir"
  cmake --build "${root_dir}/out/build/linux-debug" --target UnleashedRecomp --parallel "$jobs"
fi
echo "PASS / APROVADO: all selected validations finished."
