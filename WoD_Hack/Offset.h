#pragma once
#include <Windows.h>
#include <vector>

// ������ά������ָ��
namespace Offset {
    namespace LocalPlayerHealth {
        static constexpr DWORD64 MODULE_OFFSET = 0x09954AB0;
        static const std::vector<DWORD64> OFFSETS{ 0x30, 0x2E8, 0x230, 0x20, 0xB4 };
    }
}