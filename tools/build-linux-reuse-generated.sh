#!/usr/bin/env bash
set -euo pipefail

# EN: Rebuild host application/translation changes from previously generated
#     guest PPC sources and shader cache. These outputs must already exist
#     locally from the user's own licensed Xbox 360 copy.
# PT-BR: Recompila mudancas da interface a partir dos fontes PPC e shaders
#     gerados anteriormente pela copia licenciada do proprio usuario.
root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
jobs="${PORTABLE_BUILD_JOBS:-4}"

# EN: This helper deliberately does not extract files, copy licensed games,
#     or invent missing generated guest code.
# PT-BR: Nao extrai jogos, copia arquivos licenciados nem inventa codigo guest.
if [[ "$(uname -s)" != "Linux" ]]; then
  echo "BLOCKED: only the linux-debug preset is supported here / apenas Linux." >&2
  exit 2
fi

echo "INFO: rebuilding Linux from previously generated guest sources."
echo "INFO: recompilando Linux com fontes guest pre-gerados."

# EN: The CMake option validates all PPC files and shader cache; if any are
#     missing, configuration stops with a precise explanation.
# PT-BR: O CMake verifica todos os arquivos PPC e shader cache; caso faltem
#     fontes, interrompe com uma mensagem de diagnostico.
cmake --preset linux-debug -S "$root_dir" \
  -DUNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES=ON

cmake --build "${root_dir}/out/build/linux-debug" \
  --target UnleashedRecomp --parallel "$jobs"

echo "PASS: executable rebuilt / executavel recompilado."
