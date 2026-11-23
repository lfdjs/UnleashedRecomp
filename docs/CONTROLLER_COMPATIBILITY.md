# Suporte a controles / Gamepad compatibility

## Dispositivos e sistemas / Devices and platforms

**PT-BR:** A branch `portable` usa dois drivers distintos:
SDL2 (Linux e Windows) e libnx HID/Npad (Nintendo Switch nativo).
Os modelos são reconhecidos por tipo/estilo e convertidos para os
comandos do Xbox 360 usados pelo Sonic Unleashed original.

**EN:** Desktop uses SDL2 GameController/HIDAPI, native Nintendo Switch
uses libnx Npad. Detection is based on a device handle (desktop) or
Npad style (Switch), preserving the original Xbox 360 guest input ABI.

| Controle | Linux / Windows (SDL2) | Switch nativo (libnx) |
|---|---|---|
| Xbox 360 / Xbox One | Identificado | Fora do escopo deste driver |
| DualShock 4 | Identificado por USB/Bluetooth se o SDL o reconhecer | Sem suporte nativo específico |
| DualSense (PS5) | Identificado por USB/Bluetooth se o SDL o reconhecer | Sem suporte nativo específico |
| Nintendo Switch Pro | Identificado por HIDAPI | Npad FullKey |
| Dois Joy-Con pareados | SDL combina em um controle lógico | Npad JoyDual |
| Joy-Con no modo portátil | Não se aplica ao PC | Npad Handheld |
| Um Joy-Con isolado | Identificado; comandos incompletos | Identificado; comandos incompletos |

**PT-BR:** DualShock 4 e DualSense **não** passam a funcionar diretamente
no Switch com este código: controles Sony no Switch precisam de um driver,
adaptador ou camada de compatibilidade que os exponha ao HID do sistema.
Da mesma forma, a lista acima descreve **suporte implementado**,
não testes com hardware físico já realizados.

**EN:** Native Switch does not inherently pair PS4/PS5 controllers;
that requires an external compatibility layer or supported adapter.
Controller behavior, USB/Bluetooth pairing, rumble and latency must be
tested on actual devices. A connected device may be unrecognized if
Linux udev/HID permissions or SDL mappings are missing.

## Funcionalidades / Features

- **Detecção inicial:** SDL enumera controles existentes no começo.
- **Hotplug:** conexão/desconexão limpa o controle antigo; outro controle
  conectado passa a ser ativo sem deixar botões pressionados.
- **Troca de conexão:** classificação consulta o handle SDL aberto,
  não índices que podem mudar quando um controle é removido.
- **Joy-Con no PC:** dicas SDL HIDAPI e COMBINE_JOY_CONS ativadas antes
  da inicialização, para tentar combinar os dois dispositivos.
- **Switch portátil:** libnx informa Npad Handheld, separadamente de
  FullKey (Pro) e JoyDual (dois Joy-Con).
- **Mapeamento:** botões, direcionais, sticks e gatilhos seguem o padrão
  de posições usado na interface GameController do SDL.
- **Vibração/LEDs:** tentados pelos drivers existentes; dependem do suporte
  da plataforma e não há garantia de recursos exclusivos do DualSense.

**PT-BR:** Os ícones apresentados na interface continuam nos estilos
**PlayStation** (DualShock/DualSense) e **Xbox** (Xbox/Nintendo fallback).
A identificação Nintendo está correta nos logs, mas **não** foram
adicionadas texturas de botões A/B/X/Y específicas da Nintendo.

## Validar no Linux / Test on Linux

Na raiz do repositório:

~~~bash
git switch portable
git pull --ff-only

# Checa reconhecimento, modelo e quantidade de controles
bash tools/validate-linux-runtime.sh

# Se houver PPC/shaders previamente gerados, recompila sem default.xex
bash tools/build-linux-reuse-generated.sh

# Inicia o jogo para testar comandos reais
bash tools/validate-linux-runtime.sh --game
~~~

O log de diagnóstico agora pode incluir linhas como:

~~~text
PORTABLE_INFO|controllers|connected=1
PORTABLE_INFO|controller_0|name=PS5 Controller|mapped=1|model=DualSense|type=7
~~~

