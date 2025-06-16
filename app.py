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
    def __init__(self, name, var_type, value, is_reference = False):
        self.name = name
        self.type = var_type
        self.value = value
        self.is_reference = is_reference

class ConstNode:
    def __init__(self, name, const_type, value, is_reference = False):
        self.name = name
        self.type = const_type
        self.value = value
        self.is_reference = is_reference

class AssignNode:
    def __init__(self, name, value_type, value, is_reference = False):
        self.name = name
        self.type = value_type
        self.value = value
        self.is_reference = is_reference

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
                
            elif stmt.startswith('let ') or stmt.startswith('const '):
                match_var = re.match(r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)', stmt)
                match_const = re.match(r'const\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)', stmt)
                if match_var:
                    var_name = match_var.group(1)
                    raw_value = match_var.group(2).strip()
                    if (raw_value.startswith('"') and raw_value.endswith('"')) or \
                       (raw_value.startswith("'") and raw_value.endswith("'")):
                            var_type = "string"
                            value = raw_value[1:-1]
                            is_reference = False
                    elif raw_value.lower() == "true" or raw_value.lower() == "false":
                        var_type = "bool"
                        value = raw_value.lower()
                        is_reference = False
                    elif re.match(r'^-?\d+$', raw_value):
                        var_type = "int"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^-?\d+\.\d*$', raw_value):
                        var_type = "float"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', raw_value):
                        var_type = None
                        value = raw_value
                        is_reference = True
                    else:
                        raise SyntaxError("Invalid value in variable declaration: " + stmt)
                    
                    statements.append(LetNode(var_name, var_type, value, is_reference))
                elif match_const:
                    const_name = match_const.group(1)
                    raw_value = match_const.group(2).strip()
                    if (raw_value.startswith('"') and raw_value.endswith('"')) or \
                       (raw_value.startswith("'") and raw_value.endswith("'")):
                            const_type = "string"
                            value = raw_value[1:-1]
                            is_reference = False
                    elif raw_value.lower() == "true" or raw_value.lower() == "false":
                        const_type = "bool"
                        value = raw_value.lower()
                        is_reference = False
                    elif re.match(r'^-?\d+$', raw_value):
                        const_type = "int"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^-?\d+\.\d*$', raw_value):
                        const_type = "float"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', raw_value):
                        const_type = None
                        value = raw_value
                        is_reference = True
                    else:
                        raise SyntaxError("Invalid value in variable declaration: " + stmt)
                    
                    statements.append(ConstNode(const_name, const_type, value, is_reference))
                else:
                    raise SyntaxError("Invalid variable declaration: " + stmt)
                
            elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*\s*=', stmt):
                match = re.match(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)$', stmt)
                if match:
                    var_name = match.group(1)
                    raw_value = match.group(2).strip()
                    if(raw_value.startswith('"') and raw_value.endswith('"')) or \
                      (raw_value.startswith("'") and raw_value.endswith("'")):
                        var_type = "string"
                        value = raw_value[1:-1]
                        is_reference = False
                    elif raw_value.lower() == 'true' or raw_value.lower() == 'false':
                        var_type = 'bool'
                        value = raw_value.lower()
                        is_reference = False
                    elif re.match(r'^-?\d+$', raw_value):
                        var_type = "int"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^-?\d+\.\d*$', raw_value):
                        var_type = "float"
                        value = raw_value
                        is_reference = False
                    elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', raw_value):
                        var_type = None
                        value = raw_value
                        is_reference = True
                    else:
                        raise SyntaxError("Invalid value in assignment: " + stmt)
                    statements.append(AssignNode(var_name, var_type, value, is_reference))
                else:
                    raise SyntaxError("Invalid assignment syntax: " + stmt)
                
            else:
                raise SyntaxError("Unknown or invalid command: " + stmt)
        
        return statements

