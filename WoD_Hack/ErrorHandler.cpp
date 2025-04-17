#include "ErrorHandler.h"
#include <windows.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdio>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Psapi.lib")

namespace ErrorHandlerNs {
    static void InitializeDbgHelp() {
        SymInitialize(GetCurrentProcess(), NULL, TRUE);
        printf("[ErrorHandler] DbgHelp ��ʼ����ɡ�\n");
    }

    static std::string GetStackTrace(CONTEXT* context) {
        std::ostringstream oss;
        STACKFRAME64 stackFrame = { 0 };
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

#ifdef _M_X64
        stackFrame.AddrPC.Offset = context->Rip;
        stackFrame.AddrStack.Offset = context->Rsp;
        stackFrame.AddrFrame.Offset = context->Rbp;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrStack.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#else
        stackFrame.AddrPC.Offset = context->Eip;
        stackFrame.AddrStack.Offset = context->Esp;
        stackFrame.AddrFrame.Offset = context->Ebp;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrStack.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        DWORD machineType = IMAGE_FILE_MACHINE_I386;
#endif

        oss << "����ջ��\n";
        for (int i = 0; i < 10; ++i) {
            if (!StackWalk64(machineType, process, thread, &stackFrame, context, NULL,
                SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
                printf("[ErrorHandler] StackWalk64 �� %d �����ʧ�ܡ�\n", i);
                break;
            }

            DWORD64 address = stackFrame.AddrPC.Offset;
            if (address == 0) break;

            IMAGEHLP_MODULE64 moduleInfo = { sizeof(IMAGEHLP_MODULE64) };
            if (SymGetModuleInfo64(process, address, &moduleInfo)) {
                oss << "[" << moduleInfo.ModuleName << "] ";
            }

            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = { 0 };
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;
            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol)) {
                oss << symbol->Name << " + 0x" << std::hex << displacement;
            }
            else {
                oss << "0x" << std::hex << address;
                printf("[ErrorHandler] �޷���ȡ������Ϣ����ַ��0x%llx\n", address);
            }
            oss << "\n";
        }
        return oss.str();
    }

    static std::string GetModuleInfo() {
        std::ostringstream oss;
        HANDLE process = GetCurrentProcess();
        HMODULE modules[1024];
        DWORD cbNeeded;
        if (EnumProcessModules(process, modules, sizeof(modules), &cbNeeded)) {
            oss << "���ص�ģ�飺\n";
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
                char moduleName[MAX_PATH];
                if (GetModuleFileNameA(modules[i], moduleName, MAX_PATH)) {
                    oss << moduleName << "\n";
                }
                else {
                    printf("[ErrorHandler] �޷���ȡģ���ļ������� %lu ��ģ�飬�����룺%lu\n", i, GetLastError());
                }
            }
        }
        else {
            DWORD err = GetLastError();
            oss << "�޷�ö��ģ�飬������룺" << err << "\n";
            printf("[ErrorHandler] EnumProcessModules ʧ�ܣ�������룺%lu\n", err);
        }
        return oss.str();
    }

    static void CopyToClipboard(const std::string& text) {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
            if (hMem) {
                char* pMem = (char*)GlobalLock(hMem);
                if (pMem) {
                    memcpy(pMem, text.c_str(), text.size() + 1);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_TEXT, hMem);
                    printf("[ErrorHandler] �Ѹ�����Ϣ�������塣\n");
                }
                else {
                    GlobalFree(hMem);
                    printf("[ErrorHandler] GlobalLock ʧ�ܣ��޷����Ƶ������塣\n");
                }
            }
            else {
                printf("[ErrorHandler] GlobalAlloc ʧ�ܣ��޷����Ƶ������塣\n");
            }
            CloseClipboard();
        }
        else {
            printf("[ErrorHandler] OpenClipboard ʧ�ܣ������룺%lu\n", GetLastError());
        }
    }

    LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo) {
        std::ostringstream oss;
        oss << "Ӧ�ó��������ش�������ֹ���С�\n"
            << "����Ϊ������Ϣ��\n\n"
            << "�쳣��ϸ��Ϣ��\n"
            << "�쳣���룺0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << "\n"
            << "���ϵ�ַ��0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << "\n"
            << "�쳣��־��0x" << ExceptionInfo->ExceptionRecord->ExceptionFlags << "\n"
            << "����������" << ExceptionInfo->ExceptionRecord->NumberParameters << "\n";

        printf("[ErrorHandler] ����δ�����쳣��0x%08X\n", ExceptionInfo->ExceptionRecord->ExceptionCode);

        if (ExceptionInfo->ExceptionRecord->NumberParameters > 0) {
            oss << "�쳣������\n";
            for (DWORD i = 0; i < ExceptionInfo->ExceptionRecord->NumberParameters; ++i) {
                oss << "���� " << i << "��0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionInformation[i] << "\n";
            }
        }

        oss << "\n�Ĵ���״̬��\n";
#ifdef _M_X64
        oss << "RAX��0x" << std::hex << ExceptionInfo->ContextRecord->Rax << "\n"
            << "RBX��0x" << ExceptionInfo->ContextRecord->Rbx << "\n"
            << "RCX��0x" << ExceptionInfo->ContextRecord->Rcx << "\n"
            << "RDX��0x" << ExceptionInfo->ContextRecord->Rdx << "\n"
            << "RSI��0x" << ExceptionInfo->ContextRecord->Rsi << "\n"
            << "RDI��0x" << ExceptionInfo->ContextRecord->Rdi << "\n"
            << "RBP��0x" << ExceptionInfo->ContextRecord->Rbp << "\n"
            << "RSP��0x" << ExceptionInfo->ContextRecord->Rsp << "\n"
            << "RIP��0x" << ExceptionInfo->ContextRecord->Rip << "\n"
            << "R8��0x" << ExceptionInfo->ContextRecord->R8 << "\n"
            << "R9��0x" << ExceptionInfo->ContextRecord->R9 << "\n"
            << "R10��0x" << ExceptionInfo->ContextRecord->R10 << "\n"
            << "R11��0x" << ExceptionInfo->ContextRecord->R11 << "\n";
#else
        oss << "EAX��0x" << std::hex << ExceptionInfo->ContextRecord->Eax << "\n"
            << "EBX��0x" << ExceptionInfo->ContextRecord->Ebx << "\n"
            << "ECX��0x" << ExceptionInfo->ContextRecord->Ecx << "\n"
            << "EDX��0x" << ExceptionInfo->ContextRecord->Edx << "\n"
            << "ESI��0x" << ExceptionInfo->ContextRecord->Esi << "\n"
            << "EDI��0x" << ExceptionInfo->ContextRecord->Edi << "\n"
            << "EBP��0x" << ExceptionInfo->ContextRecord->Ebp << "\n"
            << "ESP��0x" << ExceptionInfo->ContextRecord->Esp << "\n"
            << "EIP��0x" << ExceptionInfo->ContextRecord->Eip << "\n";
#endif

        oss << "\n" << GetStackTrace(ExceptionInfo->ContextRecord);
        oss << "\n" << GetModuleInfo();

        std::string fullText = oss.str();
        printf("[ErrorHandler] �쳣�����ռ���ϡ�\n");

        std::vector<std::string> lines;
        std::istringstream iss(fullText);
        std::string line;
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }

        const size_t maxLines = 30;
        std::string displayText;
        if (lines.size() <= maxLines) {
            displayText = fullText + "\n\n[������Ϣ�Ѹ��Ƶ������塣]";
        }
        else {
            for (size_t i = 0; i < maxLines - 2; ++i) {
                displayText += lines[i] + "\n";
            }
            displayText += "\n[���ݹ������ѽضϡ�������Ϣ�Ѹ��Ƶ������塣]";
        }

        CopyToClipboard(fullText);

        MessageBoxA(NULL, displayText.c_str(), "Ӧ�ó������", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
        printf("[ErrorHandler] �쳣��Ϣ����ʾ�����ơ�\n");

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void RegisterExceptionFilter() {
        InitializeDbgHelp();
        SetUnhandledExceptionFilter(UnhandledExceptionFilter);
        printf("[ErrorHandler] δ�����쳣��������ע�ᡣ\n");
    }
}
