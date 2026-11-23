#!/usr/bin/env bash
set -euo pipefail

# EN: Test the optional mod helper using fake Flatpak/xdg-open commands.
#     No network, GameBanana files or real user installations are accessed.
# PT-BR: Testes com mocks sem rede e sem instalacoes/arquivos de terceiros.
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
script="$root/tools/controller-loading-mods.sh"
tmp="$(mktemp -d)"
trap 'rm -rf -- "$tmp"' EXIT
mkdir -p "$tmp/bin" "$tmp/game"
printf 'stub' > "$tmp/game/UnleashedRecomp"
printf 'stub' > "$tmp/game/cpkredir.ini"
cat > "$tmp/bin/flatpak" <<'MOCK'
#!/usr/bin/env bash
printf 'flatpak %s\n' "$*" >> "$CONTROLLER_TEST_LOG"
exit 0
MOCK
cat > "$tmp/bin/xdg-open" <<'MOCK'
#!/usr/bin/env bash
printf 'xdg-open %s\n' "$*" >> "$CONTROLLER_TEST_LOG"
exit 0
MOCK
chmod +x "$tmp/bin/flatpak" "$tmp/bin/xdg-open"
export PATH="$tmp/bin:$PATH"
export CONTROLLER_TEST_LOG="$tmp/commands.log"
export PORTABLE_GAME_DIR="$tmp/game"

bash "$script" --status > "$tmp/status.log"
grep -q 'CONTROLLER_LOADING|executable|FOUND' "$tmp/status.log"
grep -q 'CONTROLLER_LOADING|mod_config|FOUND' "$tmp/status.log"
grep -q 'CONTROLLER_LOADING|hedgemodmanager|INSTALLED' "$tmp/status.log"

bash "$script" --open-mod dualsense > "$tmp/dualsense.log"
bash "$script" --open-mod xbox-series > "$tmp/xbox.log"
grep -q 'xdg-open https://gamebanana.com/mods/580124' "$tmp/commands.log"
grep -q 'xdg-open https://gamebanana.com/mods/582273' "$tmp/commands.log"

bash "$script" --install-manager > "$tmp/install.log"
bash "$script" --launch-manager > "$tmp/launch.log"
grep -q 'flatpak install --user flathub io.github.hedge_dev.hedgemodmanager' "$tmp/commands.log"
grep -q 'flatpak run io.github.hedge_dev.hedgemodmanager' "$tmp/commands.log"

if bash "$script" --open-mod unknown > /dev/null 2>&1; then
  echo 'FAIL: unknown mod accepted' >&2
  exit 1
fi
if bash "$script" --install-manager unexpected > /dev/null 2>&1; then
  echo 'FAIL: unexpected parameters accepted' >&2
  exit 1
fi
bash "$script" --help > "$tmp/help.log"
grep -q 'archives cannot be added as Git submodules' "$tmp/help.log"

echo 'CONTROLLER_LOADING_HELPER|PASS|no_game_data=1|no_network=1'
