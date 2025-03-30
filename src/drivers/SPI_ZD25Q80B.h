#pragma once
#include <inttypes.h>
#include <SPIManager.h>
#include <EasyPinD.h>

/*
	Класс работы с SPI NOR памятью.
	Чип: ZD25Q80B https://www.lcsc.com/datasheet/lcsc_datasheet_2206131630_Zetta-ZD25Q80BSIGT_C3029777.pdf
*/

class SPI_ZD25Q80B : public SPIDeviceInterface
{
	// Команды чтения (Read Commands)
	static constexpr uint8_t CMD_FAST_READ_ARRAY           = 0x0B;  // Fast Read Array (FREAD)
	static constexpr uint8_t CMD_READ_ARRAY                = 0x03;  // Low Power Read Array (READ)
	static constexpr uint8_t CMD_READ_DUAL_OUTPUT          = 0x3B;  // Dual Output Read (DREAD)
	static constexpr uint8_t CMD_READ_2X_IO                = 0xBB;  // Read 2x I/O (2READ)
	static constexpr uint8_t CMD_READ_QUAD_OUTPUT          = 0x6B;  // Quad Output Read (QREAD)
	static constexpr uint8_t CMD_READ_4X_IO                = 0xEB;  // Read 4x I/O (4READ)

	// Команды программирования и стирания (Program and Erase Commands)
	static constexpr uint8_t CMD_PAGE_ERASE                = 0x81;  // Page Erase (PE)
	static constexpr uint8_t CMD_SECTOR_ERASE              = 0x20;  // Sector Erase 4KB (SE)
	static constexpr uint8_t CMD_BLOCK_ERASE_32K           = 0x52;  // Block Erase 32KB (BE32)
	static constexpr uint8_t CMD_BLOCK_ERASE_64K           = 0xD8;  // Block Erase 64KB (BE64)
	static constexpr uint8_t CMD_CHIP_ERASE_60             = 0x60;  // Chip Erase (CE)
	static constexpr uint8_t CMD_CHIP_ERASE_C7             = 0xC7;  // Chip Erase (CE)

	static constexpr uint8_t CMD_PAGE_PROGRAM              = 0x02;  // Page Program (PP)
	static constexpr uint8_t CMD_DUAL_IN_PAGE_PROGRAM      = 0xA2;  // Dual-IN Page Program (2PP)
	static constexpr uint8_t CMD_QUAD_PAGE_PROGRAM         = 0x32;  // Quad Page Program (QPP)

	static constexpr uint8_t CMD_PROGRAM_ERASE_SUSPEND_75  = 0x75;  // Program/Erase Suspend (PES)
	static constexpr uint8_t CMD_PROGRAM_ERASE_SUSPEND_B0  = 0xB0;  // Program/Erase Suspend (PES)
	static constexpr uint8_t CMD_PROGRAM_ERASE_RESUME_7A   = 0x7A;  // Program/Erase Resume (PER)
	static constexpr uint8_t CMD_PROGRAM_ERASE_RESUME_30   = 0x30;  // Program/Erase Resume (PER)

	// Команды защиты (Protection Commands)
	static constexpr uint8_t CMD_WRITE_ENABLE              = 0x06;  // Write Enable (WREN)
	static constexpr uint8_t CMD_WRITE_DISABLE             = 0x04;  // Write Disable (WRDI)
	static constexpr uint8_t CMD_VOLATILE_SR_WRITE_ENABLE  = 0x50;  // Volatile SR Write Enable (VWREN)

	// Команды безопасности (Security Commands)
	static constexpr uint8_t CMD_ERASE_SECURITY_REGISTERS  = 0x44;  // Erase Security Registers (ERSCUR)
	static constexpr uint8_t CMD_PROGRAM_SECURITY_REGISTERS = 0x42; // Program Security Registers (PRSCUR)
	static constexpr uint8_t CMD_READ_SECURITY_REGISTERS   = 0x48;  // Read Security Registers (RDSCUR)

	// Команды регистра статуса (Status Register Commands)
	static constexpr uint8_t CMD_READ_STATUS_REGISTER      = 0x05;  // Read Status Register (RDSR)
	static constexpr uint8_t CMD_READ_STATUS_REGISTER_2    = 0x35;  // Read Status Register 2 (RDSR2)
	static constexpr uint8_t CMD_READ_CONFIGURE_REGISTER   = 0x15;  // Read Configure Register (RDCR)
	static constexpr uint8_t CMD_ACTIVE_STATUS_INTERRUPT   = 0x25;  // Active Status Interrupt (ASI)

