#include "Cheat.h"
#include "Console.h"
#include <thread>
#include "Memory.h"
#include "PointerScanner.h"
#include "Offset.h"

namespace Cheat {
    void CheatLoop()
    {
        while (true)
        {
            static DWORD64 LocalPlayerHealthAddress = 0;

            // ��ȡģ���ַ
            DWORD64 moduleBase = Memory::GetModuleBase("WingsOfDawn-Win64-Shipping.exe");
            if (moduleBase == 0) {
                printf("Failed to get module base address.\n");
                return;
            }

            // �����ַ��ɨ��ָ���ȡ���յ�ַ
            DWORD64 baseAddress = moduleBase + Offset::LocalPlayerHealth::MODULE_OFFSET;
            const auto& offsets = Offset::LocalPlayerHealth::OFFSETS;
            std::vector<std::pair<DWORD64, bool>> debugSteps;
            auto [finalAddress, success] = PointerScanner::Scan(baseAddress, offsets, debugSteps);

            LocalPlayerHealthAddress = finalAddress ? finalAddress : 0;

            if (LocalPlayerHealthAddress) {
                // ʹ�� RPM ��ȡ����ֵ
                float LocalPlayerHealth = 0.0f;
                if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)LocalPlayerHealthAddress, &LocalPlayerHealth, sizeof(float), nullptr)) {
                    printf("Failed to read health.\n");
                    continue;
                }
                printf("Health = %.1f\n", LocalPlayerHealth);

                // ʹ�� WPM �޸Ľ���ֵΪ 10
                float newHealth = 10.0f;
                if (!WriteProcessMemory(GetCurrentProcess(), (LPVOID)LocalPlayerHealthAddress, &newHealth, sizeof(float), nullptr)) {
                    printf("Failed to write health.\n");
                    continue;
                }

                // ʹ�� RPM ��ȡ����ֵ��ƫ�� +0x8��
                float LocalPlayerStamina = 0.0f;
                if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(LocalPlayerHealthAddress + 0x8), &LocalPlayerStamina, sizeof(float), nullptr)) {
                    printf("Failed to read stamina.\n");
                    continue;
                }
                printf("Stamina = %.1f\n", LocalPlayerStamina);

                // ʹ�� WPM �޸�����ֵΪ 10
                float newStamina = 10.0f;
                if (!WriteProcessMemory(GetCurrentProcess(), (LPVOID)(LocalPlayerHealthAddress + 0x8), &newStamina, sizeof(float), nullptr)) {
                    printf("Failed to write stamina.\n");
                    continue;
                }
            }
        }
    }

    void StartCheatThread()
    {
        // Launch cheat loop in a detached thread
        std::thread(CheatLoop).detach();
    }
}