#pragma once
#include <inttypes.h>
#include <string.h>
#include <functional>
#include "EasyPinD.h"


class SPIBaseDevice;


class SPIManagerInterface
{
	public:
		
		struct spi_config_t
		{
			uint32_t prescaler;
			uint32_t first_bit;
		};
		
		virtual void Config(const spi_config_t &config) = 0;
		virtual void TransmitReceive(SPIBaseDevice *device, uint8_t *tx_data, uint8_t *rx_data, uint16_t length) = 0;
		virtual void TransmitData(SPIBaseDevice *device, uint8_t *data, uint16_t length) = 0;
		virtual void ReceiveData(SPIBaseDevice *device, uint8_t *data, uint16_t length) = 0;
};



class SPIBaseDevice
{
	public:
		SPIBaseDevice(EasyPinD::d_pin_t spi_cs_pin, uint32_t spi_prescaler) : 
			_spi_config{spi_prescaler, SPI_FIRSTBIT_MSB},
			_spi_cs_pin(spi_cs_pin.Port, {spi_cs_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_SET)
		{

		}
		
		virtual void Init() = 0;
		virtual void Tick(uint32_t &time) = 0;
		
		void PrepareInit(SPIManagerInterface *spi)
		{
			_spi_interface = spi;
			_spi_cs_pin.Init();
			
			return;
		}
		
		void DeviceActivate()
		{
			_spi_interface->Config(_spi_config);
			_spi_cs_pin.Off();

			return;
		}
		
		void DeviceDeactivate()
		{
			_spi_cs_pin.On();
			
			return;
		}

	protected:
		SPIManagerInterface* _spi_interface = nullptr;
		SPIManagerInterface::spi_config_t _spi_config = {};
		
	private:
		EasyPinD _spi_cs_pin;
};


/*
	Класс работы с SPI NOR памятью.
	Чип: ZD25Q80B https://www.lcsc.com/datasheet/lcsc_datasheet_2206131630_Zetta-ZD25Q80BSIGT_C3029777.pdf
*/
class SPI_ZD25Q80B : public SPIBaseDevice
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
		
		SPI_ZD25Q80B(EasyPinD::d_pin_t cs_pin, uint32_t spi_prescaler) : SPIBaseDevice(cs_pin, spi_prescaler)
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
			SendCmd4(CMD_PAGE_PROGRAM, page);
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
			while((ReadStatus() & 0x01) == 0x01)
			{
				if(--delay == 0) return false;
			}
			
