#ifndef CONSOLE_H
#define CONSOLE_H

#include <windows.h>

namespace Console {
    void InitializeAsync();
    void Printf(const char* format, ...);
    void Cleanup();
}

#endif // CONSOLE_H