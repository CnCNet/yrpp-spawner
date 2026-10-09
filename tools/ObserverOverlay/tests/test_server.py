"""Tests for the observer overlay server. Run with: python -m unittest discover -s tools/ObserverOverlay/tests"""

import json
import pathlib
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

import server  # noqa: E402

SAMPLE_STATE = {
    "version": 1,
    "frame": 1800,
    "players": [
        {
            "index": 0,
            "name": "Kane",
            "country": "Russians",
            "color": "#E01008",
            "credits": 4250,
            "power": {"output": 300, "drain": 350},
            "defeated": False,
            "production": [],
            "counts": [{"category": "Vehicle", "id": "HTNK", "name": "Rhino Tank", "count": 6}],
        }
    ],
}


class OverlayServerTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self._tmp.name)
        self.game_dir = self.root / "game"
        self.game_dir.mkdir()
        self.www_dir = self.root / "www"
        self.www_dir.mkdir()
        (self.www_dir / "index.html").write_text("<p>overlay</p>", encoding="utf-8")
        (self.root / "secret.txt").write_text("do not serve", encoding="utf-8")
        self.units_file = self.root / "units.json"
        self.units_file.write_text(json.dumps({"units": [{"id": "HTNK"}], "buildings": []}), encoding="utf-8")

        self.httpd = server.create_server(
            self.game_dir, port=0, www_dir=self.www_dir, units_file=self.units_file
        )
        self.base_url = f"http://127.0.0.1:{self.httpd.server_address[1]}"
        self.thread = threading.Thread(target=self.httpd.serve_forever, daemon=True)
        self.thread.start()

    def tearDown(self):
        self.httpd.shutdown()
        self.httpd.server_close()
        self.thread.join(timeout=5)
        self._tmp.cleanup()

    def get(self, path):
        try:
            with urllib.request.urlopen(self.base_url + path, timeout=5) as response:
                return response.status, response.headers, response.read()
        except urllib.error.HTTPError as error:
            return error.code, error.headers, error.read()

    def test_state_waits_for_game(self):
        status, _, body = self.get("/state.json")

        self.assertEqual(status, 503)
        self.assertEqual(json.loads(body), {"status": "waiting-for-game"})

    def test_state_returns_latest_snapshot(self):
        (self.game_dir / server.STATE_FILE_NAME).write_text(json.dumps(SAMPLE_STATE), encoding="utf-8")

        status, headers, body = self.get("/state.json")

        self.assertEqual(status, 200)
        self.assertEqual(json.loads(body), SAMPLE_STATE)
        self.assertEqual(headers["Cache-Control"], "no-store")
        self.assertTrue(headers["Content-Type"].startswith("application/json"))

    def test_state_ignores_corrupt_file(self):
        (self.game_dir / server.STATE_FILE_NAME).write_bytes(b'{"version": 1, "players": [')

        status, _, _ = self.get("/state.json")

        self.assertEqual(status, 503)

    def test_serves_index_and_units(self):
        status, headers, body = self.get("/")
        self.assertEqual(status, 200)
        self.assertIn(b"overlay", body)
        self.assertTrue(headers["Content-Type"].startswith("text/html"))

        status, _, body = self.get("/units.json")
        self.assertEqual(status, 200)
        self.assertEqual(json.loads(body)["units"], [{"id": "HTNK"}])

    def test_static_files_cannot_escape_www_dir(self):
        for path in ["/../secret.txt", "/%2e%2e/secret.txt", "/missing.html"]:
            with self.subTest(path=path):
                status, _, body = self.get(path)
                self.assertEqual(status, 404)
                self.assertNotIn(b"do not serve", body)

    def test_resolve_static_path(self):
        self.assertEqual(server.resolve_static_path(self.www_dir, "/"), (self.www_dir / "index.html").resolve())
        self.assertIsNone(server.resolve_static_path(self.www_dir, "/../secret.txt"))

    def test_bundled_units_config_is_valid(self):
        config = json.loads(server.DEFAULT_UNITS_FILE.read_text(encoding="utf-8"))
        for group in ("units", "buildings"):
            self.assertTrue(config[group])
            for entry in config[group]:
                self.assertRegex(entry["id"], r"^[A-Z0-9]+$")


if __name__ == "__main__":
    unittest.main()
