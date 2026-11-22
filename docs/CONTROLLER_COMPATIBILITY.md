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
