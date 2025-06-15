from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import subprocess
import threading

class SimpleHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            self.serve_file("index.html", "text/html")
        elif self.path == "/main.js":
            self.serve_file("main.js", "application/javascript")
        elif self.path == "/style.css":
            self.serve_file("style.css", "text/css")
        else:
            self.send_error(404, "File not found")
    
    def serve_file(self, filename, content_type):
        try:
            with open(filename, "rb") as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.end_headers()
            self.wfile.write(content)
        except:
            self.send_error(500, "Error loading file")

    def do_POST(self):
        if self.path == '/run':
            length  = int(self.headers.get('Content-Length'))
            body = self.rfile.read(length)
            data = json.loads(body)

            code = data.get('code', '')
            result = self.interpret_code(code)

            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(result.encode())

            print("Automatic server shutdown after processing.")
            def shutdown_server():
                httpd.shutdown()
            threading.Thread(target=shutdown_server).start()
        else:
            self.send_error(405, "Unauthorized method for this URL.")
    
    def interpret_code(self, code):
        if code.strip().startswith("print"):
            message = code.strip()[6:-1]
            try:
                result = subprocess.check_output(['./print_exec', message])
                return result.decode()
            except Exception as e:
                return 'Error C++ : ' + str(e)
        return 'Unknown order'

if __name__ == '__main__':
    httpd = HTTPServer(('localhost', 5500), SimpleHandler)
    print('🚀 Python server ready at http://localhost:5500')
    httpd.serve_forever()