#pragma once
#include <inttypes.h>
#include <SPIManager.h>
#include <DrakePinD.hpp>

/*
	Класс работы с SPI NOR памятью.
	Чип: ZD25WQ80C https://jlcpcb.com/api/file/downloadByFileSystemAccessId/8602999671405367296
*/

class SPI_ZD25WQ80C : public SPIDeviceInterface
{
	static constexpr uint8_t CMD_WRITE_ENABLE =				0x06;
	static constexpr uint8_t CMD_WRITE_DISABLE =			0x04;
	static constexpr uint8_t CMD_WRITE_ENABLE_FOR_VOLATILE_STATUS_REGISTER = 0x50;
	static constexpr uint8_t CMD_READ_STATUS_REGISTER_1 =	0x05;
	static constexpr uint8_t CMD_READ_STATUS_REGISTER_2 =	0x35;
	static constexpr uint8_t CMD_READ_CONFIGURE_REGISTER =	0x15;
	static constexpr uint8_t CMD_WRITE_STATUS_REGISTER_12 =	0x01;
	static constexpr uint8_t CMD_WRITE_STATUS_REGISTER_2 =	0x31;
	static constexpr uint8_t CMD_WRITE_CONFIGURE_REGISTER =	0x11;
	static constexpr uint8_t CMD_READ_DATA_BYTES =			0x03;
	static constexpr uint8_t CMD_READ_DATA_BYTES_AT_HIGHER_SPEED = 0x0B;
	static constexpr uint8_t CMD_PAGE_ERASE =				0x81;
	static constexpr uint8_t CMD_SECTOR_ERASE =				0x20;
	static constexpr uint8_t CMD_HALF_BLOCK_ERASE =			0x52;
	static constexpr uint8_t CMD_BLOCK_ERASE =				0xD8;
	static constexpr uint8_t CMD_CHIP_ERASE_C7 =			0xC7;
	static constexpr uint8_t CMD_CHIP_ERASE_60 =			0x60;
	static constexpr uint8_t CMD_PAGE_PROGRAM =				0x02;
	static constexpr uint8_t CMD_ERASE_SECURITY_REGISTER =	0x44;
	static constexpr uint8_t CMD_PROGRAM_SECURITY_REGISTER =0x42;
	static constexpr uint8_t CMD_READ_SECURITY_REGISTER =	0x48;
	static constexpr uint8_t CMD_DEEP_POWER_DOWN =			0xB9;
	static constexpr uint8_t CMD_RELEASE_FROM_DEEP_POWER_DOWN = 0xAB;
	static constexpr uint8_t CMD_READ_ELECTRONIC_SIGNATURE = 0xAB;
	static constexpr uint8_t CMD_READ_ELECTRONIC_MANUFACTURER_ID = 0x90;
	static constexpr uint8_t CMD_READ_IDENTIFICATION =		0x9F;
	static constexpr uint8_t CMD_PROGRAM_ERASE_SUSPEND =	0x75;
	static constexpr uint8_t CMD_PROGRAM_ERASE_RESUME =		0x7A;
	static constexpr uint8_t CMD_RESET_ENABLE =				0x66;
	static constexpr uint8_t CMD_RESET =					0x99;
	static constexpr uint8_t CMD_READ_UNIQUE_ID =			0x4B;
	static constexpr uint8_t CMD_READ_SFDP_MODE =			0x5A;
	static constexpr uint8_t CMD_CONTINUOUS_READ_MODE_RESET = 0xFF;
	
	public:
		
		static constexpr uint32_t NOR_MAX_ADDRESS = 1048575;
		static constexpr uint32_t NOR_MAX_PAGES = 4095;
		static constexpr uint32_t NOR_MAX_SECTOR = 255;
		static constexpr uint32_t NOR_MAX_BLOCK32 = 31;
		static constexpr uint32_t NOR_MAX_BLOCK64 = 15;

		static constexpr uint32_t NOR_PAGE_SIZE = 256;
		static constexpr uint32_t NOR_SECTOR_SIZE = 4096;
		static constexpr uint32_t NOR_BLOCK32_SIZE = 32768;
		static constexpr uint32_t NOR_BLOCK64_SIZE = 65536;
		static constexpr uint32_t NOR_MEM_SIZE = 1048576;
		
		SPI_ZD25WQ80C(const DrakePin::PinD_t &cs_pin, uint32_t spi_prescaler) : SPIDeviceInterface(cs_pin, spi_prescaler)
		{

		}
		
		virtual void Init() override
		{
			DeviceActivate();
			SendCmd1(CMD_RESET_ENABLE);
			DeviceDeactivate();
			
			DeviceActivate();
			SendCmd1(CMD_RESET);
			DeviceDeactivate();
			
			WaitReady();
			
			return;
		}
		