class CodeGenerator:
    def generate(self, ast):
        lines = [
            '#include <iostream>',
            '#include <string>',
            'int main() {',
            '    std::cout << std::boolalpha;'
        ]

        variables = {}
        constante = {}

        for node in ast:
            if isinstance(node, LetNode):
                var_name = node.name
                var_type = node.type
                var_value = node.value
                is_ref = getattr(node, 'is_reference', False)

                if is_ref:
                    if var_value in variables:
                        ref_type = variables[var_value]
                        variables[var_name] = ref_type
                        if ref_type == "string":
                            lines.append(f'    std::string {var_name} = {var_value};')
                        elif ref_type == "int":
                            lines.append(f'    int {var_name} = {var_value};')
                        elif ref_type == "float":
                            lines.append(f'    float {var_name} = {var_value};')
                        elif ref_type == "bool":
                            lines.append(f'    bool {var_name} = {var_value};')
                    elif var_value in constante:
                        ref_type = constante[var_value]
                        variables[var_name] = ref_type
                        if ref_type == "string":
                            lines.append(f'    std::string {var_name} = {var_value};')
                        elif ref_type == "int":
                            lines.append(f'    int {var_name} = {var_value};')
                        elif ref_type == "float":
                            lines.append(f'    float {var_name} = {var_value};')
                        elif ref_type == "bool":
                            lines.append(f'    bool {var_name} = {var_value};')
                    else:
                        raise Exception(f"Reference variable '{var_value}' not declared")
                else:
                    variables[var_name] = var_type
                    if var_type == "string":
                        lines.append(f'    std::string {var_name} = "{var_value}";')
                    elif var_type == "int":
                        lines.append(f'    int {var_name} = {var_value};')
                    elif var_type == "float":
                        lines.append(f'    float {var_name} = {var_value};')
                    elif var_type == "bool":
                        bool_val = "true" if var_value == "true" else "false"
                        lines.append(f'    bool {var_name} = {bool_val};')
            if isinstance(node, ConstNode):
                const_name = node.name
                const_type = node.type
                const_value = node.value
                is_ref = getattr(node, 'is_reference', False)

                if const_name in constante:
                    raise Exception(f"Variable '{const_name}' already declared")
                
                if is_ref:
                    if const_value in variables:
                        ref_type = variables[const_value]
                        constante[const_name] = ref_type
                        if ref_type == "string":
                            lines.append(f'    const std::string {const_name} = {const_value};')
                        elif ref_type == "int":
                            lines.append(f'    const int {const_name} = {const_value};')
                        elif ref_type == "float":
                            lines.append(f'    const float {const_name} = {const_value};')
                        elif ref_type == "bool":
                            lines.append(f'    const bool {const_name} = {const_value};')
                    elif const_value in constante:
                        ref_type = constante[const_value]
                        constante[const_name] = ref_type
                        if ref_type == "string":
                            lines.append(f'    const std::string {const_name} = {const_value};')
                        elif ref_type == "int":
                            lines.append(f'    const int {const_name} = {const_value};')
                        elif ref_type == "float":
                            lines.append(f'    const float {const_name} = {const_value};')
                        elif ref_type == "bool":
                            lines.append(f'    const bool {const_name} = {const_value};')
                    else:
                        raise Exception(f"Reference variable '{const_value}' not declared")
                else:
                    constante[const_name] = const_type
                    if const_type == "string":
                        lines.append(f'    const std::string {const_name} = "{const_value}";')
                    elif const_type == "int":
                        lines.append(f'    const int {const_name} = {const_value};')
                    elif const_type == "float":
                        lines.append(f'    const float {const_name} = {const_value};')
                    elif const_type == "bool":
                        bool_val = "true" if const_value == "true" else "false"
                        lines.append(f'    const bool {const_name} = {bool_val};')
            elif isinstance(node, PrintNode):
                lines.append(f'    std::cout << u8"{node.value}" << std::endl;')
            elif isinstance(node, PrintVarNode):
                lines.append(f'    std::cout << {node.var_name} << std::endl;')
            elif isinstance(node, AssignNode):
                var_name = node.name
                var_type = node.type
                var_value = node.value
                is_ref = getattr(node, 'is_reference', False)

                if var_name in constante:
                    raise Exception(f"Invalid assignment: '{var_name}' is a constant and cannot be reassigned (current value: {var_value})")
                
                if var_name not in variables:
                    raise Exception(f"Variable '{var_name}' is not declared")

                declared_type = variables[var_name]

                if is_ref:
                    if var_value in variables:
                        rhs_type = variables[var_value]
                        if rhs_type != declared_type:
                            raise Exception(f"Type mismatch in assignment: {declared_type} != {rhs_type}")
                        lines.append(f'    {var_name} = {var_value};')
                    elif var_value in constante:
                        rhs_type = constante[var_value]
                        if rhs_type != declared_type:
                            raise Exception(f"Type mismatch in assignment: {declared_type} != {rhs_type}")
                        lines.append(f'    {var_name} = {var_value};')
                    else:
                        raise Exception(f"Reference variable '{var_value}' not declared")
                else:
                    if declared_type != var_type:
                        raise Exception(f"Type mismatch in assignment to '{var_name}': {declared_type} != {var_type}")

                    if var_type == "string":
                        lines.append(f'    {var_name} = "{var_value}";')
                    elif var_type == "int":
                        lines.append(f'    {var_name} = {var_value};')
                    elif var_type == "float":
                        lines.append(f'    {var_name} = {var_value};')
                    elif var_type == "bool":
                        bool_val = "true" if var_value == "true" else "false"
                        lines.append(f'    {var_name} = {bool_val};')
        lines.append('    return 0;')
        lines.append('}')
        return '\n'.join(lines)

class SimpleHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            if self.path == "/" or self.path == "/index.html":
                self.serve_file("index.html", "text/html")
            elif self.path == "/main.js":
                self.serve_file("main.js", "application/javascript")
            elif self.path == "/style.css":
                self.serve_file("style.css", "text/css")
            else:
                self.send_error(404, "File not found")
        except Exception as e:
            self.send_error(500, f"Internal server error : {e}")

    def serve_file(self, filename, content_type):
        try:
            with open(filename, "rb") as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.end_headers()
            self.wfile.write(content)
        except FileNotFoundError:
            self.send_error(404, f'File not found : {filename}')
        except Exception as e:
            self.send_error(500, f"Error loading file : {e}")

    def do_POST(self):
        try:
            length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(length)
            data = json.loads(body)

            code = data.get('code', '')
            result = self.interpret_code(code)

            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(result.encode())

            #print("Request processed. Automatic server shutdown.")
            #def shutdown_server():
            #    httpd.shutdown()
            #threading.Thread(target=shutdown_server).start()
        except json.JSONDecodeError:
            self.send_error(400, "Invalid JSON data")
        except Exception as e:
            self.send_error(500, f"Internal server error : {e}")

    def interpret_code(self, code):
        try:
            parser = Parser()
            ast = parser.parse(code)

            generator = CodeGenerator()
            cpp_code = generator.generate(ast)

            with open("temp.cpp", "w", encoding = "utf-8") as f:
                f.write(cpp_code)

            subprocess.run(["g++", "temp.cpp", "-o", "temp_exec"], check=True)
            output = subprocess.check_output(["./temp_exec.exe"]).decode('utf-8')

            os.remove("temp.cpp")
            os.remove("temp_exec.exe")

            return output
        except SyntaxError as se:
            return f"Syntaxe Error : {str(se)}"
        except subprocess.CalledProcessError as cpe:
            return f"Compilation or Execution Error : {str(cpe)}"
        except Exception as e:
            return f"Unexpected Error : {str(e)}"

if __name__ == '__main__':
    httpd = HTTPServer(('localhost', 5500), SimpleHandler)
    print('Serveur Python prêt sur http://localhost:5500')
    httpd.serve_forever()