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

class MultiOpNode:
    def __init__(self, operands, operator):
        self.operands = operands
        self.operator = operator

class PrintVarNode:
    def __init__(self, var_name):
        self.var_name = var_name

import re

class Parser:
    def parse(self, source):
        self.variables = {}
        code = source.strip()
        statements = []

        code = re.sub(r'//.*', '', code)
        code = code.replace('\n', ' ').replace('\r', '')

        raw_statements = [stmt.strip() for stmt in code.split(';') if stmt.strip()]

        for stmt in raw_statements:
            if stmt.startswith("print(") and stmt.endswith(")"):
                statements.append(self._parse_print(stmt))
            elif stmt.startswith('let '):
                statements.append(self._parse_declaration(stmt, is_const=False))
            elif stmt.startswith('const '):
                statements.append(self._parse_declaration(stmt, is_const=True))
            elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*\s*=', stmt):
                statements.append(self._parse_assignment(stmt))
            else:
                raise SyntaxError("Unknown or invalid command: " + stmt)

        return statements

    def _parse_print(self, stmt):
        str_match = re.match(r'print\(["\'](.*?)["\']\)', stmt)
        var_match = re.match(r'print\(([a-zA-Z_][a-zA-Z0-9_]*)\)', stmt)
        if str_match:
            return PrintNode(str_match.group(1))
        elif var_match:
            return PrintVarNode(var_match.group(1))
        else:
            raise SyntaxError("Invalid print syntax: " + stmt)

    def _parse_declaration(self, stmt, is_const):
        pattern = r'(const|let)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)'
        match = re.match(pattern, stmt)
        if not match:
            raise SyntaxError("Invalid variable declaration: " + stmt)
        _, name, raw_value = match.groups()
        raw_value = raw_value.strip()

        if name in {"int", "float", "string", "main", "true", "false"}:
            raise SyntaxError(f"Reserved keyword used as variable name: {name}")

        var_type, value, is_reference = self._parse_value(raw_value)
        self.variables[name] = var_type

        if is_const:
            return ConstNode(name, var_type, value, is_reference)
        else:
            return LetNode(name, var_type, value, is_reference)

    def _parse_assignment(self, stmt):
        match = re.match(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)$', stmt)
        if not match:
            raise SyntaxError("Invalid assignment syntax: " + stmt)
        name, raw_value = match.groups()
        raw_value = raw_value.strip()

        if name in {"int", "float", "string", "bool", "main", "true", "false"}:
            raise SyntaxError(f"Invalid assignment to reserved name: {name}")

        var_type, value, is_reference = self._parse_value(raw_value)

        if var_type is None:
            if is_reference:
                if name in self.variables:
                    var_type = self.variables[name]
                else:
                    raise Exception(f"Variable '{name}' not declared before assignment")
            elif isinstance(value, MultiOpNode):
                operand_types = []
                for opnd in value.operands:
                    t, _, ref = self._parse_value(opnd)
                    if t is None and ref:
                        if opnd in self.variables:
                            t = self.variables[opnd]
                        else:
                            raise Exception(f"Unknown type for operand '{opnd}' in expression")
                    operand_types.append(t)
                if "float" in operand_types:
                    var_type = "float"
                elif "int" in operand_types:
                    var_type = "int"
                elif all(t == "string" for t in operand_types):
                    var_type = "string"
                else:
                    raise Exception(f"Could not infer type for expression assigned to '{name}'")
            else:
                raise Exception(f"Could not infer type for assignment to '{name}'")

        return AssignNode(name, var_type, value, is_reference)

    def _parse_value(self, raw_value):
        if (raw_value.startswith('"') and raw_value.endswith('"')) or \
           (raw_value.startswith("'") and raw_value.endswith("'")):
            return "string", raw_value[1:-1], False

        if raw_value.lower() == "true" or raw_value.lower() == "false":
            return "bool", raw_value.lower(), False

        if re.match(r'^-?\d+$', raw_value):
            return "int", raw_value, False

        if re.match(r'^-?\d+\.\d*$', raw_value):
            return "float", raw_value, False

        if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', raw_value):
            return None, raw_value, True

        multi_op = re.match(r'^(.+?)(\s*[\+\-\*/]\s*)(.+)$', raw_value)
        if multi_op:
            op = multi_op.group(2)
            parts = [p.strip() for p in raw_value.split(op)]
            return None, MultiOpNode(parts, op), False

        raise SyntaxError("Invalid value in expression: " + raw_value)

