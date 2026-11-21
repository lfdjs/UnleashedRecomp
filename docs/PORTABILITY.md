# Portable runtime / Camada compartilhada de portabilidade

The portable architecture is implemented in `UnleashedRecomp/portable/`.
A platform listed here is **not necessarily a working game port**.

A arquitetura compartilhada reside em `UnleashedRecomp/portable/`.
Um destino listado aqui **nao implica que o jogo ja funciona nessa plataforma**.

| Interface | English | Portugues |
|---|---|---|
| platform.h | Target OS, CPU and compiled features | SO/CPU de destino, recursos compilados |
| graphics.h | API preference; GPU checks remain Plume's job | API preferencial; verificacao real cabe ao Plume |
| memory.h/.cpp | Desktop virtual memory and null page guard | Memoria virtual desktop e protecao da pagina zero |
| filesystem.h/.cpp | User-data directory with original path conventions | Dados de usuario com caminhos originais |
| audio.h | PCM buffer arithmetic, safe refill | Buffer PCM e reposicao segura |
| input.h | Xbox input normalization | Normalizacao de controles Xbox |
| threading.h | Thread yield and sleep | Escalonamento e espera de threads |

## Integrated with the game / Integrado ao jogo

- `kernel/memory.cpp` now calls the shared backend on Windows and POSIX.
- `kernel/memory.cpp` usa o backend compartilhado no Windows e POSIX.
- **Nintendo Switch retains the existing special libnx mapper (4 GiB guest virtual address space and lazy commits).**
- **Nintendo Switch preserva o mapeamento especial libnx (janela virtual de 4 GiB e commits por demanda).**
- `user/paths.cpp` reuses the shared user directory resolver and preserves `portable.txt`.
- `user/paths.cpp` reutiliza os caminhos comuns e preserva `portable.txt`.
- CMake compiles the new shared implementation in the main game target.
- CMake compila a camada compartilhada dentro do executavel principal.

## Standalone tests / Testes independentes

```sh
cmake -S UnleashedRecomp/portable/tests -B build/portable-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/portable-tests --parallel
ctest --test-dir build/portable-tests --output-on-failure
```

The CI workflow runs asset-free tests on Linux, macOS and Windows.
O CI executa testes sem arquivos proprietarios no Linux, macOS e Windows.

## Pending device validation / Validacoes pendentes

- Actual Vulkan capabilities, shader translation and game rendering (Plume).
- Real audio output, controller drivers and game runtime behavior (SDL2/libnx).
- Switch memory mapping on hardware; Linux ARM64 and Android sandbox paths.
- Cross-platform complete game builds requiring lawful proprietary game assets.

Os itens acima precisam de testes reais antes de considerar qualquer port concluido.
