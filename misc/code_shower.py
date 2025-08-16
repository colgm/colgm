import time
import argparse

class Colors:
    LINE = '\033[?25l\033[94;1m'
    OPERATOR = '\033[?25l\033[36;1m'
    FUNCTION_CALL = '\033[?25l\033[36;1m'
    STRING = '\033[?25l\033[4m\033[92m'
    NUMBER = '\033[?25l\033[3m\033[93m'
    IDENTIFIER = '\033[?25l\033[91m'
    KEYWORD = '\033[?25l\033[95;1m'
    SPECIAL_ID = '\033[?25l\033[33;1m'
    COMMENT = '\033[?25l\033[37m'
    END = '\033[0m\033[?25l'

class LineNumberDumper:
    def __init__(self, line_count):
        self.padding = len(str(line_count))
        self.count = 0
    
    def dump(self):
        self.count += 1
        print(Colors.LINE + f"   {self.count:>{self.padding}} | " + Colors.END, end='')

class CodeShower:
    def __init__(self, file_path):
        self.lines = []
        with open(file_path, 'r') as f:
            self.lines = f.readlines()
    
    def dump_single_char(self, ch):
        operators = [
            ',', ';', '.', ':',
            '(', ')', '[', ']', '{', '}',
            '+', '-', '*', '/',
            '=', '>', '<',
            '!', '&', '|'
        ]
        if ch == '\n' or ch == '\t' or ch == '\r' or ch == ' ':
            print(ch, end='', flush=True)
            return
        if ch in operators:
            print(Colors.OPERATOR + ch + Colors.END, end='', flush=True)
            return
        print(ch, end='', flush=True)
        time.sleep(0.01)
    
    def dump_string(self, string: str):
        print(Colors.STRING, end='', flush=True)
        for ch in string:
            print(ch, end='', flush=True)
            time.sleep(0.01)
        print(Colors.END, end='', flush=True)
    
    def dump_number(self, number: int):
        print(Colors.NUMBER, end='', flush=True)
        print(number, end='', flush=True)
        print(Colors.END, end='', flush=True)
    
    def dump_identifier(self, identifier: str):
        keyword = [
            "use", "pub", "impl", "extern",
            "enum", "func", "struct", "union",
            "var", "if", "else", "elsif", "match",
            "while", "for", "foreach", "forindex",
            "return", "true", "false", "nil",
            "defer", "const", "break", "continue"
        ]
        if identifier in keyword:
            print(Colors.KEYWORD, end='', flush=True)
            for ch in identifier:
                print(ch, end='', flush=True)
                time.sleep(0.01)
            print(Colors.END, end='', flush=True)
            return
        
        special_id = [
            "self", "std", "str", "io",
            "i8", "i16", "i32", "i64",
            "u8", "u16", "u32", "u64",
            "f32", "f64",
            "bool", "void"
        ]
        if identifier in special_id:
            print(Colors.SPECIAL_ID, end='', flush=True)
            for ch in identifier:
                print(ch, end='', flush=True)
                time.sleep(0.01)
            print(Colors.END, end='', flush=True)
            return

        print(Colors.IDENTIFIER, end='', flush=True)
        for ch in identifier:
            print(ch, end='', flush=True)
            time.sleep(0.01)
        print(Colors.END, end='', flush=True)
    
    def dump_function_call(self, function_call: str):
        print(Colors.FUNCTION_CALL, end='', flush=True)
        for ch in function_call:
            print(ch, end='', flush=True)
            time.sleep(0.01)
        print(Colors.END, end='', flush=True)
    
    def dump_comment(self, comment: str):
        print(Colors.COMMENT, end='', flush=True)
        for ch in comment:
            print(ch, end='', flush=True)
            time.sleep(0.01)
        print(Colors.END, end='', flush=True)
    
    def tokenize(self, line: str) -> list[tuple[str, str]]:
        res = []

        i = 0
        while i < len(line):
            ch = line[i]
            if ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ch == '_':
                identifier = ""
                while ('a' <= line[i] and line[i] <= 'z') or \
                      ('A' <= line[i] and line[i] <= 'Z') or \
                      ('0' <= line[i] and line[i] <= '9')  or \
                      line[i] == '_':
                    identifier += line[i]
                    i += 1
                if i < len(line) and line[i] == '(':
                    res.append((identifier, "func_call"))
                else:
                    res.append((identifier, "id"))
            elif ch == '\'' or ch == '"':
                string = ch
                i += 1
                while i < len(line) and line[i] != ch:
                    if line[i] == '\\':
                        string += line[i]
                        i += 1
                        string += line[i]
                        i += 1
                        continue
                    string += line[i]
                    i += 1
                if i < len(line) and line[i] == ch:
                    string += ch
                    i += 1
                res.append((string, "str"))
            elif '0' <= ch and ch <= '9':
                number = ch
                i += 1
                while i < len(line) and ('0' <= line[i] and line[i] <= '9' or line[i] in ['.', 'e', 'E', 'o', 'x', '-', '+']):
                    number += line[i]
                    i += 1
                res.append((number, "num"))
            elif ch == '/' and i + 1 < len(line) and line[i + 1] == '/':
                comment = "//"
                i += 2
                while i < len(line) and line[i] != '\n':
                    comment += line[i]
                    i += 1
                res.append((comment, "comment"))
            else:
                res.append((ch, "other"))
                i += 1
        return res
    def show(self):
        print("\033c")
        line_number_dumper = LineNumberDumper(line_count = len(self.lines))
        for line in self.lines:
            line_number_dumper.dump()
            for token in self.tokenize(line):
                if token[1] == "id":
                    self.dump_identifier(token[0])
                elif token[1] == "func_call":
                    self.dump_function_call(token[0])
                elif token[1] == "str":
                    self.dump_string(token[0])
                elif token[1] == "num":
                    self.dump_number(token[0])
                elif token[1] == "comment":
                    self.dump_comment(token[0])
                else:
                    for ch in token[0]:
                        self.dump_single_char(ch)
        if self.lines[-1][-1] != "\n":
            print("")
        line_number_dumper.dump()
        print("")
        line_number_dumper.dump()
        print("")


if __name__ == '__main__':
    try:
        parser = argparse.ArgumentParser()
        parser.add_argument('file', help='file to read')
        args = parser.parse_args()

        code_shower = CodeShower(file_path=args.file)
        code_shower.show()
        input()
        print("\033[?25h")
    except KeyboardInterrupt as e:
        print("\033[?25h")