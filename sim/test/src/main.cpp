#include <iostream>

#include <sim/sim.h>
#include <sim/medium.h>
#include <avr_firmware/net.h>

int main()
{
  auto ptr = std::make_unique<Microcontroller<Atmega8>>(16'000'000, 1);

  bool stop = false;

  ptr->GetCore()->SetSignalHandler([&stop](auto sram) {
    std::cout << "Signal handler called\n"
              << "SRAM 0x60: " << (uint32_t)sram[0x60] << std::endl;
    stop = true;
  });

  ptr->LoadFlash("fp.hex");

  while (stop == false) {
    ptr->GetCore()->Step();
  }

  ptr->GetCore()->PrintRegisterFile();
  // ptr->GetCore()->PrintCurrentInstruction();
  // ptr->GetCore()->PrintSRAM();

  return 0;
}
