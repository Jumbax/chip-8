#define CLOVE_SUITE_NAME EmulatorTestSuite
#include "clove-unit.h"
#include "chip-8.h"
#include <array>

CLOVE_TEST(Opcode0_CLS)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    int* ExpectedTexture = reinterpret_cast<int*>(SDL_calloc(emulator.GetHeight() * emulator.GetWidth(), sizeof(int)));
    int* Texture = nullptr;
    int Pitch;
    SDL_LockTexture(emulator.GetTexture(), nullptr, reinterpret_cast<void**>(&Texture), &Pitch);
    CLOVE_INT_EQ(0, SDL_memcmp(ExpectedTexture, Texture, emulator.GetHeight() * emulator.GetWidth() * sizeof(int)));
    SDL_UnlockTexture(emulator.GetTexture());
    status = emulator.Opcode0(0xE0);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode0_RET)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint8_t SP = emulator.GetSP();
    status = emulator.Opcode0(0xEE);
    const uint16_t PC = emulator.GetPC();
    const std::array<uint16_t, 0x10> Stack = emulator.GetStack();
    CLOVE_INT_EQ(PC, Stack[SP & 0xF]);
    CLOVE_INT_EQ(emulator.GetSP(), SP - 1);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode1_JUMP_addr)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    status = emulator.Opcode1(0x1000);
    const uint16_t PC = emulator.GetPC();
    const uint16_t address = (0x1000 & 0xFFF) - 2;
    CLOVE_INT_EQ(PC, address);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode2_CALL_addr)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint16_t PC = emulator.GetPC();
    status = emulator.Opcode2(0x2000);
    const uint8_t SP = emulator.GetSP();
    const std::array<uint16_t, 0x10> Stack = emulator.GetStack();
    const uint16_t address = 0x2000 & 0xFFF;
    CLOVE_INT_EQ(Stack[SP], PC);
    PC = emulator.GetPC();
    CLOVE_INT_EQ(PC, address);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::NotIncrementPC), static_cast<int>(status));
    for (size_t i = 0; i < 16; i++)
    {
       status = emulator.Opcode2(0x2000);
    }
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::StackOverflow), static_cast<int>(status));
}

CLOVE_TEST(Opcode3_SE_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;
    
    uint16_t PC = emulator.GetPC();
    status = emulator.Opcode3(0x3000);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
    status = emulator.Opcode3(0x3001);
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
}

CLOVE_TEST(Opcode4_SNE_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint16_t PC = emulator.GetPC();
    status = emulator.Opcode4(0x4000);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
    CLOVE_INT_EQ(emulator.GetPC(), PC);
    status = emulator.Opcode4(0x4001);
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
}

CLOVE_TEST(Opcode5_SE_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint16_t PC = emulator.GetPC();
    uint16_t opcode = 0x5000;
    emulator.Opcode5(opcode);
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
    opcode = 0x5101;
    emulator.Opcode6(0x6001);
    status = emulator.Opcode5(opcode);
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode6_LD_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x6520;
    status = emulator.Opcode6(opcode);
    uint8_t register_index = (opcode >> 8) & 0xF;
    uint8_t register_value = opcode & 0xFF;
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
    CLOVE_INT_EQ(emulator.GetRegisters()[register_index], register_value);
}

CLOVE_TEST(Opcode7_ADD_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x7000;
    uint8_t register_index = (opcode >> 8) & 0xF;
    uint8_t value = opcode & 0xFF;
    uint8_t x = emulator.GetRegisters()[register_index];
    status = emulator.Opcode7(opcode);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
    CLOVE_INT_EQ(emulator.GetRegisters()[register_index], x + value);
}

