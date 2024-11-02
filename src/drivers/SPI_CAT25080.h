#pragma once
#include <inttypes.h>
#include <SPIManager.h>
#include <EasyPinD.h>

/*
	Класс работы с SPI EEPROM памятью.
	Чип: CAT25080 https://www.lcsc.com/datasheet/lcsc_datasheet_2210202301_onsemi-CAT25080VI-GT3_C890368.pdf
*/

class SPI_CAT25080 : public SPIDeviceInterface
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
		
		SPI_CAT25080(EasyPinD::d_pin_t cs_pin, uint32_t spi_prescaler) : SPIDeviceInterface(cs_pin, spi_prescaler)
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
		
		/// @brief Прочитать данные в объект
		/// @tparam T Любой объект для побайтового чтения
		/// @param address Адрес первого байта
		/// @param data Объект для записи данных
		/// @return true в случае успеха
		template<typename T> 
		bool ReadRaw(uint16_t address, T &data)
		{
			if(address + sizeof(T) > EEPROM_MAX_ADDRESS) return false;
			if(WaitReady() == false) return false;
			
			DeviceActivate();
			SendCmd3(CMD_READ_DATA, address);
			_spi_interface->ReceiveData(this, (uint8_t *) &data, sizeof(T));
			DeviceDeactivate();
			
			return true;
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
		
		/// @brief Записать данные из объекта
		/// @tparam T Любой объект для побайтового чтения, должен быть кратен EEPROM_MAX_ADDRESS
		/// @param address Адрес первого байта, должен быть кратен EEPROM_MAX_ADDRESS
		/// @param data Объект для записи данных
		/// @return true в случае успеха
		template<typename T> 
		bool WriteRaw(uint16_t address, const T &data)
		{
			static_assert(sizeof(T) % EEPROM_PAGE_SIZE == 0, "Size of T must be a multiple of 32 bytes.");
			
			if(address % EEPROM_PAGE_SIZE != 0 || address + sizeof(T) > EEPROM_MAX_ADDRESS) return false;
			if(WaitReady() == false) return false;
			
			const uint8_t *dataPtr = (const uint8_t *) &data;
			uint8_t pageCount = sizeof(T) / EEPROM_PAGE_SIZE;
			
			for(uint8_t i = 0; i < pageCount; ++i)
			{
				WritePage(address / EEPROM_PAGE_SIZE, (uint8_t *) dataPtr);
				
				address += EEPROM_PAGE_SIZE;
				dataPtr += EEPROM_PAGE_SIZE;
			}
			
			return true;
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
