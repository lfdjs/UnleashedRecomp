# Mods de controles nas telas de loading / Controller-loading artwork mods

## Por que não são submódulos Git?

**PT-BR:** Um submódulo Git aponta para um commit de outro repositório Git.
Os dois pacotes mencionados estão disponíveis como downloads do GameBanana,
não como repositórios Git públicos identificados. Além disso, a autora de
**Emmy's XBOX Series X Buttons** exige permissão para reutilizar suas artes.
Não adicionamos, hospedamos ou distribuímos esses arquivos pelo GitHub.

**EN:** A Git submodule references an upstream Git repository, not a
downloadable ZIP/7z mod. Do not create mirrors of third-party assets
without explicit permission from their authors.

Fontes originais / Original author pages:
- [Unleashed PS And Xbox Buttons](https://gamebanana.com/mods/580124)
- [Emmy's XBOX Series X Buttons](https://gamebanana.com/mods/582273)
- [Documentação do git submodule](https://git-scm.com/docs/git-submodule)

## Fluxo recomendado: Hedge Mod Manager no Linux

**PT-BR:** A branch \`portable\` já tem carregador de mods compatível com
o **Hedge Mod Manager (HMM)**. O gerenciador possui distribuição para Linux
via Flatpak. Nosso script facilita o processo, sem baixar ou extrair
arquivos dos mods automaticamente.

~~~bash
git switch portable
git pull --ff-only

# 1. Confere instalação e gerenciador (não modifica nada).
bash tools/controller-loading-mods.sh --status

# 2. Instala o gerenciador oficial (ação explícita e opcional).
bash tools/controller-loading-mods.sh --install-manager

# 3. Abre as páginas de download dos criadores no navegador.
bash tools/controller-loading-mods.sh --open-mod dualsense
bash tools/controller-loading-mods.sh --open-mod xbox-series

# 4. Executa o gerenciador para importar os arquivos baixados.
bash tools/controller-loading-mods.sh --launch-manager
~~~

No HMM, selecione **Unleashed Recompiled**, indique a pasta do jogo caso
sua versão compilada localmente não seja detectada e importe os arquivos
obtidos nas páginas originais. Escolha os estilos nos menus próprios de
configuração do mod, quando disponíveis. Para desfazer, desative o mod
pelo gerenciador.

**EN:** The helper only checks local setup, opens author pages and
installs/launches HMM at the user's explicit request. It never downloads,
redistributes, extracts or activates third-party mod archives. HMM manages
the imported mods and the game configuration.

## Permissões Flatpak para disco externo / External-drive access

Sua instalação Linux está em uma pasta dentro de \`/run/media/\`. Se o HMM
não conseguir enxergar esse caminho, talvez seja necessário autorizar
o acesso à pasta da instalação, com o gerenciador fechado. Execute na
raiz do repositório:

~~~bash
flatpak override --user \
  --filesystem="$PWD/out/build/linux-debug/UnleashedRecomp" \
  io.github.hedge_dev.hedgemodmanager
~~~

**PT-BR:** Conceda somente a permissão necessária. O script não muda
permissões Flatpak sozinho. Se \`flatpak\` não existir, instale-o pela
distribuição e adicione o repositório Flathub.

## Seleção automática e limitações técnicas

| Controle | Cena original selecionada | Aparência moderna |
|---|---|---|
| DualSense | \`ps3_*\` | Depende do recurso PS5 instalado |
| DualShock 4 | \`ps3_*\` | Depende da variante PlayStation |
| Xbox One/Series | \`360_*\` | Depende do recurso Xbox Series instalado |
| Xbox 360 | \`360_*\` | Padrão original ou variante selecionada |

**PT-BR:** O código detecta o controle automaticamente, mas o jogo original
só possui cenas \`ps3_*\` e \`360_*\`. Para exibir DualSense e Xbox Series
na **mesma instalação**, o conjunto de recursos instalado precisa preservar
as duas famílias e conter ambas as artes. Não está comprovado que as opções
dos pacotes citados ofereçam essa combinação pronta.

**Cuidado:** dois mods que substituem \`ui_loading.yncp\` podem entrar
em conflito, pois a prioridade de arquivos pode fazer um sobrescrever
o outro. Não habilite os dois simultaneamente sem conferir a estrutura
do pacote. A instalação do gerenciador não garante a troca visual.

**EN:** Automatic device detection selects valid in-game scene slots.
The loading art itself comes from compatible user-installed mod files.
Two mods replacing the same loading resource may conflict. Combining
styles requires a compatible single resource set and permission to edit
its artwork.

## Testes sem dados do jogo / Asset-free checks

~~~bash
bash -n tools/controller-loading-mods.sh
bash tools/tests/test-controller-loading-mods.sh
bash tools/controller-loading-mods.sh --status
~~~

Os testes usam versões simuladas de \`flatpak\` e \`xdg-open\`; não fazem
downloads, não tocam em arquivos de jogos e não testam o funcionamento
de mods gráficos reais no runtime.
