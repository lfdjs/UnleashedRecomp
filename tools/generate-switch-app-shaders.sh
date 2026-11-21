#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
root_dir="$(cd -- "${script_dir}/.." && pwd)"
shader_dir="${root_dir}/UnleashedRecomp/gpu/shader"
dxc_root="${root_dir}/tools/XenosRecomp/thirdparty/dxc-bin"

# EN: Allow developers to override DXC when cross-building from Linux or macOS.
# PT-BR: Permite informar um DXC externo em compilacoes cruzadas Linux/macOS.
if [[ -n "${UNLEASHED_DXC:-}" ]]; then
  dxc="${UNLEASHED_DXC}"
elif command -v dxc >/dev/null 2>&1; then
  dxc="$(command -v dxc)"
elif [[ "$(uname -s)" == "Darwin" ]]; then
  case "$(uname -m)" in
    arm64|aarch64) dxc_arch="arm64" ;;
    x86_64|amd64) dxc_arch="x64" ;;
    *) echo "Unsupported macOS host architecture: $(uname -m)" >&2; exit 1 ;;
  esac
  dxc="${dxc_root}/bin/${dxc_arch}/dxc-macos"
  lib_dir="${dxc_root}/lib/${dxc_arch}"
  export DYLD_LIBRARY_PATH="${lib_dir}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"
else
  echo "DXC is missing. Install a host-compatible dxc or set UNLEASHED_DXC." >&2
  echo "DXC nao encontrado. Instale o dxc compativel ou defina UNLEASHED_DXC." >&2
  exit 1
fi

# EN: Use an executable DXC to avoid misleading shader generation failures.
# PT-BR: Exige DXC executavel para evitar erros ambiguos na geracao.
if [[ ! -x "${dxc}" ]]; then
  echo "DXC is not executable: ${dxc}" >&2
  exit 1
fi

# EN: Compile one HLSL entry into a SPIR-V C header using the selected native DXC.
# PT-BR: Compila um shader HLSL em cabecalho C com SPIR-V usando o DXC nativo.
compile_shader() {
  local name="$1"
  local target="$2"
  shift 2

  "${dxc}" \
    -T "${target}" \
    -HV 2021 \
    -all-resources-bound \
    -spirv \
    -fvk-use-dx-layout \
    "$@" \
    -Fh "${shader_dir}/${name}.hlsl.spirv.h" \
    "${shader_dir}/${name}.hlsl" \
    -Vn "g_${name}_spirv"
}

# EN: Compile a vertex shader with Vulkan Y-axis correction enabled.
# PT-BR: Compila o vertex shader com ajuste do eixo Y para Vulkan.
compile_vertex_shader() {
  compile_shader "$1" vs_6_0 -fvk-invert-y -DUNLEASHED_RECOMP
}

# EN: Compile a fragment/pixel shader for the portable Vulkan pipeline.
# PT-BR: Compila o pixel shader para o pipeline Vulkan portavel.
compile_pixel_shader() {
  compile_shader "$1" ps_6_0 -DUNLEASHED_RECOMP
}

compile_pixel_shader blend_color_alpha_ps
compile_vertex_shader copy_vs
compile_pixel_shader copy_color_ps
compile_pixel_shader copy_depth_ps
compile_pixel_shader csd_filter_ps
compile_vertex_shader csd_no_tex_vs
compile_vertex_shader csd_vs
compile_pixel_shader enhanced_motion_blur_ps
compile_pixel_shader gaussian_blur_3x3
compile_pixel_shader gaussian_blur_5x5
compile_pixel_shader gaussian_blur_7x7
compile_pixel_shader gaussian_blur_9x9
compile_pixel_shader gamma_correction_ps
compile_pixel_shader imgui_ps
compile_vertex_shader imgui_vs
compile_pixel_shader movie_ps
compile_vertex_shader movie_vs
compile_pixel_shader resolve_msaa_color_2x
compile_pixel_shader resolve_msaa_color_4x
compile_pixel_shader resolve_msaa_color_8x
compile_pixel_shader resolve_msaa_depth_2x
compile_pixel_shader resolve_msaa_depth_4x
compile_pixel_shader resolve_msaa_depth_8x
