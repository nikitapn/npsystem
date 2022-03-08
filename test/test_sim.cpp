#include "stdafx.h"
#include <sim/sim.h>
#include <sim/medium.h>
#include <atomic>
#include <array>
#include <avr_firmware/net.h>

using namespace std;

std::atomic<bool> run;
std::atomic<bool> running;
HANDLE hConsole;
HWND hWnd;
CONSOLE_SCREEN_BUFFER_INFO info;
bool runtime_simulation_failed = false;
int runtime_simulation_failed_cnt = 0;

void handle_input();
void print_time();

void handle_input() {
	for (;;) {
		// test for real time
		std::this_thread::sleep_for(2s);
		run = false;
		break;
		int c = _getch();
		if (c == '0') {
			run = false;
			break;
		}
	}
}

void print_time() {
	while (run.load(std::memory_order_relaxed)) {
		//	system("cls");
		//	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
		//	std::cout << "Time:" << std::setw(15) << ctrl->GetTime() << " s\n";
		//	if (runtime_simulation_failed) std::cout << "runtime_simulation_failed";
		//	std::cout.flush();
		std::this_thread::sleep_for(10ms);
	}
	running = false;
}

auto get_frequency(int64_t seconds = 1) {
	volatile int i = 0;
	for (; i < 100000; ++i);
	auto a = __rdtsc();
	this_thread::sleep_for(std::chrono::seconds(seconds));
	auto b = __rdtsc();
	return static_cast<uint64_t>((b - a) / seconds);
}


constexpr auto Count = 4;
std::array<std::unique_ptr<Microcontroller<Atmega8>>, Count> controllers;
uint64_t cpu_frequency;


void run_multicore() {
	auto func = [](Microcontroller<Atmega8>* ctrl) {
		auto cpu_frequency_d = (double)cpu_frequency;
		while (run.load(std::memory_order_relaxed)) {
			auto ca = __rdtsc();
			auto duration = ctrl->ExecuteCore();
			duration = duration - 0.0000000118;
			auto cc_next = (uint64_t)(ca + (uint64_t)(duration * cpu_frequency_d));
			if (cc_next < __rdtsc()) {
				runtime_simulation_failed_cnt++;
				continue;
			}
			while (__rdtsc() < cc_next);
		}
	};

	runtime_simulation_failed_cnt = 0;

	std::array<std::thread, controllers.size()> threads;
	for (size_t i = 0; i < controllers.size(); ++i) {
		threads[i] = std::move(std::thread(func, controllers[i].get()));
	}

	for (auto& i : threads) i.join();


	std::cout << "runtime_simulation_failed_cnt: " << runtime_simulation_failed_cnt << std::endl;
	system("pause");
	system("cls");
}

int test_sim() {
	cpu_frequency = get_frequency();
	Medium medium;
	try {
		for (int i = 0, addr = 2; i < Count; i++, addr++) {
			auto ptr = std::make_unique<Microcontroller<Atmega8>>(16'000'000, addr);
			if (addr == 2) {
				ptr->LoadFlash("d:\\projects\\cpp\\Alpha\\x64\\firmware\\pc-link-virtual.hex");
			} else {
				ptr->LoadFlash(
					"d:\\projects\\cpp\\Alpha\\x64\\firmware\\atmega8_virtual_"
					+ std::to_string(addr) + ".hex");
			}
			medium.AddController(ptr.get());
			controllers[i] = std::move(ptr);
		}
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		return -1;
	}
	std::cout << "loaded" << std::endl;
	//	controllers[0] = std::make_unique<Microcontroller<Atmega8>>(16'000'000, 3);
	//	controllers[0]->LoadFlash("d:\\projects\\cpp\\Alpha\\x64\\firmware\\atmega8_3.hex");
	//	medium.AddPeripheral(controllers[0].get());

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	hWnd = GetConsoleWindow();

	GetConsoleScreenBufferInfo(hConsole, &info);
	MoveWindow(hWnd, 60, 0, 1030, 1440, TRUE);


	try {
		while (1) {
			SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
			//		std::cout << "Time: " << controllers[0]->GetTime() << " s\n";
			//		std::cout << "Time: " << controllers[1]->GetTime() << " s\n";
			//		std::cout << "Time: " << controllers[2]->GetTime() << " s\n";


			for (auto& c : controllers) {
				//	c->GetCore()->PrintRegisterFile();
				//	c->GetCore()->PrintCurrentInstruction();
				//	ctrl->GetCore()->PrintSRAM();
				//	ctrl->GetCore()->PrintCurrentInstruction();
				//	ctrl->PrintIOSpace();
			}
			//		controllers[1]->GetCore()->PrintSRAM();


			int c = _getch();
			switch (c)
			{
			case 's':
				//	save();
				break;
			case 'l':
				//	load();
				break;
			case 0x20:
				for (auto& c : controllers) {
					c->ExecuteCore();
				}
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				for (int i = 0; i < (int)pow(10, (c - '0')); ++i) {
					for (auto& c : controllers) {
						c->ExecuteCore();
					}
				}
				break;
			case '9':
				run = true;
				running = true;
				std::thread(handle_input).detach();
				std::thread(print_time).detach();
				run_multicore();
				break;
			case '0':
			{
				run = true;
				running = true;
				std::thread(handle_input).detach();
				std::thread(print_time).detach();




				std::cout << "runtime_simulation_failed_cnt: " << runtime_simulation_failed_cnt << std::endl;
				system("pause");
				system("cls");
				break;
			}
			default:
				break;
			}

		}
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		for (auto& c : controllers) {
			c->GetCore()->PrintLastInstructions();
		}
	}

	CloseHandle(hConsole);

	return 0;
}