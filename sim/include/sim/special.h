#pragma once

#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <optional>
#include <atomic>

class MediumState {
public:
	static constexpr int medium_state_low = 0;
	static constexpr int medium_state_high = 1;
	static constexpr int medium_state_zero = 2;

	// 00 -> 0
	// 01 -> rising_edge
	// 11 -> 0
	// 10 -> falling_edge
	
	static constexpr int logic_one = 1; 
	static constexpr int logic_zero = 0;

	static constexpr int rising_edge = 1;
	static constexpr int falling_edge = 2;

	static constexpr int get_edge[] = { 0, rising_edge, 0 , falling_edge };


	static constexpr uint64_t ONE_BIT_DURATION_19200 = 52083;
//	static constexpr uint64_t ONE_BIT_DURATION_19200 = 20000;

	void uart_put_byte(uint8_t byte, uint8_t dev_addr) noexcept {
		bool value = false;
		if (uart_byte_sending_.compare_exchange_strong(value, true)) {
			uart_byte_to_send_ = byte;
		} else {
			auto prev = uart_byte_to_send_;
			uart_byte_to_send_ |= byte;
			std::cout << "cd pr: " << std::hex << int(prev)
				<< " new: " << (int)byte << " will be: " << (int)uart_byte_to_send_ 
				<< std::dec << std::endl;
		}
	}
	bool is_tx_complete() const noexcept {
		return tx_complete_;
	}
	int get_medium_state() const noexcept {
		return medium_state_;
	}
	int get_int0_edge() const noexcept {
		return int_0_edge;
	}
	int get_int0_state() const noexcept {
		return int_0_state;
	}
	int is_int_0_changed() const noexcept {
		return int_0_changed;
	}
	std::optional<uint8_t> uart_get_sended_byte() const noexcept {
		return uart_byte_sended_ ? uart_byte_to_send_ : std::optional<uint8_t>();
	}
	void run(uint64_t duration) {
		static int state = 0;
		static int uart_data_length;
		static uint64_t time_point;
		int_0_edge = 0;
		int_0_changed = 0;
		switch (state)
		{
		case 0: // free medium
			if (uart_byte_sending_) {
				state = 1;
				medium_state_ = medium_state_low; // start bit
				int_0_state = logic_zero;
				int_0_edge = falling_edge;
				int_0_changed = 1;
				uart_data_length = 8;
				time_point = 0;
			}
			break;
		case 1: // sending byte
			time_point += duration;
			if (time_point > ONE_BIT_DURATION_19200) {
				time_point = 0;
				if (uart_data_length == -1) {
					medium_state_ = medium_state_zero;
					state = 3;
					break;
				}
				uart_data_length--;
				if (uart_data_length == 0) {
					medium_state_ = medium_state_low; // stop bit
				} else {
					medium_state_ ^= 1; // don't actually
				}
			//	int_0_edge = get_edge[(int_0_state << 1) | medium_state_];
			//	int_0_changed = (int_0_state != medium_state_);
			//	int_0_state = medium_state_;
			}
			break;
		case 3: // last bit sended
			uart_byte_sended_ = true;
			uart_byte_sending_ = false;
			state = 4;
			break;
		case 4:
			tx_complete_ = true;
			uart_byte_sended_ = false;
			state = 5;
			break;
		case 5:
			tx_complete_ = false;
			int_0_state = logic_one;
			int_0_edge = rising_edge;
			int_0_changed = 1;
			state = 0;
			break;
		default:
			assert(false);
			break;
		}
	}
private:
	uint8_t uart_byte_to_send_;
	std::atomic_bool uart_byte_sending_ = false;
	bool uart_byte_sended_ = false;
	bool tx_complete_ = false;

	int medium_state_ = medium_state_zero;
	int int_0_state = medium_state_high;
	int int_0_edge = 0;
	int int_0_changed = 0;


	
};
