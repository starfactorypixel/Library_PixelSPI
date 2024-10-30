#pragma once
#include <inttypes.h>
#include <SPIManager.h>
#include <EasyPinD.h>

/*

*/

template <uint8_t _dev_count> 
class SPI_HC165 : public SPIDeviceInterface
{
	using func_change_t = void (*)(uint8_t device, uint8_t pin, bool state);
	
	public:
		SPI_HC165(EasyPinD::d_pin_t cs_pin, EasyPinD::d_pin_t latch_pin, uint32_t spi_prescaler) : 
			SPIDeviceInterface(cs_pin, spi_prescaler), 
			_latch_pin(latch_pin.Port, {latch_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_SET)
		{
			//_spi_config.first_bit = SPI_FIRSTBIT_LSB;

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
