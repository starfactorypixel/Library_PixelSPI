#pragma once
#include <inttypes.h>
#include "SPIManagerInterface.h"
#include "SPIDeviceInterface.h"

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
			memset(devices, 0x00, sizeof(devices));
		}
		
		void AddDevice(SPIDeviceInterface &device)
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
			for(SPIDeviceInterface &device : devices)
			{
				device.DeviceDeactivate();
			}
			
			return;
		}
		
		virtual void Config(const spi_config_t &config) override
		{
			_callback_config(config);
		}
		
		virtual void TransmitData(SPIDeviceInterface *device, uint8_t *data, uint16_t length) override
		{
			_callback_tx(data, length);
		}
		
		virtual void ReceiveData(SPIDeviceInterface *device, uint8_t *data, uint16_t length) override
		{
			_callback_rx(data, length);
		}
		
		virtual void TransmitReceive(SPIDeviceInterface *device, uint8_t *tx_data, uint8_t *rx_data, uint16_t length) override
		{
			_callback_txrx(tx_data, rx_data, length);
		}
		
	private:
		
		SPIDeviceInterface *devices[_device_max];
		uint8_t devices_count = 0;
		
		callback_config_t _callback_config;
		callback_tx_t _callback_tx;
		callback_rx_t _callback_rx;
		callback_txrx_t _callback_txrx;
};
