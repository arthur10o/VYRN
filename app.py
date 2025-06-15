from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import subprocess
import threading
import re
import os

class PrintNode:
    def __init__(self, value):
        self.value = value

class LetNode:
    def __init__(self, name, value):
        self.name = name
        self.value = value

class PrintVarNode:
    def __init__(self, var_name):
        self.var_name = var_name

class Parser:
    def parse(self, source):
        code = source.strip()
        statements = []

        code = re.sub(r'//.*', '', code)
        code = code.replace('\n', ' ').replace('\r', '')

        raw_statements = [stmt.strip() for stmt in code.split(';') if stmt.strip()]

        for stmt in raw_statements:
            if stmt.startswith("print(") and stmt.endswith(")"):
                str_match = re.match(r'print\(["\'](.*?)["\']\)', stmt)
                var_match = re.match(r'print\(([a-zA-Z_][a-zA-Z0-9_]*)\)', stmt)
                if str_match:
                    statements.append(PrintNode(str_match.group(1)))
                elif var_match:
                    statements.append(PrintVarNode(var_match.group(1)))
                else:
                    raise SyntaxError("Invalid print syntax: " + stmt)
            elif stmt.startswith('let '):
                match = re.match(r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*["\'](.*?)["\']', stmt)
                if match:
                    var_name = match.group(1)
                    value = match.group(2)
                    statements.append(LetNode(var_name, value))
                else:
                    raise SyntaxError("Invalid variable declaration: " + stmt)
            else:
                raise SyntaxError("Unknown or invalid command: " + stmt)
        
        return statements

class CodeGenerator:
    def generate(self, ast):
        lines = [
            '#include <iostream>',
            '#include <string>',
            'int main() {'
        ]

        variables = set()

        for node in ast:
            if isinstance(node, LetNode):
                var_name = node.name
                var_value = node.value
                variables.add(var_name)
                lines.append(f'    std::string {var_name} = "{var_value}";')
            elif isinstance(node, PrintNode):
                lines.append(f'    std::cout << "{node.value}" << std::endl;')
            elif isinstance(node, PrintVarNode):
                lines.append(f'    std::cout << {node.var_name} << std::endl;')
        lines.append('    return 0;')
        lines.append('}')
        return '\n'.join(lines)

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
        length = int(self.headers.get('Content-Length'))
        body = self.rfile.read(length)
        data = json.loads(body)

        code = data.get('code', '')
        result = self.interpret_code(code)

        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.end_headers()
        self.wfile.write(result.encode())

        #print("Requête traitée. Arrêt automatique du serveur.")
        #def shutdown_server():
        #    httpd.shutdown()
        #threading.Thread(target=shutdown_server).start()

    def interpret_code(self, code):
        try:
            parser = Parser()
            ast = parser.parse(code)

            generator = CodeGenerator()
            cpp_code = generator.generate(ast)

            with open("temp.cpp", "w") as f:
                f.write(cpp_code)

            subprocess.run(["g++", "temp.cpp", "-o", "temp_exec"], check=True)
            output = subprocess.check_output(["./temp_exec.exe"]).decode()

            os.remove("temp.cpp")
            os.remove("temp_exec.exe")

            return output
        except Exception as e:
            return f"Error: {str(e)}"

if __name__ == '__main__':
    httpd = HTTPServer(('localhost', 5500), SimpleHandler)
    print('Serveur Python prêt sur http://localhost:5500')
    httpd.serve_forever()