			return true;
		}
		
		uint16_t ReadStatus()
		{
			uint8_t data[2] = {0x00};
			
			DeviceActivate();
			SendCmd1(CMD_READ_STATUS_REGISTER);
			_spi_interface->ReceiveData(this, &data[0], 1);
			DeviceDeactivate();

			DeviceActivate();
			SendCmd1(CMD_READ_STATUS_REGISTER_2);
			_spi_interface->ReceiveData(this, &data[1], 1);
			DeviceDeactivate();
			
			return (((uint16_t)data[1] << 8) | (uint16_t)data[0]);
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


/*
	Класс работы с SPI EEPROM памятью.
	Чип: CAT25080 https://www.lcsc.com/datasheet/lcsc_datasheet_2210202301_onsemi-CAT25080VI-GT3_C890368.pdf
*/
class SPI_CAT25080 : public SPIBaseDevice
{
	static constexpr uint8_t CMD_WRITE_ENABLE     = 0x06;
	static constexpr uint8_t CMD_WRITE_DISABLE    = 0x04;
	static constexpr uint8_t CMD_READ_STATUS      = 0x05;
	static constexpr uint8_t CMD_WRITE_STATUS     = 0x01;
	static constexpr uint8_t CMD_READ_DATA        = 0x03;
	static constexpr uint8_t CMD_WRITE_DATA       = 0x02;
	
	public:

		static constexpr uint16_t EEPROM_MAX_ADDRESS = 1023;
		static constexpr uint16_t EEPROM_MAX_PAGES = 31;
		static constexpr uint16_t EEPROM_PAGE_SIZE = 32;
		
		SPI_CAT25080(EasyPinD::d_pin_t cs_pin, uint32_t spi_prescaler) : SPIBaseDevice(cs_pin, spi_prescaler)
		{

		}

		virtual void Init() override
		{
			// Сброс состояния eeprom, т.к. нету команды сброса.
			DeviceActivate();
			DeviceDeactivate();
			
			return;
		}
		
		virtual void Tick(uint32_t &time) override
		{
			return;
		}
		
		/// @brief Прочитать байт
		/// @param address Адрес байта
		/// @return Прочитанный байт
		uint8_t ReadByte(uint16_t address)
		{
			if(address > EEPROM_MAX_ADDRESS) return 0xFF;
			if(WaitReady() == false) return 0xFF;
			
			uint8_t result[1];
			
			DeviceActivate();
			SendCmd3(CMD_READ_DATA, address);
			_spi_interface->ReceiveData(this, result, sizeof(result));
			DeviceDeactivate();
			
			return result[0];
		}
		
		/// @brief Прочитать станицу, 32 байта
		/// @param page Номер старницы
		/// @param data Массив для записи
		void ReadPage(uint16_t page, uint8_t *data)
		{
			if(page > EEPROM_MAX_PAGES) return;
			if(WaitReady() == false) return;

			DeviceActivate();
			SendCmd3(CMD_READ_DATA, (page * EEPROM_PAGE_SIZE));
			_spi_interface->ReceiveData(this, data, EEPROM_PAGE_SIZE);
			DeviceDeactivate();
			
			return;
		}
		
		
		/// @brief Записать байт
		/// @param address Адрес байта
		/// @param data Записываемый байт
		void WriteByte(uint16_t address, uint8_t data)
		{
			if(address > EEPROM_MAX_ADDRESS) return;
			if(WaitReady() == false) return;
			
			WriteEnable();
			DeviceActivate();
			SendCmd3(CMD_WRITE_DATA, address);
			_spi_interface->TransmitData(this, (uint8_t *) &data, 1);
			DeviceDeactivate();
			
			return;
		}
		
		/// @brief Записать станицу, 32 байта
		/// @param page Адрес станицы
		/// @param data Массив для записи
		void WritePage(uint16_t page, uint8_t *data)
		{
			if(page > EEPROM_MAX_PAGES) return;
			if(WaitReady() == false) return;
			
			WriteEnable();
			DeviceActivate();
			SendCmd3(CMD_WRITE_DATA, (page * EEPROM_PAGE_SIZE));
			_spi_interface->TransmitData(this, data, EEPROM_PAGE_SIZE);
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
			while((ReadStatus() & 0x01) == 0x01)
			{
				if(--delay == 0) return false;
			}
			
			return true;
		}
		
		uint8_t ReadStatus()
		{
			uint8_t result[1];
			
			DeviceActivate();
			SendCmd1(CMD_READ_STATUS);
			_spi_interface->ReceiveData(this, result, sizeof(result));
			DeviceDeactivate();
			
			return result[0];
		}
		
		void SendCmd1(uint8_t cmd)
		{
			uint8_t data[1];
			data[0] = cmd;
			
			_spi_interface->TransmitData(this, data, sizeof(data));
		}
		
		void SendCmd3(uint8_t cmd, uint16_t address)
		{
			uint8_t data[3];
			data[0] = cmd;
			data[1] = (address >> 8) & 0xFF;
			data[2] = address & 0xFF;
			
			_spi_interface->TransmitData(this, data, sizeof(data));
		}
		
	private:
	
};


template <uint8_t _dev_count> 
class SPI_HC595 : public SPIBaseDevice
{
	public:
		SPI_HC595(EasyPinD::d_pin_t cs_pin, EasyPinD::d_pin_t latch_pin, uint32_t spi_prescaler) : 
			SPIBaseDevice(cs_pin, spi_prescaler), 
			_latch_pin(latch_pin.Port, {latch_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_SET)
		{

		}
		
		virtual void Init() override
		{
			_latch_pin.Init();
			memset(_data, 0x00, sizeof(_data));
			_SPI_Run();
			
			return;
		}
		
		virtual void Tick(uint32_t &time) override
		{
			return;
		}
		
		void SetState(uint8_t idx, bool state)
		{
			return SetState((idx / 8), (idx % 8), state);
		}
		
		void SetState(uint8_t device, uint8_t pin, bool state)
		{
			if(device >= _dev_count) return;
			if(pin >= 8) return;
			
			_data[device] = (_data[device] & ~(1 << pin)) | (state << pin);
			_SPI_Run();
			
			return;
		}
		
		bool GetState(uint8_t idx)
		{
			return GetState((idx / 8), (idx % 8));
		}
		
		bool GetState(uint8_t device, uint8_t pin)
		{
			if(device >= _dev_count) return false;
			if(pin >= 8) return false;
			
			return (_data[device] >> pin) & 0b00000001;
		}
		
	private:
		
		void _SPI_Run()
		{
			_latch_pin.Off();
			asm("nop\n nop\n");
			_latch_pin.On();
			
			DeviceActivate();
			_spi_interface->TransmitData(this, _data, sizeof(_data));
			DeviceDeactivate();

			return;
		}
		
		EasyPinD _latch_pin;
		uint8_t _data[_dev_count];
};



template <uint8_t _dev_count> 
class SPI_HC165 : public SPIBaseDevice
{
	using func_change_t = void (*)(uint8_t device, uint8_t pin, bool state);
	
	public:
		SPI_HC165(EasyPinD::d_pin_t cs_pin, EasyPinD::d_pin_t latch_pin, uint32_t spi_prescaler) : 
			SPIBaseDevice(cs_pin, spi_prescaler), 
			_latch_pin(latch_pin.Port, {latch_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_SET)
		{
			_spi_config.first_bit = SPI_FIRSTBIT_LSB;

			return;
		}
		
		virtual void Init() override
		{
			_latch_pin.Init();
			
			memset(_data_new, 0x00, sizeof(_data_new));
			memset(_data_old, 0x00, sizeof(_data_old));
			
			_SPI_Run();
			for(uint8_t i = 0; i < _dev_count; ++i)
			{
				for(uint8_t j = 0; j < 8 ; ++j)
				{
					bool n = (_data_new[i] >> j) & 0x01;
					//bool o = (_data_old[i] >> j) & 0x01;
					
					_CallbackRun(i, j, n);
				}
			}
			
			return;
		}
		
		virtual void Tick(uint32_t &time) override
		{
			if(time - _last_tick > 25)
			{
				_last_tick = time;
				
				Read();
			}
			
			return;
		}
		
		void SetCallback(func_change_t callback)
		{
			_callback = callback;
			
			return;
		}
		
		void Read()
		{
			_SPI_Run();
			
			if(_callback == nullptr) return;
			
			for(uint8_t i = 0; i < _dev_count; ++i)
			{
				if(_data_new[i] == _data_old[i]) continue;
				
				for(uint8_t j = 0; j < 8 ; ++j)
				{
					bool n = (_data_new[i] >> j) & 0x01;
					bool o = (_data_old[i] >> j) & 0x01;
					if(n == o) continue;
					
					_CallbackRun(i, j, n);
				}
			}
			
			return;
		}
		
		bool GetState(uint8_t idx)
		{
			return GetState((idx / 8), (idx % 8));
		}
		
		bool GetState(uint8_t device, uint8_t pin)
		{
			if(device >= _dev_count) return false;
			if(pin >= 8) return false;
			
			return (_data_new[device] >> pin) & 0b00000001;
		}
		
	private:

		inline void _CallbackRun(uint8_t device, uint8_t pin, bool state)
		{
			if(_callback)
			{
				_callback(device, pin, !state);
			}
			
			return;
		}
		
		void _SPI_Run()
		{
			_latch_pin.Off();
			memcpy(_data_old, _data_new, sizeof(_data_old));
			_latch_pin.On();

			DeviceActivate();
			_spi_interface->ReceiveData(this, _data_new, sizeof(_data_new));
			DeviceDeactivate();
			
			return;
		}
		
		EasyPinD _latch_pin;
		uint8_t _data_new[_dev_count];
		uint8_t _data_old[_dev_count];
		uint32_t _last_tick = 0;
		func_change_t _callback = nullptr;
};




template <uint8_t _device_max> 
class SPIManager : public SPIManagerInterface
{
	using callback_config_t = void (*)(const spi_config_t &config);
	using callback_tx_t = void (*)(uint8_t *data, uint16_t length);
	using callback_rx_t = void (*)(uint8_t *data, uint16_t length);
	using callback_txrx_t = void (*)(uint8_t *tx_data, uint8_t *rx_data, uint16_t length);
	
	public:
		SPIManager(callback_config_t cfg, callback_tx_t tx, callback_rx_t rx, callback_txrx_t txrx) : _callback_config(cfg), _callback_tx(tx), _callback_rx(rx), _callback_txrx(txrx)
		{

		}
		
		void AddDevice(SPIBaseDevice &device)
		{
			if(devices_count >= _device_max) return;
			
			devices[devices_count++] = &device;
			device.PrepareInit(this);
			device.Init();
			
			return;
		}
		
		void Tick(uint32_t &time)
		{
			for(uint8_t i = 0; i < devices_count; ++i)
			{
				devices[i]->Tick(time);
			}
			
			return;
		}
		
		void DiselectAll()
		{
			for(SPIBaseDevice &device : devices)
			{
				device.DeviceDeactivate();
			}
			
			return;
		}
		
		virtual void Config(const spi_config_t &config) override
		{
			_callback_config(config);
		}
		
		virtual void TransmitData(SPIBaseDevice *device, uint8_t *data, uint16_t length) override
		{
			_callback_tx(data, length);
		}
		
		virtual void ReceiveData(SPIBaseDevice *device, uint8_t *data, uint16_t length) override
		{
			_callback_rx(data, length);
		}
		
		virtual void TransmitReceive(SPIBaseDevice *device, uint8_t *tx_data, uint8_t *rx_data, uint16_t length) override
		{
			_callback_txrx(tx_data, rx_data, length);
		}
		
	private:
		
		SPIBaseDevice *devices[_device_max];
		uint8_t devices_count = 0;
		
		callback_config_t _callback_config;
		callback_tx_t _callback_tx;
		callback_rx_t _callback_rx;
		callback_txrx_t _callback_txrx;
};