class CodeGenerator:
    def generate(self, ast):
        lines = [
            '#include <iostream>',
            '#include <string>',
            '#include <iomanip>',
            'using namespace std::string_literals;',
            'int main() {',
            '    std::cout << std::boolalpha;',
            '    std::cout << std::setprecision(21);'
        ]

        variables = {}
        constantes = {}

        for node in ast:
            if isinstance(node, (LetNode, ConstNode)) and isinstance(node.value, MultiOpNode):
                handle_multi_operation(node, lines, variables, constantes, is_const = isinstance(node, ConstNode))
                continue
            if isinstance(node, LetNode):
                self._declare_variable(node, lines, variables, constantes, is_const=False)

            elif isinstance(node, ConstNode):
                self._declare_variable(node, lines, variables, constantes, is_const=True)

            elif isinstance(node, PrintNode):
                lines.append(f'    std::cout << u8"{node.value}" << std::endl;')

            elif isinstance(node, PrintVarNode):
                lines.append(f'    std::cout << {node.var_name} << std::endl;')

            elif isinstance(node, AssignNode):
                self._assign_variable(node, lines, variables, constantes)

        lines.append('    return 0;')
        lines.append('}')
        return '\n'.join(lines)

    def _declare_variable(self, node, lines, variables, constantes, is_const):
        name = node.name
        var_type = node.type
        value = node.value
        is_ref = getattr(node, 'is_reference', False)
        target_dict = constantes if is_const else variables

        if is_const and name in constantes:
            raise Exception(f"Variable '{name}' already declared")

        if is_ref:
            ref_type = self._get_reference_type(value, variables, constantes)
            target_dict[name] = ref_type
            prefix = 'const ' if is_const else ''
            lines.append(f'    {prefix}{self._cpp_type(ref_type)} {name} = {value};')
        else:
            target_dict[name] = var_type
            cpp_value = self._format_value(var_type, value)
            prefix = 'const ' if is_const else ''
            lines.append(f'    {prefix}{self._cpp_type(var_type)} {name} = {cpp_value};')

    def _assign_variable(self, node, lines, variables, constantes):
        name = node.name
        value = node.value
        var_type = node.type
        is_ref = getattr(node, 'is_reference', False)

        if name in constantes:
            raise Exception(f"Cannot assign to constant '{name}'")

        if name not in variables:
            raise Exception(f"Variable '{name}' is not declared")

        declared_type = variables[name]

        if is_ref:
            ref_type = self._get_reference_type(value, variables, constantes)
            if ref_type != declared_type:
                raise Exception(f"Type mismatch in assignment: {declared_type} != {ref_type}")
            lines.append(f'    {name} = {value};')
        else:
            if isinstance(value, MultiOpNode):
                expr = f" {value.operator} ".join(value.operands)
                lines.append(f'    {name} = {expr};')
            else:
                if declared_type != var_type:
                    raise Exception(f"Type mismatch in assignment to '{name}': {declared_type} != {var_type}")
                cpp_value = self._format_value(var_type, value)
                lines.append(f'    {name} = {cpp_value};')

    def _get_reference_type(self, name, variables, constantes):
        if name in variables:
            return variables[name]
        if name in constantes:
            return constantes[name]
        raise Exception(f"Reference variable '{name}' not declared")

    def _cpp_type(self, var_type):
        return {
            "string": "std::string",
            "int": "int",
            "float": "long double",
            "bool": "bool"
        }.get(var_type, "auto")

    def _format_value(self, var_type, value):
        if var_type == "string":
            return f'"{value}"'
        if var_type == "bool":
            return "true" if value == "true" else "false"
        if var_type == "float":
            return value + 'L'
        return value

