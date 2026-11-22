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
