# Português (Brasil) / Brazilian Portuguese (pt-BR)

## Incluído / Included

**PT-BR:** A nova tradução acrescenta Português (Brasil) como sétimo idioma
da interface do Sonic Unleashed Recompiled, preservando os identificadores
numéricos dos seis idiomas anteriores. Há tradução das mensagens do instalador,
interface de opções, mensagens de erro, conquistas, menus e botões.
O idioma é salvo como PortugueseBrazil no arquivo config.toml.

**EN:** Adds Brazilian Portuguese as a seventh UI locale without changing
the numeric IDs of the existing six languages. The installer, error dialogs,
port menus, option descriptions, achievements and button labels are translated.
The persistent configuration entry uses the stable ASCII value PortugueseBrazil.

## Escopo e limitações / Scope and limitations

**PT-BR:** A tradução cobre as strings de interface controladas por este
repositório. Ela NÃO inclui nova dublagem, legendas, diálogos, texturas,
logotipos ou outros recursos originais dos arquivos de Xbox 360.
O menu de idioma da dublagem continua oferecendo inglês e japonês.
Recomenda-se conferir visualmente os caracteres ã, õ, ç, ê e demais
acentos nas fontes utilizadas pelo instalador e pelo menu de opções.

**EN:** Interface-only translation; no new Portuguese voice acting or
automatic translations of Xbox 360 game assets, dialogue, subtitles or
baked-in images. The available voice languages remain English and Japanese.
Inspect the embedded font glyphs for accented Portuguese characters.

## Instalação e teste / Build and test

~~~bash
git switch portable
git pull --ff-only
python3 tools/tests/check-pt-br-localization.py
cmake --build out/build/linux-debug --target UnleashedRecomp --parallel 4
bash tools/validate-linux-runtime.sh --game
~~~

Selecione PORTUGUÊS (BR) na grade com sete idiomas do instalador.
Verifique títulos, descrições, instruções, botões e quebras de linha.
Após a instalação, o idioma também estará disponível nas opções
de Sistema. Uma reinicialização pode ser solicitada.

Choose PORTUGUÊS (BR) in the seven-language installer grid. Check menu
width, wrapping, accents, all option categories and error messages.
After installation, use the language setting in the System options;
the application may request a restart.

## Integridade / Translation integrity

**PT-BR:** Textos permanecem como strings UTF-8 em C++, mantendo tokens
de formatação printf e quebras de linha. O teste automatizado confere
todas as chaves, descrições, grupos de enumeração e número de botões.

**EN:** UTF-8 source literals preserve printf specifiers and line breaks.
The CI coverage test checks all source keys, option labels, enum variants
and the installer language selector.

**EN/PT-BR:** Functional code comments are bilingual; original game assets
and third-party submodules have not been modified.

## Conquistas do Xbox 360 / Xbox 360 achievement fallback

**PT-BR:** A base XDBF original contém os seis idiomas oferecidos pelo jogo.
Como não existe uma tradução original dos recursos de conquistas para
Português (Brasil), nomes e descrições de conquistas continuam sendo
obtidos em inglês quando o novo idioma de interface está selecionado.
Isso evita utilizar um identificador de idioma que não existe no XDBF.
O título da interface, botões e mensagens do port permanecem em pt-BR.

**EN:** The original XDBF files contain only six supported asset languages.
With the pt-BR interface selected, achievement names and descriptions use
English XDBF content as a safe fallback. Port UI labels remain Portuguese.

## Recompilar sem default.xex / Rebuilding without default.xex

**PT-BR:** Se o instalador já foi compilado anteriormente, mas o Ninja agora
reclama de `UnleashedRecompLib/private/default.xex`, isso significa que o
gerador de código PPC continua dependendo da entrada original. O executável
antigo pode abrir normalmente mesmo após um erro de compilação; isso
**não** comprova que a tradução nova está presente.

**EN:** A successful launch after Ninja reports a missing `default.xex`
can be the *old* binary. The normal generator depends on the user's
licensed `default.xex`, `default.xexp` and `shader.ar`.

**PT-BR:** Agora é possível reconstruir apenas a aplicação e interface
usando os **261 arquivos `ppc_recomp.N.cpp`**, os cabeçalhos gerados e
o cache de shaders da compilação anterior, caso todos ainda existam.
Este modo é opcional e valida o conjunto antes de alterar a configuração.
Não cria conteúdos originais do jogo nem baixa arquivos licenciados.

**EN:** Source-only rebuilds reuse previously generated PPC and shader files
from an earlier licensed build. The option checks every expected generated
file and refuses to run with missing or empty data. Guest code and shaders
are **not regenerated** in this mode.

~~~bash
git switch portable
git pull --ff-only
python3 tools/tests/check-pt-br-localization.py
bash tools/build-linux-reuse-generated.sh
bash tools/validate-linux-runtime.sh --game
~~~

**PT-BR:** Se o script informar arquivos `ppc` ou `shader_cache.cpp`
ausentes, restaure os originais da sua própria cópia e execute o build
integral: `bash tools/validate-portable-runtime.sh --full`.
Esse modo desativa explicitamente a opção de reaproveitamento.

**EN:** When guest code, recompiler configuration or game data changes,
disable the reuse mode and regenerate from the legally obtained sources.
Do not commit game inputs or generated outputs containing licensed data.

## Textos que pertencem ao jogo / Game-owned text resources

**PT-BR:** A localização do port e os arquivos de mensagens originais
do Xbox 360 são camadas diferentes. Quando Português (Brasil) está ativo,
o runtime informa **inglês** ao jogo pelas APIs XGetLanguage e
ExGetXConfigSetting, preservando os IDs originais e evitando acessar
um idioma inexistente. Traduções de diálogos devem substituir os
recursos ingleses por meio de um mod de dados compatível.

**EN:** Guest code still uses its original six Xbox 360 language IDs.
The shared XboxGuestLanguage mapping converts the pt-BR host locale to
English for guest language and achievement resource calls; a compatible
data mod can then override the English game assets with Portuguese text.

**PT-BR:** O inventário de recursos, a tradução local de XTB/XML e as
instruções sobre FCO/FTE estão em [Tradução de recursos do jogo](GAME_TRANSLATION_PT_BR.md).
Não há diálogos ou legendas originais do jogo incluídos neste repositório.

**EN:** See [Game asset translation workflow](GAME_TRANSLATION_PT_BR.md)
for local XTB editing, resource inventory, and FCO/FTE limitations.