class SimpleHandler(BaseHTTPRequestHandler):
    STATIC_ROUTES = {
        "/": ("index.html", "text/html"),
        "/index.html": ("index.html", "text/html"),
        "/main.js": ("main.js", "application/javascript"),
        "/style.css": ("style.css", "text/css"),
    }

    def do_GET(self):
        try:
            route = self.STATIC_ROUTES.get(self.path)
            if route:
                filename, content_type = route
                self.serve_file(filename, content_type)
            else:
                self.respond_error(404, "File not found")
        except Exception as e:
            self.respond_error(500, f"Internal server error: {e}")

    def do_POST(self):
        try:
            length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(length)
            data = json.loads(body)

            code = data.get('code', '')
            result = self.interpret_code(code)

            self.respond_text(200, result)
        except json.JSONDecodeError:
            self.respond_error(400, "Invalid JSON data")
        except Exception as e:
            self.respond_error(500, f"Internal server error: {e}")

    def serve_file(self, filename, content_type):
        try:
            with open(filename, "rb") as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.end_headers()
            self.wfile.write(content)
        except FileNotFoundError:
            self.respond_error(404, f"File not found: {filename}")
        except Exception as e:
            self.respond_error(500, f"Error loading file: {e}")

    def respond_text(self, status_code, text):
        self.send_response(status_code)
        self.send_header('Content-Type', 'text/plain; charset=utf-8')
        self.end_headers()
        self.wfile.write(text.encode('utf-8'))

    def respond_error(self, code, message):
        self.send_error(code, message)

    def interpret_code(self, code):
        try:
            parser = Parser()
            ast = parser.parse(code)

            generator = CodeGenerator()
            cpp_code = generator.generate(ast)

            with open("temp.cpp", "w", encoding="utf-8") as f:
                f.write(cpp_code)

            print("--- Code C++ généré ---")
            print(cpp_code)

            subprocess.run(["g++", "temp.cpp", "-o", "temp_exec"], check=True)
            output = subprocess.check_output(["./temp_exec.exe"]).decode('utf-8')

            os.remove("temp.cpp")
            os.remove("temp_exec.exe")

            return output
        except SyntaxError as se:
            return f"Syntax Error: {str(se)}"
        except subprocess.CalledProcessError as cpe:
            return f"Compilation or Execution Error: {str(cpe)}"
        except Exception as e:
            return f"Unexpected Error: {str(e)}"

def handle_multi_operation(node, lines, variables, constantes, is_const=False):
    operands = node.value.operands
    op = node.value.operator.strip()
    var_name = node.name

    def infer_type(val):
        if val in variables:
            return variables[val]
        elif val in constantes:
            return constantes[val]
        elif re.match(r'^-?\d+$', val):
            return "int"
        elif re.match(r'^-?\d+\.\d*$', val):
            return "float"
        elif (val.startswith('"') and val.endswith('"')) or (val.startswith("'") and val.endswith("'")):
            return "string"
        else:
            raise Exception(f"Unrecognized value or variable: {val}")
    
    operand_types = [infer_type(opnd) for opnd in operands]

    if "string" in operand_types:
        result_type = "string"
        cpp_type = "std::string"
    elif "float" in operand_types:
        result_type = "float"
        cpp_type = "long double"
    else:
        result_type = "int"
        cpp_type = "int"

    expr_parts = []

    for opnd, t in zip(operands, operand_types):
        if result_type == "string":
            if t == "string":
                expr_parts.append(opnd)
            else:
                expr_parts.append(f'std::to_string({opnd})')
        elif op == '/':
            result_type = 'float'
            cpp_type = 'long double'
            expr_parts = [f'static_cast<long double>({opnd})' for opnd in operands]
        else:
            expr_parts.append(opnd)


    expr = f" {op} ".join(expr_parts)
    decl = "const " if is_const else ""
    lines.append(f'    {decl}{cpp_type} {var_name} = {expr};')

    target = constantes if is_const else variables
    target[var_name] = result_type

if __name__ == '__main__':
    httpd = HTTPServer(('localhost', 5500), SimpleHandler)
    print('Serveur Python prêt sur http://localhost:5500')
    httpd.serve_forever()