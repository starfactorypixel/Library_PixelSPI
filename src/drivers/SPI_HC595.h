#pragma once
#include <inttypes.h>
#include <string.h>
#include <SPIManager.h>
#include <EasyPinD.h>

/*

*/

template <uint8_t _dev_count> 
class SPI_HC595 : public SPIDeviceInterface
{
	public:
		SPI_HC595(EasyPinD::d_pin_t cs_pin, EasyPinD::d_pin_t latch_pin, EasyPinD::d_pin_t oe_pin, uint32_t spi_prescaler) : 
			SPIDeviceInterface(cs_pin, spi_prescaler), 
			_latch_pin(latch_pin.Port, {latch_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_RESET), 
			_oe_pin(oe_pin.Port, {oe_pin.Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH}, GPIO_PIN_SET)
		{

		}
		
		virtual void Init() override
		{
			_latch_pin.Init();
			_oe_pin.Init();
			memset(_data, 0x00, sizeof(_data));
			_SPI_Run();
			
			return;
		}
		
		virtual void Tick(uint32_t &time) override
		{
			return;
		}
		
		void OutputEnable()
		{
			_oe_pin.Off();
			
			return;
		}
		
		void OutputDisable()
		{
			_oe_pin.On();
			
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
		
		void WriteByte(uint8_t device, const uint8_t byte)
		{
			if(device >= _dev_count) return;
			
			_data[device] = byte;
			_SPI_Run();
			
			return;
		}
		
		void WriteByMask(uint8_t device, const uint8_t byte, const uint8_t mask)
		{
			if(device >= _dev_count) return;
			
			_data[device] = (_data[device] & ~mask) | (byte & mask);
			_SPI_Run();
			
			return;
		}
		
	private:
		
		void _SPI_Run()
		{
			DeviceActivate();
			_spi_interface->TransmitData(this, _data, sizeof(_data));
			DeviceDeactivate();
			
			_latch_pin.On();
			asm("nop\n nop\n");
			_latch_pin.Off();
			
			return;
		}
		
		EasyPinD _latch_pin;
		EasyPinD _oe_pin;
		uint8_t _data[_dev_count];
};
