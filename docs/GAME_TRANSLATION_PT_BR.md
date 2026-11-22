# Tradução dos recursos do jogo / Game asset translation pipeline

## Cobertura e formatos / Scope and file formats

**PT-BR:** A interface do port (instalador, opções e conquistas) já tem
tradução pt-BR. Os textos exibidos pelo jogo Xbox 360 são recursos
independentes, e não ficam automaticamente traduzidos ao alterar locale.cpp.
Conforme a documentação de modding do Hedgehog Engine:

- **XTB (`.xtb`):** tabelas com categorias, identificadores e mensagens;
  SonicColorsXTBConv permite converter entre XTB e XML usando sonic2010/swa.
- **FCO/FTE (`.fco` e `.fte`):** elementos de interface com glifos/texturas.
  Em Sonic Unleashed, FCO geralmente guarda IDs de sprites, não texto puro;
  a ferramenta Converse necessita de tabelas de tradução e fontes adequadas.
- **AR/ARL (`.ar.00` e `.arl`):** arquivos de recursos do jogo; modificar
  o XTB ou FCO normalmente exige extração/empacotamento com ferramentas
  compatíveis. A pipeline abaixo não altera diretamente esses arquivos.
- **Texturas de cenários:** textos pintados na imagem exigem edição gráfica
  e são uma camada separada da tradução dos diálogos.

**EN:** The built-in launcher is localized separately from licensed Xbox 360
game assets. XTB message tables, FCO/FTE image-font text and rendered textures
require distinct workflows. This tool handles local XTB XML editing only.

Referências / References:
- https://hedgedocs.com/wiki/How_to_use_Converse
- https://github.com/TheExentist151/SonicColorsXTBConv
- https://github.com/thesupersonic16/HedgeModManager/wiki/Basic-Mod-File-Structure

## Segurança / Safety

**PT-BR:** Não distribua o jogo, scripts de origem, dados XEX, arquivos
extraídos ou os textos originais incluídos nos modelos privados.
Use somente a sua própria instalação. O script não baixa dados, não altera
o diretório do jogo e não gera arquivos binários `.xtb` ou `.ar`.
Arquivos JSON de tradução e XML exportados podem conter textos originais;
mantenha-os fora do repositório e não faça commit desses arquivos.

**EN:** JSON templates and XML exports can contain copyrighted game text.
Keep them local, out of Git and public repositories. No original game data
is downloaded, redistributed or modified in place. This is an optional
editing tool, not an automatic machine translation service.

## Etapa 1 — Inventariar / Step 1 — inventory

~~~bash
python3 tools/ptbr-game-text.py inventory --directory /caminho/do/jogo
~~~

**PT-BR:** Mostra somente extensões, quantidades e alguns caminhos relativos.
Não lê conteúdo de arquivos. Arquivos `.ar.00` precisam de ferramentas
próprias para listar os recursos que contêm.

## Etapa 2 — Traduzir tabelas XTB / Step 2 — translate exported XTB XML

1. Extraia os seus recursos com ferramentas compatíveis; exporte um `.xtb`
   para XML usando o modo `sonic2010/swa` do SonicColorsXTBConv.
2. Crie um modelo editável local e preencha `pt_br` para cada entrada.
3. Valide os parâmetros e gere um novo XML, preservando o original.
4. Converta o XML traduzido de volta para XTB usando a ferramenta adequada.
5. Empacote como mod e teste dentro do jogo; confirme fontes e quebras de linha.

~~~bash
python3 tools/ptbr-game-text.py export \
  --xml /caminho/extraido/Dialogos.xtb.xml \
  --out /caminho/privado/Dialogos.pt-br.json

# Preencha os campos pt_br no arquivo JSON.
python3 tools/ptbr-game-text.py audit \
  --xml /caminho/extraido/Dialogos.xtb.xml \
  --translations /caminho/privado/Dialogos.pt-br.json

python3 tools/ptbr-game-text.py apply \
  --xml /caminho/extraido/Dialogos.xtb.xml \
  --translations /caminho/privado/Dialogos.pt-br.json \
  --out /caminho/privado/Dialogos.traduzido.xml
~~~

**PT-BR:** O JSON armazena identificador, índice, texto original e
campo `pt_br`. O programa compara o SHA-256 do XML para evitar aplicar
traduções em versões incompatíveis e rejeita mudanças em parâmetros
`printf` ou referências numéricas. Campos não traduzidos preservam
o texto original, e o relatório informa a quantidade pendente.

**EN:** A SHA-256 of the source XML prevents accidental mismatched asset
versions. Format placeholders must match; empty `pt_br` values retain
the original text. The emitted file is XML, not a playable XTB binary.

## Etapa 3 — Pacote de mod / Step 3 — mod skeleton

~~~bash
python3 tools/ptbr-game-text.py init-mod --out /caminho/privado/MeuModPTBR
~~~

**PT-BR:** O comando cria somente `mod.ini` e a pasta `work/` vazia,
seguindo o padrão do Hedge Mod Manager. Insira nela apenas os seus arquivos
processados e organizados conforme os caminhos que o jogo solicita.
Ative o mod pelo gerenciador compatível. Nenhum mod é instalado
automaticamente; o script não gera arquivos de jogo nem empacota arquivos AR.

**EN:** Mod metadata is generated, but no proprietary assets are packaged.
Actual file placement must match the game's requested archive/resource paths.

## Próxima auditoria / Remaining gameplay coverage

**PT-BR:** Até receber amostras de XTB exportados da sua cópia e tabelas
de tradução FCO/FTE, não é possível afirmar que diálogos, legendas, HUD
original, tutoriais ou cenas foram traduzidos. A prioridade é analisar
esses arquivos e construir os textos pt-BR a partir das fontes autorizadas,
validando acentuação, contexto e duração das legendas.

**EN:** Gameplay dialogue/subtitle completion is not claimed. Asset formats
and in-game rendering still require testing with user-provided licensed data.

## Testes / Tests

~~~bash
python3 tools/tests/test-ptbr-game-text.py
python3 tools/tests/check-pt-br-localization.py
cmake -S UnleashedRecomp/portable/tests -B build/portable-runtime-debug
cmake --build build/portable-runtime-debug
ctest --test-dir build/portable-runtime-debug --output-on-failure
~~~
