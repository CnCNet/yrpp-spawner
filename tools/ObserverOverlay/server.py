"""Local web server for the CnCNet Yuri's Revenge observer overlay.

The spawner writes ``observer_overlay.json`` to the game directory while you observe a game
(``ObserverOverlay=yes`` in RA2MD.INI). This server makes that file, and the overlay pages,
available to OBS as a Browser Source:

    python server.py --game-dir "C:\\Games\\CnCNet\\Yuri's Revenge"

Then add a Browser Source in OBS pointing to http://127.0.0.1:8765/ (all players) or
http://127.0.0.1:8765/player.html?slot=0 (a single player panel, slot 0..7).

Only the Python standard library is used, so no install step is needed.
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import pathlib
import sys
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlsplit

STATE_FILE_NAME = "observer_overlay.json"
DEFAULT_PORT = 8765
TOOL_DIR = pathlib.Path(__file__).resolve().parent
DEFAULT_WWW_DIR = TOOL_DIR / "www"
DEFAULT_UNITS_FILE = TOOL_DIR / "units.json"


def read_state(game_dir: pathlib.Path) -> dict | None:
    """Read the latest snapshot written by the spawner.

    Returns ``None`` if the file doesn't exist yet or can't be parsed.
    The spawner replaces the file atomically, so a parse error should only happen if
    something else wrote to it.
    """
    try:
        raw = (game_dir / STATE_FILE_NAME).read_bytes()
    except OSError:
        return None
    try:
        state = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, ValueError):
        return None
    return state if isinstance(state, dict) else None


def resolve_static_path(www_dir: pathlib.Path, url_path: str) -> pathlib.Path | None:
    """Map a URL path to a file inside ``www_dir``. Returns ``None`` for anything outside of it."""
    relative = url_path.lstrip("/") or "index.html"
    candidate = (www_dir / relative).resolve()
    try:
        candidate.relative_to(www_dir.resolve())
    except ValueError:
        return None
    return candidate if candidate.is_file() else None


def make_handler(game_dir: pathlib.Path, www_dir: pathlib.Path, units_file: pathlib.Path):
    class OverlayRequestHandler(BaseHTTPRequestHandler):
        server_version = "CnCNetObserverOverlay/1"

        def do_GET(self) -> None:  # noqa: N802 - name required by BaseHTTPRequestHandler
            path = urlsplit(self.path).path
            if path == "/state.json":
                self._send_state()
            elif path == "/units.json":
                self._send_file(units_file, "application/json")
            else:
                static_path = resolve_static_path(www_dir, path)
                if static_path is None:
                    self._send_json({"error": "not-found"}, HTTPStatus.NOT_FOUND)
                    return
                content_type = mimetypes.guess_type(static_path.name)[0] or "application/octet-stream"
                self._send_file(static_path, content_type)

        def _send_state(self) -> None:
            state = read_state(game_dir)
            if state is None:
                self._send_json({"status": "waiting-for-game"}, HTTPStatus.SERVICE_UNAVAILABLE)
                return
            self._send_json(state, HTTPStatus.OK)

        def _send_file(self, file_path: pathlib.Path, content_type: str) -> None:
            try:
                body = file_path.read_bytes()
            except OSError:
                self._send_json({"error": "not-found"}, HTTPStatus.NOT_FOUND)
                return
            self._send_body(body, content_type, HTTPStatus.OK)

        def _send_json(self, payload: dict, status: HTTPStatus) -> None:
            self._send_body(json.dumps(payload).encode("utf-8"), "application/json", status)

        def _send_body(self, body: bytes, content_type: str, status: HTTPStatus) -> None:
            self.send_response(status)
            if content_type.startswith("text/") or content_type == "application/json":
                content_type += "; charset=utf-8"
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, format: str, *args) -> None:  # noqa: A002 - signature from the base class
            # The overlay polls twice a second; don't flood the console.
            return

    return OverlayRequestHandler


def create_server(
    game_dir: pathlib.Path,
    host: str = "127.0.0.1",
    port: int = DEFAULT_PORT,
    www_dir: pathlib.Path = DEFAULT_WWW_DIR,
    units_file: pathlib.Path = DEFAULT_UNITS_FILE,
) -> ThreadingHTTPServer:
    handler = make_handler(game_dir, www_dir, units_file)
    return ThreadingHTTPServer((host, port), handler)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Serve the CnCNet observer overlay to OBS.")
    parser.add_argument(
        "--game-dir",
        type=pathlib.Path,
        default=pathlib.Path.cwd(),
        help=f"Folder that contains gamemd.exe, where the spawner writes {STATE_FILE_NAME}. Default: current folder.",
    )
    parser.add_argument("--host", default="127.0.0.1", help="Address to listen on. Default: 127.0.0.1")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Port to listen on. Default: {DEFAULT_PORT}")
    parser.add_argument(
        "--units",
        type=pathlib.Path,
        default=DEFAULT_UNITS_FILE,
        help="JSON file listing the units and buildings to count on the overlay.",
    )
    args = parser.parse_args(argv)

    if not args.game_dir.is_dir():
        parser.error(f"--game-dir does not exist: {args.game_dir}")

    server = create_server(args.game_dir, args.host, args.port, units_file=args.units)
    print(f"Observer overlay: http://{args.host}:{args.port}/  (reading {args.game_dir / STATE_FILE_NAME})")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
