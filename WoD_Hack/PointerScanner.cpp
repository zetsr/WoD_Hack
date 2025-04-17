#include "PointerScanner.h"
#include "Console.h"
#include <Windows.h>
#include <stdio.h>

namespace PointerScanner {

    // ��ȫ�жϵ�ַ�Ƿ�ɶ������ⴥ���쳣��
    bool IsReadableAddress(void* address) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0)
            return false;

        DWORD protect = mbi.Protect;

        return (mbi.State == MEM_COMMIT) &&
            !(protect & PAGE_GUARD) &&
            !(protect & PAGE_NOACCESS) &&
            (
                (protect & PAGE_READONLY) ||
                (protect & PAGE_READWRITE) ||
                (protect & PAGE_EXECUTE_READ) ||
                (protect & PAGE_EXECUTE_READWRITE)
                );
    }

    std::pair<DWORD64, bool> Scan(DWORD64 baseAddress, const std::vector<DWORD64>& offsets, std::vector<std::pair<DWORD64, bool>>& debugSteps) {
        debugSteps.clear();
        DWORD64 currentAddress = baseAddress;
        debugSteps.push_back({ currentAddress, false }); // ��ʼ��ַ

        if (offsets.empty()) {
            return { currentAddress, true }; // ��ƫ�ƣ�ֱ�ӷ���
        }

        // ��һ���������û�ַ
        if (!IsReadableAddress((void*)currentAddress)) {
            printf("Base address not readable: 0x%llX\n", currentAddress);
            return { 0, false };
        }

        currentAddress = *(volatile DWORD64*)currentAddress;
        debugSteps.push_back({ currentAddress, true });

        if (!currentAddress) {
            printf("Base dereference returned null at address: 0x%llX\n", currentAddress);
            return { 0, false };
        }

        for (size_t i = 0; i < offsets.size(); ++i) {
            currentAddress += offsets[i];
            debugSteps.push_back({ currentAddress, false }); // ��ƫ��

            // �����һ��ʱ�����н�����
            if (i < offsets.size() - 1) {
                if (!IsReadableAddress((void*)currentAddress)) {
                    printf("Address unreadable at step %zu, offset 0x%llX: 0x%llX\n", i + 1, offsets[i], currentAddress);
                    return { 0, false };
                }

                currentAddress = *(volatile DWORD64*)currentAddress;
                debugSteps.push_back({ currentAddress, true });

                if (!currentAddress) {
                    printf("Null encountered at step %zu, offset 0x%llX, address: 0x%llX\n", i + 1, offsets[i], currentAddress);
                    return { 0, false };
                }
            }
        }

        return { currentAddress, true };
    }
}
