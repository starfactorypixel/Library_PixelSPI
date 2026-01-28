// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// https://github.com/sandeepmistry/arduino-CAN
// Porting for StarPixel project: Dragon_Knight, https://github.com/Dragon-Knight

#pragma once
#include <inttypes.h>
#include <string.h>
#include <SPIManager.h>
#include <EasyPinD.h>

class SPI_MCP2515 : public SPIDeviceInterface
{
	static constexpr uint32_t NO_CAN_ID = 0xFFFFFFFF;
	
	using func_rx_t = void (*)(uint32_t address, uint8_t *data, uint8_t length);
	
	struct packet_t
	{
		bool flag = false;			// Некий флаг для работы, для tx это инициализация пакета, для rx это готовность пакета
		bool extended = false;		// Флаг exID
		bool rtr = false;			// Флаг RTR
		uint32_t id = NO_CAN_ID;	// Идентификатор пакета CAN
		uint8_t dlc = 0;			// DLC ??
		uint8_t length = 0;			// Длина пакета CAN
		uint8_t data[8] = {};		// Данные пакета CAN
	};
	
	public:
		
		SPI_MCP2515(EasyPinD::d_pin_t cs_pin, EasyPinD::d_pin_t int_pin, uint32_t spi_prescaler) : 
			SPIDeviceInterface(cs_pin, spi_prescaler), 
			_int_pin(int_pin.Port, {int_pin.Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}), 
			_rx{}, _tx{},
			_onReceive(nullptr),
			_last_tick(0)
		{}
		
		bool begin(uint32_t clock_frequency, uint32_t baud_rate, func_rx_t callback);
		void end();
		
		virtual void Init() override;
		virtual void Tick(uint32_t &time) override;
		
		bool beginPacket(uint16_t id, bool rtr = false);
		bool beginExtendedPacket(uint32_t id, bool rtr = false);
		uint8_t write(uint8_t byte){ return write(&byte, 1); }
		uint8_t write(const uint8_t *buffer, uint8_t size);
		bool endPacket();
		
		uint8_t parsePacket();
		
		bool filter(uint16_t id) { return filter(id, 0x7ff); }
		bool filter(uint16_t id, uint16_t mask);
		bool filterExtended(uint32_t id) { return filterExtended(id, 0x1fffffff); }
		bool filterExtended(uint32_t id, uint32_t mask);
		
		bool cmd_reset();
		bool cmd_observe();
		bool cmd_loopback();
		bool cmd_sleep();
		bool cmd_wakeup();
		
	private:
		
		uint8_t readRegister(uint8_t address);
		void modifyRegister(uint8_t address, uint8_t mask, uint8_t value);
		void writeRegister(uint8_t address, uint8_t value);
		bool writeReadRegister(uint8_t address, uint8_t value);
		
		EasyPinD _int_pin;
		packet_t _rx;
		packet_t _tx;
		func_rx_t _onReceive;
		uint32_t _last_tick;
		
		
		struct cnf_f
		{
			uint32_t clockFrequency;
			uint32_t baudRate;
			uint8_t cnf[3];
		};
		static const cnf_f _cnf_map[26];
};
