#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")

void printStackTraceSimple() {
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    PVOID stack[100];
    USHORT frames = CaptureStackBackTrace(0, 100, stack, NULL);

    // 获取符号信息
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames; i++) {
        DWORD64 displacement = 0;
        if (SymFromAddr(process, (DWORD64)(stack[i]), &displacement, symbol)) {
            printf("[%d] %s\n", frames - i - 1, symbol->Name);
        } else {
            printf("[%d] 0x%p\n", frames - i - 1, stack[i]);
        }
    }

    free(symbol);
    SymCleanup(process);
}

int main() {
    printStackTraceSimple();
    return 0;
}