#!/usr/bin/env python3
"""EN: Local, asset-safe XTB translation workflow for a legally installed game.
PT-BR: Fluxo local de traducao XTB para arquivos da propria copia do jogo.

This never downloads game data, extracts archives, or writes in-place originals.
Nao baixa jogos, nao extrai arquivos .ar e nao altera os originais.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

SCHEMA = "unleashed-ptbr-xtb-v1"
MAX_XML_BYTES = 64 * 1024 * 1024
EXTENSIONS = (".xtb", ".fco", ".fte", ".dds", ".ar", ".arl", ".ar.00", ".xml")
FORMAT_TOKEN = re.compile(r"%(?:\d+\$)?[-+ #0]*(?:\d+|\*)?(?:\.(?:\d+|\*))?[diuoxXfFeEgGaAcsp]")
INDEX_TOKEN = re.compile(r"\{\d+(?::[^}]*)?\}")


class TranslationError(Exception):
    """EN: A recoverable user-input error. PT-BR: Erro recuperavel da entrada."""


def read_xml(file: Path) -> tuple[ET.Element, bytes]:
    """EN: Parse a bounded XML export; prevent unsafe or unknown structures.
    PT-BR: Analisa XML exportado com limite de tamanho e estrutura conhecida.
    """
    if not file.is_file():
        raise TranslationError(f"XML not found / XML nao encontrado: {file}")
    if file.stat().st_size > MAX_XML_BYTES:
        raise TranslationError("XTB XML exceeds 64 MiB / XML maior que 64 MiB")
    data = file.read_bytes()
    if b"<!DOCTYPE" in data.upper() or b"<!ENTITY" in data.upper():
        raise TranslationError("DTD/entity declarations are not accepted / DTD nao aceito")
    try:
        root = ET.fromstring(data)
    except ET.ParseError as exc:
        raise TranslationError(f"Invalid XML / XML invalido: {exc}") from exc
    if root.tag != "XTBContainer":
        raise TranslationError("Expected XTBContainer exported with sonic2010/swa converter")
    return root, data


def get_entries(root: ET.Element) -> list[tuple[int, int, ET.Element, ET.Element, ET.Element]]:
    """EN: Locate converter-compatible category and entry data nodes.
    PT-BR: Identifica categorias e dados de entradas do formato do conversor.
    """
    categories = root.find("Categories")
    if categories is None:
        raise TranslationError("Missing Categories node / Categorias ausentes")
    found = []
    for ci, category in enumerate(categories.findall("Category")):
        entries = category.find("Entries")
        if entries is None:
            raise TranslationError(f"Missing Entries in category {ci}")
        for ei, entry in enumerate(entries.findall("Entry")):
            datum = entry.find("Data")
            if datum is None or list(datum):
                raise TranslationError(
                    f"Missing or structured Data in category {ci}, entry {ei}; "
                    "only plain-text XTB Data is supported"
                )
            found.append((ci, ei, category, entry, datum))
    if not found:
        raise TranslationError("No XTB entries found / Nenhuma entrada XTB encontrada")
    return found


def tokens(text: str) -> collections.Counter:
    """EN: Compare printf and indexed placeholders without changing content.
    PT-BR: Compara parametros printf e referencias numericas sem alteracoes.
    """
    return collections.Counter(FORMAT_TOKEN.findall(text) + INDEX_TOKEN.findall(text))


def safe_output(input_path: Path, output: Path) -> Path:
    """EN: Never overwrite a licensed or exported source in place.
    PT-BR: Nunca sobrescreve arquivos originais ou XMLs de entrada.
    """
    if input_path.resolve() == output.resolve():
        raise TranslationError("Output must differ from input / Saida deve ser diferente")
    if output.exists():
        raise TranslationError(f"Output already exists / Saida ja existe: {output}")
    if not output.parent.is_dir():
        raise TranslationError(f"Output directory missing / Diretorio ausente: {output.parent}")
    return output


def export_template(input_path: Path, output: Path) -> None:
    """EN: Emit a local user-owned JSON editing template; do not commit game text.
    PT-BR: Gera um JSON local para traducao; nao publique textos licenciados.
    """
    root, raw = read_xml(input_path)
    entries = []
    for ci, ei, category, entry, datum in get_entries(root):
        entries.append({
            "category_index": ci,
            "entry_index": ei,
            "category": category.attrib.get("name", ""),
            "key": entry.attrib.get("name", ""),
            "source": datum.text or "",
            "pt_br": "",
        })
    result = {
        "format": SCHEMA,
        "resource": input_path.name,
        "source_sha256": hashlib.sha256(raw).hexdigest(),
        "entries": entries,
    }
    safe_output(input_path, output)
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"PTBR_XTB|EXPORTED|entries={len(entries)}|path={output}")
    print("INFO: keep this template local / mantenha este arquivo local")


def open_template(path: Path) -> dict:
    """EN: Validate translation JSON before touching output files.
    PT-BR: Valida o JSON antes de produzir arquivos traduzidos.
    """
    try:
        data = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, ValueError) as exc:
        raise TranslationError(f"Cannot read JSON / Falha no JSON: {exc}") from exc
    if not isinstance(data, dict) or data.get("format") != SCHEMA:
        raise TranslationError(f"Unexpected translation format / Formato inesperado: {SCHEMA}")
    if not isinstance(data.get("entries"), list):
        raise TranslationError("Missing entries list / Lista entries ausente")
    return data


def apply_template(input_path: Path, translations: Path, output: Path | None) -> None:
    """EN: Apply only explicit pt-BR entries, preserving the XML hierarchy.
    PT-BR: Altera somente textos pt-BR preenchidos, mantendo a hierarquia XML.
    """
    root, raw = read_xml(input_path)
    data = open_template(translations)
    original_hash = hashlib.sha256(raw).hexdigest()
    if data.get("source_sha256") != original_hash:
        raise TranslationError(
            "Source XML differs from export. Re-export to avoid mismatched entry IDs. / "
            "XML mudou desde a exportacao: gere um novo modelo."
        )
    entries = get_entries(root)
    if len(data["entries"]) != len(entries):
        raise TranslationError("Entry count differs / Quantidade de entradas diferente")
    completed = 0
    for (ci, ei, category, entry, datum), translated in zip(entries, data["entries"]):
        if not isinstance(translated, dict) or (
            translated.get("category_index") != ci
            or translated.get("entry_index") != ei
            or translated.get("category") != category.attrib.get("name", "")
            or translated.get("key") != entry.attrib.get("name", "")
            or translated.get("source") != (datum.text or "")
        ):
            raise TranslationError(
                f"Source entry was changed / Entrada original alterada: {ci}/{ei}"
            )
        pt = translated.get("pt_br")
        if not isinstance(pt, str):
            raise TranslationError(f"pt_br must be a string / Campo deve ser texto: {ci}/{ei}")
        if not pt.strip():
            continue
        if "\x00" in pt:
            raise TranslationError("NUL character in translated text / Caractere NUL")
        if tokens(pt) != tokens(datum.text or ""):
            raise TranslationError(
                f"Placeholder mismatch / Parametros de formato diferentes: {ci}/{ei}"
            )
        if output is not None:
            datum.text = pt
        completed += 1
    print(f"PTBR_XTB|AUDIT|total={len(entries)}|translated={completed}|pending={len(entries)-completed}")
    if output is None:
        return
    if completed == 0:
        raise TranslationError("No pt-BR entries filled / Nenhuma traducao preenchida")
    safe_output(input_path, output)
    if translations.resolve() == output.resolve():
        raise TranslationError("Output must differ from the translation JSON / Saida invalida")
    ET.indent(root, space="  ")
    ET.ElementTree(root).write(output, encoding="utf-8", xml_declaration=True)
    print(f"PTBR_XTB|PATCHED_XML|{output}")


def inventory(directory: Path, limit: int) -> None:
    """EN: Read only relative names and sizes; never copy licensed files.
    PT-BR: Le somente nomes relativos e tamanhos; nao copia arquivos do jogo.
    """
    if not directory.is_dir():
        raise TranslationError(f"Folder not found / Pasta nao encontrada: {directory}")
    counts: collections.Counter[str] = collections.Counter()
    samples: dict[str, list[str]] = collections.defaultdict(list)
    seen = 0
    for candidate in directory.rglob("*"):
        if candidate.is_symlink() or not candidate.is_file():
            continue
        seen += 1
        if seen > limit:
            raise TranslationError(f"File scan limit reached ({limit}); use a smaller folder")
        name = candidate.name.lower()
        kind = next((ext for ext in sorted(EXTENSIONS, key=len, reverse=True)
                     if name.endswith(ext)), None)
        if kind is None:
            continue
        counts[kind] += 1
        if len(samples[kind]) < 8:
            samples[kind].append(candidate.relative_to(directory).as_posix())
    result = {"format": "unleashed-ptbr-inventory-v1",
              "files_examined": seen, "counts": dict(sorted(counts.items())),
              "examples": dict(sorted(samples.items()))}
    print(json.dumps(result, indent=2, ensure_ascii=False))
    print("INFO: names only; no game content read / apenas nomes; sem ler conteudo")



def candidates(directory: Path, limit: int, max_files: int) -> None:
    """EN: Rank likely dialogue/UI archives by filename without opening any.
    PT-BR: Prioriza arquivos possiveis de dialogos pela nomenclatura, sem abri-los.
    """
    if not directory.is_dir():
        raise TranslationError(f"Folder not found / Pasta nao encontrada: {directory}")
    if limit <= 0 or max_files <= 0:
        raise TranslationError("Limits must be positive / Limites devem ser positivos")
    scored: list[tuple[int, str, dict]] = []
    seen = 0
    for archive in directory.rglob("*"):
        if archive.is_symlink() or not archive.is_file():
            continue
        seen += 1
        if seen > max_files:
            raise TranslationError(
                f"File scan limit reached ({max_files}); use a smaller folder"
            )
        lower = archive.name.lower()
        if lower.endswith(".ar.00"):
            stem = archive.name[:-6]
        elif lower.endswith(".ar"):
            stem = archive.name[:-3]
        else:
            continue
        name = stem.lower()
        if name.startswith("town_") and "_common" in name:
            score, reason = 1, "town/common: NPC dialogue candidate"
        elif name.startswith("town_"):
            score, reason = 2, "town area: possible local text"
        elif name.startswith("worldmap"):
            score, reason = 3, "world map: possible menu text"
        elif name.startswith("hint") or name.startswith("tutorial"):
            score, reason = 4, "hints/tutorial: possible instructional text"
        elif name.startswith("application") or name.startswith("title"):
            score, reason = 5, "application/title: possible UI resources"
        else:
            continue
        sidecar = archive.with_name(stem + ".arl")
        record = {
            "archive": archive.relative_to(directory).as_posix(),
            "sidecar_arl": sidecar.relative_to(directory).as_posix()
            if sidecar.is_file() else None,
            "priority": score,
            "reason": reason,
        }
        scored.append((score, record["archive"].lower(), record))
    scored.sort(key=lambda item: (item[0], item[1]))
    print(json.dumps({
        "format": "unleashed-ptbr-archive-candidates-v1",
        "candidate_archives": len(scored),
        "shown": min(limit, len(scored)),
        "archives": [entry for _, _, entry in scored[:limit]],
    }, ensure_ascii=False, indent=2))
    print("INFO: filename ranking only; archive contents NOT verified")
    print("INFO: prioridade por nome; conteudo interno NAO verificado")


def init_mod(output: Path) -> None:
    """EN: Create only an HMM-compatible metadata skeleton, not game assets.
    PT-BR: Gera apenas metadados HMM; nao inclui dados do Xbox 360.
    """
    if output.exists() and any(output.iterdir()):
        raise TranslationError(f"Output directory must be empty / Pasta nao vazia: {output}")
    output.mkdir(parents=True, exist_ok=True)
    (output / "work").mkdir(exist_ok=True)
    (output / "mod.ini").write_text(
        "[Main]\n"
        'ID="org.local.unleashed.ptbr.text"\n'
        "IncludeDirCount=1\n"
        'IncludeDir0="."\n\n'
        "[Desc]\n"
        'Title="Português (Brasil) - recursos de texto"\n'
        'Description="Recursos próprios de localização pt-BR."\n'
        'Author="Local"\n'
        'Version="0.1.0"\n',
        encoding="utf-8",
    )
    print(f"PTBR_MOD|SKELETON|{output}")
    print("INFO: add only files you edited legally; enable through a compatible mod manager")


def make_parser() -> argparse.ArgumentParser:
    """EN: Separate read-only audits and opt-in writes.
    PT-BR: Separa auditoria sem escrita de criacao opcional de arquivos.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_subparsers(dest="command", required=True)
    scan = actions.add_parser("inventory", help="Scan installed resource extensions")
    scan.add_argument("--directory", type=Path, required=True)
    scan.add_argument("--max-files", type=int, default=100000)
    pick = actions.add_parser("candidates", help="Rank likely text archives by name")
    pick.add_argument("--directory", type=Path, required=True)
    pick.add_argument("--limit", type=int, default=12)
    pick.add_argument("--max-files", type=int, default=100000)
    export = actions.add_parser("export", help="Export an XTB-converter XML as translation JSON")
    export.add_argument("--xml", type=Path, required=True)
    export.add_argument("--out", type=Path, required=True)
    audit = actions.add_parser("audit", help="Check translation progress without writing")
    audit.add_argument("--xml", type=Path, required=True)
    audit.add_argument("--translations", type=Path, required=True)
    apply = actions.add_parser("apply", help="Generate an XML ready for the XTB converter")
    apply.add_argument("--xml", type=Path, required=True)
    apply.add_argument("--translations", type=Path, required=True)
    apply.add_argument("--out", type=Path, required=True)
    create = actions.add_parser("init-mod", help="Create an empty compatible mod skeleton")
    create.add_argument("--out", type=Path, required=True)
    return parser


def main() -> int:
    """EN: Report safe, actionable failures without exposing game file bytes.
    PT-BR: Mostra erros claros sem expor o conteudo dos arquivos do jogo.
    """
    args = make_parser().parse_args()
    try:
        if args.command == "inventory":
            if args.max_files <= 0:
                raise TranslationError("max-files must be positive")
            inventory(args.directory, args.max_files)
        elif args.command == "candidates":
            candidates(args.directory, args.limit, args.max_files)
        elif args.command == "export":
            export_template(args.xml, args.out)
        elif args.command == "audit":
            apply_template(args.xml, args.translations, None)
        elif args.command == "apply":
            apply_template(args.xml, args.translations, args.out)
        elif args.command == "init-mod":
            init_mod(args.out)
        return 0
    except (TranslationError, OSError) as exc:
        print(f"PTBR_XTB|ERROR|{exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
