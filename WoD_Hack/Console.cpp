#include "Console.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <thread>

namespace Console {
    static bool isInitialized = false;

    void InitializeConsole()
    {
        // ��������̨
        AllocConsole();

        // ��ȡ����̨���ھ��
        HWND consoleWindow = GetConsoleWindow();

        // ���ÿ���̨����
        SetConsoleTitleA("DLL Console");

        // �ض��� stdout ������̨
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);

        // ���ÿ���̨�����������С
        SetConsoleOutputCP(CP_UTF8);

        // ȷ�� printf �������
        setvbuf(stdout, NULL, _IONBF, 0);

        isInitialized = true;

        // �����ʼ�������Ϣ
        Printf("Console Initialized\n");
    }

    void InitializeAsync()
    {
        // �ں�̨�߳��г�ʼ������̨�������������߳�
        std::thread(InitializeConsole).detach();
    }

    void Printf(const char* format, ...)
    {
        if (!isInitialized) {
            // �������̨��δ��ʼ�����ȴ�Ƭ��
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