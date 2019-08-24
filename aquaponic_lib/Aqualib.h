/*
  Aqualib.h - Library for Aquaponic system code.
  Created by Bundit Hou, September 24, 2019.
  Not released into the public domain.
*/


// ifndef = if not define
// Basically, this prevents problems if someone accidently #include's your library twice.
#ifndef Aqualib_h
#define Aqualib_h

// include arduino function
#include "Arduino.h"
#include "WProgram.h"
#include "HardwareSerial.h"

class pHsensor
{
	public:
		pHsensor(int pin, int led);
		/*
			call calulatepH() at loop() before getpH() or getVoltage()
		*/
		void calulatepH(void);
		float getVoltage(void);
		float getpH(void);

	private:
		int pH_pin;
		int LED;

		float Offset = 0.00;            //deviation compensate
		int samplingInterval = 20;
		int printInterval = 800;          // 800 millisec
		int ArrayLenth = 40;            //times of collection
		int pHArray[ArrayLenth];                                      //Store the average value of the sensor feedback
		int pHArrayIndex = 0;
		static float pHValue, voltage;

		double averagearray(int* arr, int number);
};

class o2sensor
{
	public:
		o2sensor(int pin);
		/*
			call calulateO2() at loop() before getpH() or getVoltage()
		*/
		void readDoCharacteristicValues(void);
		void calulateO2(void);

		float getO2(void);
		float getTemperature(void);
		float getVoltage(void);

		void doCalibration(byte mode);

	private:
		int o2_pin;

		int VREF = 5000;    //for arduino uno, the ADC reference is the AVCC, that is 5000mV(TYP)
		float doValue;                                                //current dissolved oxygen value, unit; mg/L
		float temperature = 25;                                       //default temperature is 25^C, you can use a temperature sensor to read it
		
		int ReceivedBufferLength = 20;
		char receivedBuffer[ReceivedBufferLength + 1];        // store the serial command
		byte receivedBufferIndex = 0;
	
		int SCOUNT = 30;          // sum of sample point
		int analogBuffer[SCOUNT];    //store the analog value in the array, readed from ADC
		int analogBufferTemp[SCOUNT];
		int analogBufferIndex = 0, copyIndex = 0;

		float SaturationDoVoltage, SaturationDoTemperature;
		float averageVoltage;

		const float SaturationValueTab[41] PROGMEM = {      //saturation dissolved oxygen concentrations at various temperatures
			14.46, 14.22, 13.82, 13.44, 13.09,
			12.74, 12.42, 12.11, 11.81, 11.53,
			11.26, 11.01, 10.77, 10.53, 10.30,
			10.08, 9.86,  9.66,  9.46,  9.27,
			9.08,  8.90,  8.73,  8.57,  8.41,
			8.25,  8.11,  7.96,  7.82,  7.69,
			7.56,  7.43,  7.30,  7.18,  7.07,
			6.95,  6.84,  6.73,  6.63,  6.53,
			6.41,
		};

		int getMedianNum(int bArray[], int iFilterLen);
		byte uartParse();
		boolean serialDataAvailable(void);
};

class soilMoisturesensor
{
	public:
		soilMoisturesensor(int pin);
		float getSoilMoisture(void);

	private:
		int soil_pin;

};

class ultrasonicsensor
{
	public:
		ultrasonicsensor(int trigPin, int echoPin);
		float getDistance(void);

	private:
		int trig_pin;
		int echo_pin;
};

#endif

