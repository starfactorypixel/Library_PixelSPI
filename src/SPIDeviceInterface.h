#pragma once
#include <inttypes.h>
#include <SPIManagerInterface.h>
#include <DrakePinD.hpp>

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