**Atenção:** acima há somente um exemplo de formato de saída, não
uma detecção realizada nesta máquina. Verifique os valores reais.

Conecte primeiro um DualShock 4 por USB; depois teste Bluetooth e repita
com o DualSense e o Switch Pro. Para dois Joy-Con, pareie ambos no sistema
operacional e confira se o SDL apresenta um controle combinado.

Se constar `mapped=0`, o sistema reconheceu um joystick, mas o SDL ainda
não fornece mapeamento de GameController; verifique o acesso HIDAPI,
permissões e eventuais arquivos `SDL_GAMECONTROLLERCONFIG`.

## Testes automáticos / Asset-free tests

Os testes verificam sem hardware: classificação dos modelos SDL,
mapeamento das famílias de ícones e estilos do Npad, inclusive portátil.
Os jobs Linux/Windows e SDL diagnóstico não são uma compilação do jogo
licenciado nem testes de gameplay no Switch.

**EN:** The CI SDL model tests and Nintendo style unit tests require
neither game data nor real controllers. Full-game rendering and input
behavior remain separate manual acceptance tests.

## Ícones adaptativos: DualSense e Xbox Series / Adaptive button prompts

**PT-BR:** O `ButtonGuide` agora possui uma terceira camada visual
procedural (ImGui), aplicada **somente quando a opção Ícones do controle
está em Automático**. O novo desenho não modifica as texturas licenciadas
presentes no submódulo `UnleashedRecompResources`.

| Dispositivo detectado | Estilo automático dos botões A/B/X/Y |
|---|---|
| DualSense / PlayStation 5 | Cruz, círculo, quadrado e triângulo **brancos**, com fundo escuro |
| DualShock 4 / DualShock 3 | Símbolos clássicos do atlas PlayStation |
| Xbox One / Xbox Series | Letras A/B/X/Y **verde/vermelha/azul/amarela**, em fundo escuro |
| Xbox 360 | Atlas Xbox original |
| Nintendo Switch Pro / Joy-Con / portátil | Atlas Xbox original até haver ícones Nintendo |
| Desconhecido | Atlas Xbox original |

**EN:** The runtime draws modern face-button symbols directly with ImGui.
Xbox Series controllers generally report as the SDL2 `XBOXONE` family;
the application cannot distinguish older Xbox One pads reliably from Series
controllers using that SDL type alone. Both use the new Series-like art.
DualSense uses white cross/circle/square/triangle. These are original
vector approximations rather than official vendor artwork.

**PT-BR:** A configuração **Xbox** ou **PlayStation** escolhida manualmente
continua usando o atlas original, sem novos desenhos. Em modo **Automático**,
Xbox One/Series agora também desenha **LB/RB** como botões alongados, com
borda discreta e letras brancas **em negrito**. As letras A/B/X/Y ficaram
maiores e mais espessas, com a letra X azul destacada da cruz PlayStation.
L1/R1 no DualSense, L2/R2, gatilhos, Menu/Options/Create, teclado e
mensagens de botões renderizadas **dentro do próprio jogo Xbox 360**
não foram alterados.
Essas mudanças abrangem os guias do instalador, opções e overlays que
utilizam `ButtonGuide`, não todos os recursos do jogo.

**EN:** The port-owned ButtonGuide now draws bold, oversized colored
Xbox One/Series ABXY letters and new dark, rounded white-lettered LB/RB
bumper caps. DualSense keeps its existing white geometric face symbols
and its original L1/R1 atlas. Trigger/menu art, in-engine tutorial artwork
and manually forced icon sets are unchanged. The procedural symbols must
be checked for alignment and scaling on real hardware; asset-free CI
tests only verify style selection.

~~~bash
git switch portable
git pull --ff-only
bash tools/build-linux-reuse-generated.sh
bash tools/validate-linux-runtime.sh
bash tools/validate-linux-runtime.sh --game
~~~

**PT-BR:** Conecte o DualSense e confira `icons=dualsense-white` no log de
diagnóstico, se o SDL o detectar corretamente. Abra o instalador ou as opções
e veja se os botões ✕/○/□/△ aparecem com traços brancos. Ao conectar Xbox
Series/One, o diagnóstico deve indicar `icons=xbox-series-color` e o guia
deve apresentar letras coloridas. Para comparar com a arte antiga,
selecione manualmente PlayStation ou Xbox nas opções.

