#!/usr/bin/env bash
set -euo pipefail

# EN: Validate guest-artifact preflight with harmless tiny CMake fixtures.
#     This is NOT a full game build and never creates actual Xbox 360 data.
# PT-BR: Testa a verificacao com arquivos CMake temporarios e inocuos.
#     NAO compila o jogo nem gera dados do Xbox 360.
root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
scratch="$(mktemp -d)"
trap 'rm -rf -- "$scratch"' EXIT

cat > "$scratch/verify.cmake" <<'CMAKE'
include("${VERIFY_MODULE}")
unleashed_recomp_verify_pregenerated_guest_sources("${PPC_FILE}" "${SHADER_FILE}")
CMAKE

# EN: A complete non-empty source set must pass with no game data.
# PT-BR: Um conjunto completo e nao vazio deve passar sem arquivos do jogo.
printf '// EN/PT-BR: fixture only\n' > "$scratch/ppc.cpp"
printf '// EN/PT-BR: fixture only\n' > "$scratch/shader.cpp"
cmake -DVERIFY_MODULE="$root_dir/cmake/VerifyPregeneratedGuestSources.cmake" \
  -DPPC_FILE="$scratch/ppc.cpp" -DSHADER_FILE="$scratch/shader.cpp" \
  -P "$scratch/verify.cmake"

# EN: Missing files must fail, rather than silently linking stale content.
# PT-BR: Arquivos ausentes devem falhar em vez de mascarar o problema.
if cmake -DVERIFY_MODULE="$root_dir/cmake/VerifyPregeneratedGuestSources.cmake" \
  -DPPC_FILE="$scratch/ppc.cpp" -DSHADER_FILE="$scratch/missing.cpp" \
  -P "$scratch/verify.cmake" > "$scratch/missing.log" 2>&1; then
  echo "FAIL: missing source unexpectedly accepted" >&2
  exit 1
fi
grep -q "Missing 1 pre-generated guest source" "$scratch/missing.log"

# EN: Empty source files are also rejected before compiling the game.
# PT-BR: Fontes vazios tambem sao rejeitados antes da compilacao.
: > "$scratch/shader.cpp"
if cmake -DVERIFY_MODULE="$root_dir/cmake/VerifyPregeneratedGuestSources.cmake" \
  -DPPC_FILE="$scratch/ppc.cpp" -DSHADER_FILE="$scratch/shader.cpp" \
  -P "$scratch/verify.cmake" > "$scratch/empty.log" 2>&1; then
  echo "FAIL: empty source unexpectedly accepted" >&2
  exit 1
fi
grep -q "Missing 1 pre-generated guest source" "$scratch/empty.log"

echo "PASS: pre-generated guest source checks reject missing and empty files."
echo "APROVADO: verificacao rejeita arquivos guest ausentes ou vazios."
