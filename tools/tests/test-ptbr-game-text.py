#!/usr/bin/env python3
"""EN: Asset-free integration tests for local pt-BR XTB translation pipeline.
PT-BR: Testes completos do pipeline usando mensagens artificiais e originais.
"""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROGRAM = ROOT / "tools" / "ptbr-game-text.py"

# EN: These are newly authored test strings; no game resources are embedded.
# PT-BR: Strings criadas apenas para testes, sem recursos originais do jogo.
TEST_XML = """<?xml version="1.0" encoding="utf-8"?>
<XTBContainer>
  <Styles />
  <Categories>
    <Category name="Tutorial">
      <Entries>
        <Entry name="Collect" style="Normal" is_unicode="false"><Data>Collect %d rings!</Data></Entry>
        <Entry name="Ready" style="Normal" is_unicode="true"><Data>Ready?</Data></Entry>
      </Entries>
    </Category>
  </Categories>
</XTBContainer>
"""


class TestGameTextPipeline(unittest.TestCase):
    def setUp(self) -> None:
        """EN: Isolate files and outputs so tests cannot alter a game install.
        PT-BR: Isola testes para nunca modificar uma instalacao do jogo.
        """
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.xml = self.root / "sample.xtb.xml"
        self.template = self.root / "sample.pt-br.json"
        self.output = self.root / "translated.xtb.xml"
        self.xml.write_text(TEST_XML, encoding="utf-8")

    def tearDown(self) -> None:
        """EN: Delete only test fixtures. PT-BR: Apaga somente dados de teste."""
        self.temp.cleanup()

    def run_tool(self, *args: object, expected: int = 0) -> str:
        """EN: Execute the public CLI and check its return status.
        PT-BR: Executa a interface de linha de comando e valida o retorno.
        """
        proc = subprocess.run([sys.executable, str(PROGRAM), *map(str, args)],
                              text=True, capture_output=True, check=False)
        self.assertEqual(proc.returncode, expected, proc.stdout + proc.stderr)
        return proc.stdout + proc.stderr

    def test_01_round_trip_and_status(self) -> None:
        """EN: Translate text while preserving entry IDs, flags and placeholders.
        PT-BR: Traduz texto preservando IDs, flags e parametros.
        """
        self.run_tool("export", "--xml", self.xml, "--out", self.template)
        doc = json.loads(self.template.read_text(encoding="utf-8"))
        self.assertEqual(doc["format"], "unleashed-ptbr-xtb-v1")
        self.assertEqual(len(doc["entries"]), 2)
        doc["entries"][0]["pt_br"] = "Colete %d anéis!"
        self.template.write_text(json.dumps(doc, ensure_ascii=False), encoding="utf-8")
        info = self.run_tool("audit", "--xml", self.xml, "--translations", self.template)
        self.assertIn("total=2|translated=1|pending=1", info)
        self.run_tool("apply", "--xml", self.xml,
                      "--translations", self.template, "--out", self.output)
        root = ET.parse(self.output).getroot()
        first, second = root.findall("./Categories/Category/Entries/Entry")
        self.assertEqual(first.findtext("Data"), "Colete %d anéis!")
        self.assertEqual(first.attrib["name"], "Collect")
        self.assertEqual(first.attrib["is_unicode"], "false")
        self.assertEqual(second.findtext("Data"), "Ready?")
        self.assertEqual(second.attrib["is_unicode"], "true")
        self.assertEqual(ET.parse(self.xml).getroot().findtext(
            "./Categories/Category/Entries/Entry/Data"), "Collect %d rings!")
        self.run_tool("apply", "--xml", self.xml, "--translations",
                      self.template, "--out", self.output, expected=2)

    def test_02_reject_wrong_placeholders_and_changed_source(self) -> None:
        """EN: Never apply a stale index or delete a formatting argument.
        PT-BR: Nao aplica IDs desatualizados nem elimina parametros.
        """
        self.run_tool("export", "--xml", self.xml, "--out", self.template)
        document = json.loads(self.template.read_text(encoding="utf-8"))
        document["entries"][0]["pt_br"] = "Colete anéis!"
        self.template.write_text(json.dumps(document, ensure_ascii=False), encoding="utf-8")
        self.run_tool("audit", "--xml", self.xml,
                      "--translations", self.template, expected=2)
        document["entries"][0]["pt_br"] = "Colete %d anéis!"
        self.template.write_text(json.dumps(document, ensure_ascii=False), encoding="utf-8")
        self.xml.write_text(TEST_XML.replace("Collect %d rings!", "Collect rings!"),
                            encoding="utf-8")
        self.run_tool("apply", "--xml", self.xml, "--translations",
                      self.template, "--out", self.output, expected=2)
        self.assertFalse(self.output.exists())

    def test_03_reject_unknown_format_and_unsafe_xml(self) -> None:
        """EN: Guard against unexpected binary XML formats and DTD entities.
        PT-BR: Rejeita formatos inesperados e entidades DTD.
        """
        self.xml.write_text('<NotXTBContainer />', encoding="utf-8")
        self.run_tool("export", "--xml", self.xml, "--out", self.template, expected=2)
        self.xml.write_text('<!DOCTYPE XTBContainer [<!ENTITY unsafe "x">]>'
                            '<XTBContainer><Categories /></XTBContainer>', encoding="utf-8")
        self.run_tool("export", "--xml", self.xml, "--out", self.template, expected=2)

    def test_04_archive_candidates(self) -> None:
        """EN: Rank town archives with .arl sidecars without unpacking assets.
        PT-BR: Prioriza arquivos das cidades e identifica o .arl associado.
        """
        game = self.root / "game"
        game.mkdir()
        for name in (
            "Town_EuropeanCity_Common.ar.00",
            "Town_EuropeanCity_Common.arl",
            "WorldMap.ar.00",
            "Sonic.ar.00",
            "Application.ar",
        ):
            (game / name).write_bytes(b"fixture")
        result = self.run_tool("candidates", "--directory", game, "--limit", 2)
        doc = json.loads(result.split("\nINFO:")[0])
        self.assertEqual(doc["candidate_archives"], 3)
        self.assertEqual(doc["shown"], 2)
        self.assertEqual(doc["archives"][0]["archive"],
                         "Town_EuropeanCity_Common.ar.00")
        self.assertEqual(doc["archives"][0]["sidecar_arl"],
                         "Town_EuropeanCity_Common.arl")
        self.assertEqual(doc["archives"][1]["archive"], "WorldMap.ar.00")
        self.run_tool("candidates", "--directory", game, "--limit", 0, expected=2)

    def test_04_inventory_and_mod_skeleton(self) -> None:
        """EN: Inspect file names only, and create no game assets in a mod.
        PT-BR: Inspeciona apenas nomes e nao inclui dados do jogo no mod.
        """
        data = self.root / "install"
        data.mkdir()
        (data / "City.fco").write_bytes(b"example")
        (data / "City.fte").write_bytes(b"example")
        (data / "archive.ar.00").write_bytes(b"example")
        report = self.run_tool("inventory", "--directory", data)
        self.assertIn('".fco": 1', report)
        self.assertIn('".fte": 1', report)
        self.assertIn('".ar.00": 1', report)
        mod = self.root / "ptbr-mod"
        self.run_tool("init-mod", "--out", mod)
        self.assertTrue((mod / "mod.ini").exists())
        self.assertTrue((mod / "work").is_dir())
        self.assertEqual(len(list((mod / "work").iterdir())), 0)
        self.run_tool("init-mod", "--out", mod, expected=2)


if __name__ == "__main__":
    unittest.main()
