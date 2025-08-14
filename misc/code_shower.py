import time
import argparse

class Colors:
    BLUE = '\033[94;1m'
    CYAN = '\033[36;1m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    KEYWORD = '\033[95;1m'
    SPECIAL_ID = '\033[33;1m'
    END = '\033[0m'

class LineNumberDumper:
    def __init__(self, line_count):
        self.padding = len(str(line_count))
        self.count = 0
    
    def dump(self):
        self.count += 1
        print(Colors.BLUE + f" {self.count:>{self.padding}} | " + Colors.END, end='')

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
            print(Colors.CYAN + ch + Colors.END, end='', flush=True)
            time.sleep(0.01)
            return
        print(ch, end='', flush=True)
        time.sleep(0.03)
    
    def dump_string(self, string: str):
        print(Colors.GREEN, end='', flush=True)
        for ch in string:
            print(ch, end='', flush=True)
            time.sleep(0.03)
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
                time.sleep(0.03)
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
                time.sleep(0.03)
            print(Colors.END, end='', flush=True)
            return

        print(Colors.RED, end='', flush=True)
        for ch in identifier:
            print(ch, end='', flush=True)
            time.sleep(0.03)
        print(Colors.END, end='', flush=True)
    
    def tokenize(self, line: str) -> list[tuple[str, str]]:
        res = []
        other = ""

        i = 0
        while i < len(line):
            ch = line[i]
            if ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ch == '_':
                if len(other) != 0:
                    res.append((other, "other"))
                    other = ""
                identifier = ""
                while ('a' <= line[i] and line[i] <= 'z') or \
                      ('A' <= line[i] and line[i] <= 'Z') or \
                      ('0' <= line[i] and line[i] <= '9')  or \
                      line[i] == '_':
                    identifier += line[i]
                    i += 1
                res.append((identifier, "id"))
            elif ch == '\'' or ch == '"':
                if len(other) != 0:
                    res.append((other, "other"))
                    other = ""
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
            else:
                other += ch
                i += 1
        if len(other) != 0:
            res.append((other, "other"))
        return res
    def show(self):
        line_number_dumper = LineNumberDumper(line_count = len(self.lines))
        for line in self.lines:
            line_number_dumper.dump()
            for token in self.tokenize(line):
                if token[1] == "id":
                    self.dump_identifier(token[0])
                    continue
                if token[1] == "str":
                    self.dump_string(token[0])
                    continue
                for ch in token[0]:
                    self.dump_single_char(ch)
        print()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', help='file to read')
    args = parser.parse_args()

    code_shower = CodeShower(file_path=args.file)
    code_shower.show()