CLOVE_TEST(Opcode8_LD_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8000;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], emulator.GetRegisters()[registerY_index]);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_OR_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8001;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], x | y)
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_AND_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8002;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index],x & y)
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_XOR_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8003;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], x ^ y)
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_ADD_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8124;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    
    emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 0);
    
    emulator.Opcode6(0x61FF);
    emulator.Opcode6(0x62FF);
    
    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    uint8_t result = x + y;
    
    status = emulator.Opcode8(opcode);
    
    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 1);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], result);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_SUB_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8125;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;

    emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 0);

    emulator.Opcode6(0x61FF);
    emulator.Opcode6(0x62F0);

    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    uint8_t result = x - y;

    status = emulator.Opcode8(opcode);

    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 1);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], result);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_SHR_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8006;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    uint8_t i = emulator.GetRegisters()[registerX_index];
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], i / 2);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_SUBN_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x8127;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;

    emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 0);

    emulator.Opcode6(0x61F0);
    emulator.Opcode6(0x62FF);

    uint8_t x = emulator.GetRegisters()[registerX_index];
    uint8_t y = emulator.GetRegisters()[registerY_index];
    uint8_t result = y - x;

    status = emulator.Opcode8(opcode);

    CLOVE_INT_EQ(emulator.GetRegisters()[0xF], 1);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerY_index], result);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode8_SHL_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0x800E;
    uint8_t registerX_index = (opcode >> 8) & 0xF;
    uint8_t registerY_index = (opcode >> 4) & 0xF;
    uint8_t i = emulator.GetRegisters()[registerX_index];
    status = emulator.Opcode8(opcode);
    CLOVE_INT_EQ(emulator.GetRegisters()[registerX_index], i * 2);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(Opcode9_SNE_Vx_Vy)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint16_t opcode = 0x9000;
    uint16_t PC = emulator.GetPC();
    emulator.Opcode9(opcode);
    CLOVE_INT_EQ(emulator.GetPC(), PC);
    emulator.Opcode6(0x6001);
    opcode = 0x9101;
    status = emulator.Opcode9(opcode);
    CLOVE_INT_EQ(emulator.GetPC(), PC + 2);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeA_LD_I, addr)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint16_t value = (0xA000 & 0xFFF);
    status = emulator.OpcodeA(0xA000);
    CLOVE_INT_EQ(emulator.GetI(), value);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeB_JP_V0_addr)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0xB000;
    const uint16_t address = ((opcode & 0x0FFF) - 2) + emulator.GetRegisters()[0];
    status = emulator.OpcodeB(0xB000);
    CLOVE_INT_EQ(emulator.GetPC(), address);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeC_RND_Vx_byte)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint16_t opcode = 0xC000;
    uint8_t register_index = (opcode >> 8) & 0xF;
    uint8_t random_mask = opcode & 0xFF;
    status = emulator.OpcodeC(0xC000);
    CLOVE_INT_EQ(emulator.GetRegisters()[register_index], (std::rand() % 256) & random_mask);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeD_DRW_Vx_Vy_nibble)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    status = emulator.OpcodeD(0xD000);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeE_SKP_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;
    status = emulator.OpcodeE(0x9E);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeE_SKNP_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;
    status = emulator.OpcodeE(0xA1);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_Vx_DT)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x07 >> 8) & 0xF;
    uint8_t i = emulator.GetDelayTimer();
    
    status = emulator.OpcodeF(0x07);
    CLOVE_INT_EQ(emulator.GetRegisters()[register_index], i);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_Vx_K)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    const uint8_t register_index = (0x0A >> 8) & 0xF;
    status = emulator.OpcodeF(0x0A);
    CLOVE_IS_TRUE(emulator.GetSuspended());
    CLOVE_INT_EQ(register_index, emulator.GetWaitForKeyboardRegister_Index());
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::WaitForKeyboard), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_DT_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x15 >> 8) & 0xF;
    status = emulator.OpcodeF(0x15);
    CLOVE_INT_EQ(emulator.GetDelayTimer(), emulator.GetRegisters()[register_index]);
    CLOVE_INT_EQ(emulator.GetDeltaTimerTicks(), 17 + SDL_GetTicks64());
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_ST_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    status = emulator.OpcodeF(0x18);
    uint8_t register_index = (0x18 >> 8) & 0xF;
    CLOVE_INT_EQ(emulator.GetSoundTimer(), emulator.GetRegisters()[register_index]);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_ADD_I_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x1E >> 8) & 0xF;
    uint16_t oldI = emulator.GetI();
    status = emulator.OpcodeF(0x1E);
    CLOVE_INT_EQ(emulator.GetI(), oldI + emulator.GetRegisters()[register_index]);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_F_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x29 >> 8) & 0xF;
    uint16_t oldI = emulator.GetI();
    status = emulator.OpcodeF(0x29);
    CLOVE_INT_EQ(emulator.GetI(), oldI * emulator.GetRegisters()[register_index]);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_B_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x33 >> 8) & 0xF;
    uint8_t value = emulator.GetRegisters()[register_index];
    status = emulator.OpcodeF(0x33);
    uint8_t oldvalue_0 = value / 100;
    uint8_t oldvalue_1 = value - (emulator.GetMemoryMapping()[emulator.GetI()] * 100) / 10;
    uint8_t oldvalue_2 = value % 10;
    CLOVE_INT_EQ(emulator.GetMemoryMapping()[emulator.GetI()], oldvalue_0);
    CLOVE_INT_EQ(emulator.GetMemoryMapping()[emulator.GetI() + 1], oldvalue_1);
    CLOVE_INT_EQ(emulator.GetMemoryMapping()[emulator.GetI() + 2], oldvalue_2);
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_I_Vx)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;

    uint8_t register_index = (0x55 >> 8) & 0xF;
    status = emulator.OpcodeF(0x55);
    for (uint8_t i = 0; i < register_index; ++i)
    {
        CLOVE_INT_EQ(emulator.GetMemoryMapping()[emulator.GetI() + i], emulator.GetRegisters()[i]);
    }
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}

CLOVE_TEST(OpcodeF_LD_Vx_I)
{
    chipotto::Emulator emulator;
    chipotto::OpcodeStatus status;
 
    uint8_t register_index = (0x65 >> 8) & 0xF;
    status = emulator.OpcodeF(0x65);
    for (uint8_t i = 0; i < register_index; ++i)
    {
        CLOVE_INT_EQ(emulator.GetRegisters()[i], emulator.GetMemoryMapping()[emulator.GetI() + i]);
    }
    CLOVE_INT_EQ(static_cast<int>(chipotto::OpcodeStatus::IncrementPC), static_cast<int>(status));
}