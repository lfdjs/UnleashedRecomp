#!/usr/bin/env bash
set -euo pipefail

# EN: Capture host preflight and optional real-game logs without modifying
#     game installations, disabling Vulkan errors or running the installer.
# PT-BR: Coleta diagnosticos e logs opcionais sem modificar a instalacao,
#     ocultar erros Vulkan ou executar o instalador automaticamente.
root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
game_bin="${PORTABLE_GAME_BIN:-${root_dir}/out/build/linux-debug/UnleashedRecomp/UnleashedRecomp}"
report_dir="${PORTABLE_REPORT_DIR:-${root_dir}/build/portable-runtime-reports}"
mode=preflight

# EN: Accept --game to run actual gameplay after recording the host checks.
# PT-BR: Aceita --game para executar o jogo apos registrar as verificacoes.
if [[ "${1:-}" == "--game" ]]; then
  mode=game
  shift
elif [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
  cat <<'HELP'
Usage / Uso:
  bash tools/validate-linux-runtime.sh
  bash tools/validate-linux-runtime.sh --game [optional game arguments]

Optional / Opcional:
  PORTABLE_GAME_BIN=/path/to/UnleashedRecomp
  PORTABLE_REPORT_DIR=/path/to/reports

The default preflight checks SDL, Vulkan loader, audio and guest memory without
launching the game. --game starts the game interactively and saves its stdout.
O diagnostico padrao nao executa o jogo. --game inicia o jogo e salva o log.
HELP
  exit 0
elif [[ "$#" -ne 0 ]]; then
  echo "Invalid option / Opcao invalida: $1" >&2
  exit 2
fi

# EN: The game binary is produced by the complete Linux CMake target.
# PT-BR: O executavel e gerado pelo target integral Linux do CMake.
if [[ ! -f "$game_bin" || ! -x "$game_bin" ]]; then
  echo "BLOCKED: executable missing / executavel ausente: $game_bin" >&2
  echo "Run / Execute: cmake --build out/build/linux-debug --target UnleashedRecomp --parallel 4" >&2
  exit 2
fi
if [[ "$(uname -s)" != "Linux" ]]; then
  echo "BLOCKED: this script currently validates the Linux host only." >&2
  exit 2
fi

# EN: Never overwrite an earlier diagnostic session; use timestamp and PID.
# PT-BR: Cada execucao usa data e PID, preservando relatorios anteriores.
mkdir -p "$report_dir"
stamp="$(date -u +%Y%m%dT%H%M%SZ)-$$"
prefix="${report_dir}/runtime-${stamp}"
system_report="${prefix}-system.txt"
preflight_log="${prefix}-preflight.log"
result_file="${prefix}-result.txt"
game_log="${prefix}-game.log"

# EN: Report software environment and dynamic library dependencies.
# PT-BR: Registra informacoes do sistema e bibliotecas dinamicas requeridas.
{
  printf "Date UTC: %s\n" "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  printf "Binary: %s\n" "$game_bin"
  printf "Branch: %s\n" "$(git -C "$root_dir" branch --show-current 2>/dev/null || echo unknown)"
  printf "Commit: %s\n" "$(git -C "$root_dir" rev-parse --short HEAD 2>/dev/null || echo unknown)"
  uname -a
  printf "\n--- Dynamic library dependencies / Bibliotecas dinamicas ---\n"
  if command -v ldd >/dev/null 2>&1; then
    ldd "$game_bin" || true
  else
    echo "ldd not available / nao disponivel"
  fi
  printf "\n--- Vulkan summary / Resumo Vulkan ---\n"
  if command -v vulkaninfo >/dev/null 2>&1; then
    if command -v timeout >/dev/null 2>&1; then
      timeout 20s vulkaninfo --summary || echo "vulkaninfo failed or timed out / falhou ou excedeu o limite"
    else
      vulkaninfo --summary || echo "vulkaninfo failed / falhou"
    fi
  else
    echo "vulkaninfo not installed; preflight will still check the loader"
  fi
} >"$system_report" 2>&1

# EN: Preserve preflight's exact status code even when an SDL subsystem fails.
# PT-BR: Preserva o codigo de retorno do diagnostico mesmo se o SDL falhar.
set +e
"$game_bin" --portable-preflight >"$preflight_log" 2>&1
preflight_status=$?
set -e
printf "preflight_exit=%d\n" "$preflight_status" >"$result_file"
printf "INFO: host report / relatorio do host: %s\n" "$system_report"
printf "INFO: preflight log / log de diagnostico: %s\n" "$preflight_log"
printf "INFO: preflight status / resultado: %d\n" "$preflight_status"
grep -E "^(PORTABLE_(CHECK|INFO|PREFLIGHT_BEGIN|PREFLIGHT_END))" "$preflight_log" || cat "$preflight_log"

if [[ "$mode" == preflight ]]; then
  exit "$preflight_status"
fi

# EN: The user must explicitly choose --game; launch normally and allow
#     interactive closing. Do not impose a timeout on real gameplay.
# PT-BR: O jogo so inicia mediante --game e continua interativo, sem timeout.
echo "INFO: launching game / iniciando jogo; close the game to end capture."
set +e
"$game_bin" "$@" 2>&1 | tee "$game_log"
game_status="${PIPESTATUS[0]}"
set -e
printf "game_exit=%d\n" "$game_status" >>"$result_file"
printf "INFO: game log / log de jogo: %s\n" "$game_log"
printf "INFO: game exit / saida: %s\n" "$game_status"
exit "$game_status"
