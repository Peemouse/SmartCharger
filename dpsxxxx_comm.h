/*
dpsxxxx_comm.h
*/

#ifndef DPSXXXX_COMM_h
#define DPSXXXX_COMM_h

#include <Arduino.h>

class DPScomm
{

	typedef struct {
		float setVoltage;
		float setAmp;
		float outVoltage;
		float outAmp;
		float power;
		float inputVoltage;
		bool lock;
		uint16_t protect;
		bool cccvState;
		bool onoff;
		uint16_t brightness;
	}DPSvaluesPacket;

	public:
		/**
		 * @brief      Class constructor
		 */
		DPScomm(void);

		DPSvaluesPacket DPSvalues;

		/**
			* @brief		Initializes the Modbus communication with DPS50xx
		*/
		void DPScommInit(void);

		/**
			* @brief		Read the DPS model through Modbus
			* @return		DPS model (eg. 5015)
		*/
		uint16_t checkDPSmodel(void);

		/**
			* @brief		Read all operating values through Modbus and store them in DPSvalues structure.
			* @return		true if successful
		*/
		bool readDPSOperatingValues(void);

		/**
			* @brief		Read DPS50xx LCD brightness
			* @return		true if successful
		*/
		bool readLCDBrightness(void);

		/**
			* @brief		Set DPS50xx LCD brightness
			* @param		brightness - brightness from 0 to 5 (0 = darkest)
			* @return		true if successful
		*/
		bool setLCDBrightness(uint8_t brigthness);

		/**
			* @brief		Set output voltage (limits are checked regarding parameters declared in HW_limits.h)
			* @param		voltage - set output voltage (in V)
			* @return		true if successful, false means that voltage parameter is out of range
		*/
		bool setOutVoltage(float voltage);

		/**
			* @brief		Set output current (limits are checked regarding parameters declared in HW_limits.h)
			* @param		current - set output current (in Amp)
			* @return		true if successful, false means that current parameter is out of range
		*/
		bool setOutCurrent(float current);

		/**
			* @brief		Lock the DPS50xx keys. A locker should appear on the DPS50xx display. The Modbus commands are still executed.
		*/
		void lockDPS(void);

		/**
			* @brief		Unlock the DPS50xx keys. The locker should disappear on the DPS50xx display.
		*/
		void unlockDPS(void);

		/**
			* @brief		Switch the output ON.
		*/
		void switchOutputON(void);


		/**
			* @brief		Switch the output OFF.
		*/
		void switchOutputOFF(void);

		/**
			* @brief		Get the DPS50xx max power (regarding parameters declared in HW_limits.h)
			* @return		Max output power of the DPS50xx
		*/
		float getMaxPower(void);


};
#endif
