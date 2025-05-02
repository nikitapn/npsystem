#include <iostream>

#include <sim/sim.h>
#include <sim/medium.h>
#include <avr_firmware/net.h>
#include <gtest/gtest.h>

void load_and_execute(std::string_view fw, auto signal_handler) {
  auto ptr = std::make_unique<Microcontroller<Atmega8>>(16'000'000, 1);

  bool stop = false;
  ptr->GetCore()->SetSignalHandler([&stop, &signal_handler](auto sram) {
    stop = true;
    signal_handler(sram);
  });

  ptr->LoadFlash(fw);

  while (stop == false) {
    ptr->GetCore()->Step();
  }
}

TEST(Test, Addition) {
  load_and_execute("add.hex", [](auto sram) {
    EXPECT_EQ(sram[0x60], 30);
  });
}

TEST(Test, Multiplication) {
  load_and_execute("mul.hex", [](auto sram) {
    EXPECT_EQ(sram[0x60], 132);
  });
}

TEST(Test, FloatingPoint) {
  load_and_execute("fp32.hex", [](auto sram) {
    EXPECT_EQ(*(uint32_t*)&sram[0x60], 0x3de978d4ul);
  });
}

TEST(Test, AdditionWithOverflow) {
  load_and_execute("add2.hex", [](auto sram) {
    EXPECT_EQ(*(int16_t*)&sram[0x60], 30 + 240);
  });
}
