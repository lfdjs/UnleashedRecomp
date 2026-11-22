#!/usr/bin/env python3
"""EN: Audit the Brazilian Portuguese source strings without game assets.
PT-BR: Audita as strings pt-BR sem usar arquivos licenciados do jogo.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LANG = "ELanguage::PortugueseBrazil"


def read(relative: str) -> str:
    """EN: Read UTF-8 C++ text and fail on invalid encoding.
    PT-BR: Le os fontes C++ em UTF-8 e detecta codificacao invalida.
    """
    return (ROOT / relative).read_text(encoding="utf-8")


def section_until(source: str, start: int, token: str) -> str:
    """EN: Return an initializer block up to a known closing token.
    PT-BR: Retorna um bloco de inicializacao ate seu fechamento.
    """
    end = source.find(token, start)
    if end < 0:
        raise AssertionError(f"Missing section terminator at byte {start}")
    return source[start:end]


def cpp_string(block: str, language: str) -> str:
    """EN: Decode a C++ quoted literal containing JSON-compatible escapes.
    PT-BR: Decodifica um literal C++ com escapes compativeis com JSON.
    """
    match = re.search(
        r"\{\s*ELanguage::" + re.escape(language) +
        r'\s*,\s*("(?:\\.|[^"\\])*")\s*\}',
        block,
    )
    if match is None:
        raise AssertionError(f"Missing {language} translation")
    return json.loads(match.group(1))


def format_tokens(text: str) -> tuple[str, ...]:
    """EN: Preserve printf format tokens, including field width and type.
    PT-BR: Preserva parametros printf com largura, precisao e tipo.
    """
    return tuple(re.findall(
        r"%(?!%)(?:[-+ #0]*\d*(?:\.\d+)?)?[sdifuxX]",
        text,
    ))


def validate_ui() -> int:
    """EN: Check each English UI key has one pt-BR translation and valid tokens.
    PT-BR: Confere todas as chaves da interface e os parametros de formato.
    """
    source = read("UnleashedRecomp/locale/locale.cpp")
    keys = list(re.finditer(r'^\s{8}"([A-Za-z0-9_]+)",\s*$', source, re.MULTILINE))
    assert keys, "No UI locale keys"
    for i, key in enumerate(keys):
        end = keys[i + 1].start() if i + 1 < len(keys) else source.index("std::string& Localise(")
        block = source[key.start():end]
        occurrences = len(re.findall(r"\{\s*" + re.escape(LANG) + r"\s*,", block))
        assert occurrences == 1, f"Invalid pt-BR count for {key.group(1)}: {occurrences}"
        english = cpp_string(block, "English")
        portuguese = cpp_string(block, "PortugueseBrazil")
        assert portuguese.strip(), f"Empty pt-BR string for {key.group(1)}"
        assert format_tokens(english) == format_tokens(portuguese), (
            f"Format mismatch for {key.group(1)}: {english!r} / {portuguese!r}"
        )
    return len(keys)


def validate_config() -> tuple[int, int]:
    """EN: Validate localized setting descriptions and every English enum value.
    PT-BR: Confere descricoes das opcoes e valores enumerados disponiveis.
    """
    source = read("UnleashedRecomp/locale/config_locale.cpp")
    direct = list(re.finditer(r"^CONFIG_DEFINE_LOCALE\((\w+)\)", source, re.MULTILINE))
    groups = list(re.finditer(r"^CONFIG_DEFINE_ENUM_LOCALE\((\w+)\)", source, re.MULTILINE))
    assert direct and groups, "Missing configuration locale blocks"
    for key in direct:
        block = section_until(source, key.start(), "\n};")
        assert len(re.findall(r"\{\s*" + re.escape(LANG) + r"\s*,", block)) == 1, (
            f"Invalid option translation: {key.group(1)}"
        )
        assert re.search(
            r'\{\s*' + re.escape(LANG) +
            r'\s*,\s*\{\s*"(?:\\.|[^"\\])*"\s*,\s*"(?:\\.|[^"\\])*"\s*\}\s*\}',
            block
        ), f"Malformed name/description: {key.group(1)}"

    for key in groups:
        block = section_until(source, key.start(), "\n};")
        if key.group(1) == "ELanguage":
            # EN: Display language names in their native spelling.
            # PT-BR: Exibe nomes de idiomas com a grafia nativa.
            assert re.search(
                r'\{\s*' + re.escape(LANG) + r'\s*,\s*\{\s*"PORTUGUÊS \(BRASIL\)"',
                block,
            ), "Missing Portuguese language label"
            continue

        pt = re.search(r'\b' + re.escape(LANG) + r'\s*,\s*\{', block)
        assert pt, f"Missing enum language block: {key.group(1)}"
        english_part = block[block.index("ELanguage::English,"):pt.start()]
        pt_part = block[pt.end():]
        variant = re.compile(r"\{\s*" + re.escape(key.group(1)) + r"::(\w+)\s*,\s*\{")
        en_values = set(variant.findall(english_part))
        pt_values = set(variant.findall(pt_part))
        assert en_values and en_values == pt_values, (
            f"Missing enum variants for {key.group(1)}: {en_values - pt_values}"
        )

    return len(direct), len(groups)


def validate_selection() -> None:
    """EN: Preserve existing enum values, user config, and seven installer rows.
    PT-BR: Preserva os IDs, a configuracao persistente e os sete idiomas.
    """
    header = read("UnleashedRecomp/locale/locale.h")
    match = re.search(r"enum class ELanguage[^{}]*\{([^{}]+)\}", header, re.DOTALL)
    assert match is not None, "ELanguage declaration missing"
    values = re.findall(
        r"^\s*(English|Japanese|German|French|Spanish|Italian|PortugueseBrazil)\b",
        match.group(1), re.MULTILINE,
    )
    assert values == [
        "English", "Japanese", "German", "French",
        "Spanish", "Italian", "PortugueseBrazil",
    ], f"Legacy language enum values changed: {values}"

    config = read("UnleashedRecomp/user/config.cpp")
    assert re.search(
        r'\{\s*"PortugueseBrazil"\s*,\s*' + re.escape(LANG),
        config
    ), "Missing serialized configuration locale"

    wizard = read("UnleashedRecomp/ui/installer_wizard.cpp")
    names = section_until(wizard, wizard.index("const char* LANGUAGE_TEXT[]"), "\n};")
    choices = section_until(wizard, wizard.index("const ELanguage LANGUAGE_ENUM[]"), "\n};")
    assert len(re.findall(r'^\s*"(?:\\.|[^"\\])*"', names, re.MULTILINE)) == 7
    assert len(re.findall(r'^\s*ELanguage::\w+', choices, re.MULTILINE)) == 7
    assert LANG in choices
    assert "std::size(LANGUAGE_ENUM)" in wizard
    assert "leftColumnCount = 4" in wizard


def main() -> int:
    """EN: Report completeness of each translation layer for the build log.
    PT-BR: Informa a cobertura do idioma no registro do CI.
    """
    ui = validate_ui()
    options, enums = validate_config()
    validate_selection()
    print(f"PT_BR_LOCALE|PASS|ui={ui}|option_labels={options}|enum_groups={enums}|picker=7")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, ValueError, UnicodeError) as exc:
        print(f"PT_BR_LOCALE|FAIL|{exc}", file=sys.stderr)
        raise SystemExit(1)
