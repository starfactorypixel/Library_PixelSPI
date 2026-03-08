#pragma once
#include <inttypes.h>
#include <SPIManagerInterface.h>
#include <DrakePinD.hpp>

#if defined(ESP32)
	#include "esp_rom_sys.h"
	inline void HAL_Delay(uint32_t delay)
	{
		esp_rom_delay_us((delay * 1024));
	}
#endif

class SPIDeviceInterface
{
	public:
		SPIDeviceInterface(const DrakePin::PinD_t &spi_cs_pin, uint32_t spi_prescaler) : 
			_spi_config{spi_prescaler, 0}, 
			_spi_cs_pin(spi_cs_pin, DrakePin::Output, DrakePin::High)
		{}
		
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
		DrakePinD _spi_cs_pin;
};
