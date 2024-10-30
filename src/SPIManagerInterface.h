#pragma once
#include <inttypes.h>

class SPIDeviceInterface;

class SPIManagerInterface
{
	public:
		
		struct spi_config_t
		{
			uint32_t prescaler;
			uint32_t first_bit;
		};
		
		virtual void Config(const spi_config_t &config) = 0;
		virtual void TransmitReceive(SPIDeviceInterface *device, uint8_t *tx_data, uint8_t *rx_data, uint16_t length) = 0;
		virtual void TransmitData(SPIDeviceInterface *device, uint8_t *data, uint16_t length) = 0;
		virtual void ReceiveData(SPIDeviceInterface *device, uint8_t *data, uint16_t length) = 0;
};