**EN:** Test USB and Bluetooth separately. Confirm hotplug, prompts and
styling on the installer/options screens. Full-game binary compilation
is not performed by the asset-free CI workflow.


## Desenhos completos de controle no loading / Full controller art on loading screens

**PT-BR:** As seis cenas originais de controle (Sonic diurno, Werehog,
Super Sonic etc.) utilizam os identificadores \`360_sonic1\`,
\`360_evil\` e outros equivalentes \`ps3_*\`. O hook existente
\`LoadingScreenControllerMidAsmHook\` substitui somente os três
primeiros caracteres, preservando o restante do identificador.

O novo perfil \`ui/loading_controller_profile.h\` distingue
**DualSense**, **Xbox One/Series**, **PlayStation clássico** e
**Xbox clássico**. A seleção Automático escolhe \`ps3_*\` para DualSense
e \`360_*\` para Xbox One/Series. Escolhas manuais Xbox/PlayStation
continuam funcionando. O log informa a família escolhida.

**IMPORTANTE:** Isso **não** desenha um controle moderno por si só.
O jogo original **não contém** cenas \`ps5_*\` nem \`xbs_*\`.
Tentar usá-las diretamente pode quebrar o carregamento. Para ver
o desenho físico moderno, instale um **mod de recursos de loading**
que substitua o slot \`ps3_*\` por DualSense e o slot \`360_*\`
por Xbox One/Series no mesmo conjunto de recursos.

- [Unleashed PS And Xbox Buttons](https://gamebanana.com/mods/580124)
  oferece estilos PS5 e Xbox e recursos de loading. Configure as
  variantes PS5/Xbox separadamente, caso essa opção esteja disponível.
- [Emmy's Xbox Series X Buttons](https://gamebanana.com/mods/582273)
  traz modelos de controle Xbox Series nas telas de loading.

Não faça upload, cópia ou incorporação dessas artes ao repositório:
respeite as licenças, opções e ordem de carregamento dos mods.
No Linux, a instalação do gerenciador e a escolha dos perfis do
mod podem exigir configuração manual. **Não misture dois mods que
substituem o mesmo \`ui_loading.yncp\` sem antes verificar compatibilidade.**

**EN:** The original loading screen contains exactly two scene families.
Modern hardware profiles select the corresponding existing \`ps3\` or
\`360\` slot; user-installed art mods supply different controller artwork.
This avoids invalid guest scene names and changes no game binaries,
textures, archives or third-party artwork. CI covers model-to-slot
selection, not user mods or actual loading-screen rendering.

Use \`bash tools/validate-linux-runtime.sh --game\` and inspect logs:
\`Loading screen controller: DualSense (ps3 scene slot)\` or
\`Loading screen controller: Xbox One/Series (360 scene slot)\`.
If the artwork remains PS3/Xbox 360, the loading asset mod is missing,
disabled or not set to the desired controller variant.


## Instalação opcional dos desenhos de loading / Optional loading artwork

**PT-BR:** Os mods de arte do GameBanana não são repositórios Git e não
devem ser copiados como submódulos sem autorização dos autores. A branch
`portable` oferece um assistente de integração seguro (sem baixar artes)
e orientação para o **Hedge Mod Manager** no Linux:

~~~bash
bash tools/controller-loading-mods.sh --status
bash tools/controller-loading-mods.sh --install-manager
bash tools/controller-loading-mods.sh --open-mod dualsense
bash tools/controller-loading-mods.sh --open-mod xbox-series
bash tools/controller-loading-mods.sh --launch-manager
~~~

**EN:** This helper only checks local setup, optionally installs/launches
the official HMM Flatpak and opens the authors' download pages. Users
install their chosen archives through HMM. No third-party assets are
vendored, mirrored or added as Git submodules.

Detalhes, permissões Flatpak e possíveis conflitos entre os slots
`ps3_*`/`360_*`: [Guia de mods do loading](CONTROLLER_LOADING_MODS.md).
