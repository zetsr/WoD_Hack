#include <windows.h>
#include "Console.h"
#include "Cheat.h"
#include "ErrorHandler.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ErrorHandlerNs::RegisterExceptionFilter();
        // Initialize console asynchronously
        Console::InitializeAsync();
        // Start cheat thread
        Cheat::StartCheatThread();
        break;
    case DLL_THREAD_ATTACH:
        Console::Printf("DLL Thread Attached\n");
        break;
    case DLL_THREAD_DETACH:
        Console::Printf("DLL Thread Detached\n");
        break;
    case DLL_PROCESS_DETACH:
        Console::Printf("DLL Process Detached\n");
        // Cleanup console
        Console::Cleanup();
        break;
    }
    return TRUE;
}