		virtual void Tick(uint32_t &time) override
		{
			return;
		}
		
		
		/// @brief Прочитать байты
		/// @param address Адррес первого байта
		/// @param length Кол-во читаемых байт
		/// @param data Массив куда положить прочитанные данные
		void ReadBytes(uint32_t address, uint32_t length, uint8_t *data)
		{
			if(address > NOR_MAX_ADDRESS) return;
			if(WaitReady() == false) return;

			DeviceActivate();
			SendCmd4(CMD_READ_DATA_BYTES, address);
			_spi_interface->ReceiveData(data, length);
			DeviceDeactivate();
			
			return;
		}
	
		/// @brief Прочитать страницу, 256 байт
		/// @param page Адрес страницы
		/// @param data Массив куда положить прочитанные данные
		void ReadPage(uint32_t page, uint8_t *data)
		{
			if(page > NOR_MAX_PAGES) return;

			ReadBytes((page * NOR_PAGE_SIZE), NOR_PAGE_SIZE, data);
			
			return;
		}

		
		/// @brief Записать страницу, 256 байт
		/// @param page Адрес страницы
		/// @param data Массив откуда взять записываемые данные
		void WritePage(uint32_t page, uint8_t *data)
		{
			if(page > NOR_MAX_PAGES) return;
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd4(CMD_PAGE_PROGRAM, (page * NOR_PAGE_SIZE));
			_spi_interface->TransmitData(data, NOR_PAGE_SIZE);
			DeviceDeactivate();
			
			return;
		}
		
		
		/// @brief Очистить страницу, 256 байт
		/// @param page Адрес страницы
		void ErasePage(uint32_t page)
		{
			if(page > NOR_MAX_PAGES) return;
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd4(CMD_PAGE_ERASE, page);
			DeviceDeactivate();
			
			return;
		}

		/// @brief Очистить сектор 4K
		/// @param sector Адрес сектора
		void EraseSector(uint32_t sector)
		{
			if(sector > NOR_MAX_SECTOR) return;
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd4(CMD_SECTOR_ERASE, sector);
			DeviceDeactivate();
			
			return;
		}

		/// @brief Очистить блок 32К
		/// @param block Адрес блока
		void EraseBlock32(uint32_t block)
		{
			if(block > NOR_MAX_BLOCK32) return;
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd4(CMD_HALF_BLOCK_ERASE, block);
			DeviceDeactivate();
			
			return;
		}

		/// @brief Очистить блок 64К
		/// @param block Адрес блока
		void EraseBlock64(uint32_t block)
		{
			if(block > NOR_MAX_BLOCK64) return;
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd4(CMD_BLOCK_ERASE, block);
			DeviceDeactivate();
			
			return;
		}

		/// @brief Очистить всю память чипа
		void EraseChip()
		{
			if(WaitReady() == false) return;

			WriteEnable();
			DeviceActivate();
			SendCmd1(CMD_CHIP_ERASE_60);
			DeviceDeactivate();
			
			return;
		}
		
		void WriteEnable()
		{
			DeviceActivate();
			SendCmd1(CMD_WRITE_ENABLE);
			DeviceDeactivate();

			return;
		}
		
		bool WaitReady(uint32_t delay = 100000)
		{
			while(delay--)
			{
				if((ReadStatus1() & 0x01) == 0)
					return true;
			}
			
			return false;
		}
		
		uint8_t ReadStatus1()
		{
			uint8_t status;
			
			DeviceActivate();
			SendCmd1(CMD_READ_STATUS_REGISTER_1);
			_spi_interface->ReceiveData(&status, 1);
			DeviceDeactivate();
			
			return status;
		}
		
		template<uint8_t N> 
		void ReadDevID(uint8_t (&data)[N])
		{
			static_assert(N >= 3, "Buffer too small for DevID");
			
			DeviceActivate();
			SendCmd1(CMD_READ_IDENTIFICATION);
			_spi_interface->ReceiveData(data, 3);
			DeviceDeactivate();
			
			return;
		}
		
		template<uint8_t N> 
		void ReadUniqueID(uint8_t (&data)[N])
		{
			static_assert(N >= 16, "Buffer too small for UniqueID");
			
			DeviceActivate();
			uint8_t tx[5] = {CMD_READ_UNIQUE_ID, 0x00, 0x00, 0x00, 0x00};
			_spi_interface->TransmitData(tx, sizeof(tx));
			_spi_interface->ReceiveData(data, 16);
			DeviceDeactivate();
			
			return;
		}
		
		void SendCmd1(uint8_t cmd)
		{
			uint8_t data[1] = {0x00};
			data[0] = cmd;
			
			_spi_interface->TransmitData(data, sizeof(data));
		}
		
		void SendCmd4(uint8_t cmd, uint32_t address)
		{
			uint8_t data[4] = {0x00};
			data[0] = cmd;
			data[1] = (address >> 16) & 0xFF;
			data[2] = (address >> 8) & 0xFF;
			data[3] = address & 0xFF;
			
			_spi_interface->TransmitData(data, sizeof(data));
		}

	private:

};
