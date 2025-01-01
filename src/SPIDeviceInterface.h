#pragma once
#include <inttypes.h>
#include <SPIManagerInterface.h>
#include <EasyPinD.h>

class SPIDeviceInterface
{
	static constexpr uint32_t MY_SPI_FIRSTBIT_MSB = 0;
	
	public:
		SPIDeviceInterface(EasyPinD::d_pin_t spi_cs_pin, uint32_t spi_prescaler) : 
			_spi_config{spi_prescaler, MY_SPI_FIRSTBIT_MSB}, 
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
