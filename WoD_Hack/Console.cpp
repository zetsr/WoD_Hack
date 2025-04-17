#include "Console.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <thread>

namespace Console {
    static bool isInitialized = false;

    void InitializeConsole()
    {
        // 创建控制台
        AllocConsole();

        // 获取控制台窗口句柄
        HWND consoleWindow = GetConsoleWindow();

        // 设置控制台标题
        SetConsoleTitleA("DLL Console");

        // 重定向 stdout 到控制台
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);

        // 设置控制台输出缓冲区大小
        SetConsoleOutputCP(CP_UTF8);

        // 确保 printf 立即输出
        setvbuf(stdout, NULL, _IONBF, 0);

        isInitialized = true;

        // 输出初始化完成信息
        Printf("Console Initialized\n");
    }

    void InitializeAsync()
    {
        // 在后台线程中初始化控制台，避免阻塞主线程
        std::thread(InitializeConsole).detach();
    }

    void Printf(const char* format, ...)
    {
        if (!isInitialized) {
            // 如果控制台尚未初始化，等待片刻
            Sleep(100);
        }

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    void Cleanup()
    {
        if (isInitialized) {
            FreeConsole();
            isInitialized = false;
        }
    }
}