	// Прочие команды (Other Commands)
	static constexpr uint8_t CMD_WRITE_STATUS_REGISTER     = 0x01;  // Write Status Register (WRSR)
	static constexpr uint8_t CMD_WRITE_CONFIGURE_REGISTER  = 0x31;  // Write Configure Register (WRCR)
	static constexpr uint8_t CMD_RESET_ENABLE              = 0x66;  // Reset Enable (RSTEN)
	static constexpr uint8_t CMD_RESET                     = 0x99;  // Reset (RST)

	static constexpr uint8_t CMD_READ_ID                   = 0x9F;  // Read Manufacturer/Device ID (RDID)
	static constexpr uint8_t CMD_READ_MANUFACTURER_ID      = 0x90;  // Read Manufacture ID (REMS)
	static constexpr uint8_t CMD_DUAL_READ_MANUFACTURER_ID = 0x92;  // Dual Read Manufacturer ID (DREMS)
	static constexpr uint8_t CMD_QUAD_READ_MANUFACTURER_ID = 0x94;  // Quad Read Manufacturer ID (QREMS)

	// Команды управления питанием (Power Management Commands)
	static constexpr uint8_t CMD_DEEP_POWER_DOWN           = 0xB9;  // Deep Power-down (DP)
	static constexpr uint8_t CMD_RELEASE_POWER_DOWN        = 0xAB;  // Release Power-down / Read Electronic ID (RDP/RES)

	// Прочие команды (Other)
	static constexpr uint8_t CMD_SET_BURST_LENGTH          = 0x77;  // Set Burst Length (SBL)
	static constexpr uint8_t CMD_READ_SFDP                 = 0x5A;  // Read SFDP (RDSFDP)
	static constexpr uint8_t CMD_RELEASE_READ_ENHANCED     = 0xFF;  // Release from read enhanced
	static constexpr uint8_t CMD_READ_UNIQUE_ID            = 0x4B;  // Read Unique ID (RUID)


	
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
		
		SPI_ZD25Q80B(EasyPinD::d_pin_t cs_pin, uint32_t spi_prescaler) : SPIDeviceInterface(cs_pin, spi_prescaler)
		{

		}
		
		virtual void Init() override
		{
			SendCmd1(CMD_RESET_ENABLE);
			SendCmd1(CMD_RESET);
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
			SendCmd4(CMD_READ_ARRAY, address);
			_spi_interface->ReceiveData(this, data, length);
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
			_spi_interface->TransmitData(this, data, NOR_PAGE_SIZE);
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
			SendCmd4(CMD_BLOCK_ERASE_32K, block);
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
			SendCmd4(CMD_BLOCK_ERASE_64K, block);
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
			SendCmd1(CMD_READ_STATUS_REGISTER);
			_spi_interface->ReceiveData(this, &status, 1);
			DeviceDeactivate();
			
			return status;
		}
		
		void ReadDevID(uint8_t *data)
		{
			DeviceActivate();
			SendCmd1(CMD_READ_ID);
			_spi_interface->ReceiveData(this, data, 3);
			DeviceDeactivate();
			
			return;
		}
		
		void ReadUniqueID(uint8_t *data)
		{
			DeviceActivate();
			SendCmd4(CMD_READ_UNIQUE_ID, 0UL);
			_spi_interface->ReceiveData(this, data, 16);
			DeviceDeactivate();
			
			return;
		}
		
		void SendCmd1(uint8_t cmd)
		{
			uint8_t data[1] = {0x00};
			data[0] = cmd;
			
			_spi_interface->TransmitData(this, data, sizeof(data));
		}
		
		void SendCmd4(uint8_t cmd, uint32_t address)
		{
			uint8_t data[4] = {0x00};
			data[0] = cmd;
			data[1] = (address >> 16) & 0xFF;
			data[2] = (address >> 8) & 0xFF;
			data[3] = address & 0xFF;
			
			_spi_interface->TransmitData(this, data, sizeof(data));
		}

	private:

};
