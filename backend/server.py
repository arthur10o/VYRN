from http.server import BaseHTTPRequestHandler, HTTPServer
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
        body = self.rfile.read(content_length)

        try:
            data = json.loads(body)
            code = data.get('code')
            if code is None:
                raise ValueError("Missing 'code' field")
        except Exception as e:
            self._set_headers(400, 'application/json')
            self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))
            return

        os.makedirs(os.path.dirname(CODE_FILE), exist_ok=True)
        with open(CODE_FILE, 'w', encoding='utf-8') as f:
            f.write(code)

        try:
            completed = subprocess.run([CPP_EXECUTABLE], check=True, capture_output=True, text=True)

            if os.path.exists(OUTPUT_FILE):
                with open(OUTPUT_FILE, 'r', encoding='utf-8') as f:
                    result = f.read()
            else:
                result = 'Erreur: le fichier de sortie est manquant.'

            self._set_headers(200, 'text/plain')
            self.wfile.write(result.encode('utf-8'))

        except subprocess.CalledProcessError as e:
            self._set_headers(500, 'text/plain')
            print("Erreur subprocess:", e.stderr)
            error_msg = f"Erreur lors de l'exécution du parseur C++:\n{e.stderr}"
            self.wfile.write(error_msg.encode('utf-8'))


def run_server():
    print(f"Serveur démarré sur http://{HOST}:{PORT}")
    server = HTTPServer((HOST, PORT), SimpleHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nArrêt du serveur.")
        server.server_close()


if __name__ == '__main__':
    run_server()
