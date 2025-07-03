from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import subprocess
import os
import mimetypes

HOST = 'localhost'
PORT = 5500

FRONTEND_DIR = 'frontend'
CODE_FILE = 'communication/input_code.txt'
OUTPUT_FILE = 'communication/program_output.txt'
CPP_EXECUTABLE = './backend/parser/parser_exec'
PARSING_ERRORS_PATH = 'communication/parsing_errors.txt'
COMPILE_ERRORS_PATH = 'communication/compile_errors.txt'
MAX_CODE_SIZE = 100_000

class SimpleHandler(BaseHTTPRequestHandler):
    def _set_headers(self, status=200, content_type='text/html'):
        self.send_response(status)
        self.send_header('Content-type', content_type)
        self.end_headers()

    def do_GET(self):
        path = self.path
        if path == '/':
            path = '/index.html'
        file_path = os.path.join(FRONTEND_DIR, path.strip('/'))
        if os.path.exists(file_path):
            content_type, _ = mimetypes.guess_type(file_path)
            self._set_headers(200, content_type or 'application/octet-stream')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
        else:
            self._set_headers(404)
            self.wfile.write(b'404 Not Found')

    def do_POST(self):
        if self.path != '/run':
            self._set_headers(404, 'application/json')
            self.wfile.write(b'{"error": "Endpoint not found"}')
            return
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length > MAX_CODE_SIZE:
            self._set_headers(413, 'application/json')
            self.wfile.write(json.dumps({'error': 'Code trop volumineux'}).encode('utf-8'))
            return
        body = self.rfile.read(content_length)
        try:
            data = json.loads(body)
            code = data.get('CODE')
            if code is None:
                raise ValueError("Missing 'code' field")
            if len(code) > MAX_CODE_SIZE:
                raise ValueError("Code trop volumineux")
        except Exception as e:
            self._set_headers(400, 'application/json')
            self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))
            return
        
        for path in (PARSING_ERRORS_PATH, COMPILE_ERRORS_PATH, OUTPUT_FILE):
            if os.path.exists(path):
                try:
                    os.remove(path)
                except Exception:
                    pass
        os.makedirs(os.path.dirname(CODE_FILE), exist_ok=True)
        with open(CODE_FILE, 'w', encoding='utf-8') as f:
            f.write(code)
        try:
            completed = subprocess.run([CPP_EXECUTABLE], check=True, capture_output=True, text=True, timeout=10)
        except subprocess.TimeoutExpired:
            self._set_headers(504, 'text/plain')
            self.wfile.write("Timeout lors de l'exécution du parseur C++.".encode('utf-8'))
            return
        except subprocess.CalledProcessError as e:
            self._set_headers(500, 'text/plain')
            error_msg = f"Erreur lors de l'exécution du parseur C++:\n{e.stderr}".encode('utf-8')
            self.wfile.write(error_msg)
            return

        if os.path.exists(PARSING_ERRORS_PATH):
            with open(PARSING_ERRORS_PATH, 'r', encoding='utf-8') as f:
                error_content = f.read()
            if error_content:
                self._set_headers(400, 'text/plain')
                self.wfile.write(error_content.encode('utf-8'))
                os.remove(PARSING_ERRORS_PATH)
                return
            os.remove(PARSING_ERRORS_PATH)
        if os.path.exists(COMPILE_ERRORS_PATH):
            with open(COMPILE_ERRORS_PATH, 'r', encoding='utf-8') as f:
                compile_errors = f.read()
            if compile_errors:
                self._set_headers(400, 'text/plain')
                self.wfile.write(("Compilation errors:\n" + compile_errors).encode('utf-8'))
                os.remove(COMPILE_ERRORS_PATH)
                return
            os.remove(COMPILE_ERRORS_PATH)
        if os.path.exists(OUTPUT_FILE):
            with open(OUTPUT_FILE, 'r', encoding='utf-8') as f:
                result = f.read()
            self._set_headers(200, 'text/plain')
            self.wfile.write(result.encode('utf-8'))
            os.remove(OUTPUT_FILE)
        else:
            self._set_headers(200, 'text/plain')
            self.wfile.write(b'No output generated.')

def run_server():
    print(f"Serveur démarré sur http://{HOST}:{PORT}")
    server = ThreadingHTTPServer((HOST, PORT), SimpleHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nArrêt du serveur.")
        server.server_close()

if __name__ == '__main__':
    run_server()