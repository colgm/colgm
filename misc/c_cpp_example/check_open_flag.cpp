#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "sizeof mode_t: " << sizeof(mode_t) << std::endl;
    std::cout << "O_RDONLY: 0x" << std::hex << O_RDONLY << std::dec << std::endl;
    std::cout << "O_WRONLY: 0x" << std::hex << O_WRONLY << std::dec << std::endl;
    std::cout << "O_RDWR: 0x" << std::hex << O_RDWR << std::dec << std::endl;
    std::cout << "O_CREAT: 0x" << std::hex << O_CREAT << std::dec << std::endl;
    std::cout << "O_EXCL: 0x" << std::hex << O_EXCL << std::dec << std::endl;
    std::cout << "O_TRUNC: 0x" << std::hex << O_TRUNC << std::dec << std::endl;
    std::cout << "O_APPEND: 0x" << std::hex << O_APPEND << std::dec << std::endl;
#ifdef _WIN32
    std::cout << "O_BINARY: 0x" << std::hex << O_BINARY << std::dec << std::endl;
    std::cout << "O_TEXT: 0x" << std::hex << O_TEXT << std::dec << std::endl;
#endif
    return 0;
}