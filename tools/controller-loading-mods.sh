#!/usr/bin/env bash
set -euo pipefail

# EN: Optional loading-screen controller artwork workflow. The original
#     GameBanana mod archives are not Git repositories, and one author
#     explicitly restricts asset reuse. Do not vendor, mirror, download,
#     patch or automatically install third-party copyrighted files.
# PT-BR: Fluxo opcional para artes dos controles no loading. Os mods do
#     GameBanana nao sao repositorios Git; um autor restringe o reuso.
#     Nao inclui, baixa nem instala automaticamente artes de terceiros.

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
GAME_DIR="${PORTABLE_GAME_DIR:-${ROOT}/out/build/linux-debug/UnleashedRecomp}"
APP_ID="io.github.hedge_dev.hedgemodmanager"
DUALSENSE_URL="https://gamebanana.com/mods/580124"
XBOX_URL="https://gamebanana.com/mods/582273"

usage() {
  cat <<'HELP'
Gamepad loading artwork helper / Assistente de artes para a tela de loading

Usage / Uso:
  bash tools/controller-loading-mods.sh --status
  bash tools/controller-loading-mods.sh --install-manager
  bash tools/controller-loading-mods.sh --launch-manager
  bash tools/controller-loading-mods.sh --open-mod dualsense
  bash tools/controller-loading-mods.sh --open-mod xbox-series
  bash tools/controller-loading-mods.sh --help

EN: Mods are downloaded by the user from the original authors, then installed
    and activated through Hedge Mod Manager. This helper never downloads or
    redistributes mod assets or modifies the Sonic Unleashed game files.
PT-BR: Baixe os mods nos sites dos autores e instale-os pelo Hedge Mod
       Manager. O assistente nao baixa nem redistribui arquivos de mods.

Notes / Observacoes:
  - GameBanana archives cannot be added as Git submodules.
  - The original game has only ps3_* and 360_* loading-scene slots.
  - Enabling multiple mods for the same loading resource may conflict.
  - Xbox Series artwork requires a mod, not just controller detection.
HELP
}

require_flatpak() {
  if ! command -v flatpak >/dev/null 2>&1; then
    echo "BLOCKED: Flatpak not found / Flatpak nao instalado." >&2
    echo "Ubuntu: sudo apt install flatpak" >&2
    return 2
  fi
}

status() {
  printf 'CONTROLLER_LOADING|game_dir|%s\n' "$GAME_DIR"
  if [[ -f "$GAME_DIR/UnleashedRecomp" ]]; then
    echo 'CONTROLLER_LOADING|executable|FOUND'
  else
    echo 'CONTROLLER_LOADING|executable|NOT_FOUND'
  fi
  if [[ -f "$GAME_DIR/cpkredir.ini" ]]; then
    echo 'CONTROLLER_LOADING|mod_config|FOUND'
  else
    echo 'CONTROLLER_LOADING|mod_config|NOT_FOUND'
  fi
  if command -v flatpak >/dev/null 2>&1; then
    if flatpak info "$APP_ID" >/dev/null 2>&1; then
      echo 'CONTROLLER_LOADING|hedgemodmanager|INSTALLED'
    else
      echo 'CONTROLLER_LOADING|hedgemodmanager|NOT_INSTALLED'
    fi
  else
    echo 'CONTROLLER_LOADING|flatpak|NOT_INSTALLED'
  fi
  echo 'INFO: this is a non-invasive status check / diagnostico sem alteracoes'
}

open_page() {
  local url="$1"
  echo "SOURCE_URL|$url"
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$url"
  else
    echo 'INFO: xdg-open unavailable; open the URL manually.'
  fi
}

case "${1:---help}" in
  --status)
    [[ "$#" -eq 1 ]] || { usage >&2; exit 2; }
    status
    ;;
  --install-manager)
    [[ "$#" -eq 1 ]] || { usage >&2; exit 2; }
    require_flatpak
    echo 'INFO: installing official Hedge Mod Manager from Flathub / instalando do Flathub'
    flatpak install --user flathub "$APP_ID"
    ;;
  --launch-manager)
    [[ "$#" -eq 1 ]] || { usage >&2; exit 2; }
    require_flatpak
    if ! flatpak info "$APP_ID" >/dev/null 2>&1; then
      echo 'BLOCKED: Hedge Mod Manager missing. Run --install-manager first.' >&2
      exit 2
    fi
    flatpak run "$APP_ID"
    ;;
  --open-mod)
    [[ "$#" -eq 2 ]] || { usage >&2; exit 2; }
    case "$2" in
      dualsense) open_page "$DUALSENSE_URL" ;;
      xbox-series) open_page "$XBOX_URL" ;;
      *) echo 'Invalid mod / Mod invalido: choose dualsense or xbox-series.' >&2; exit 2 ;;
    esac
    ;;
  --help|-h)
    usage
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac
