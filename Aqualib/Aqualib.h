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
#include "HardwareSerial.h"

// temperature
#include <OneWire.h>
#include <DallasTemperature.h>
//OneWire oneWire;
class temperaturesensor
{
public:
	temperaturesensor(OneWire* onewire);
	float getTemperature(void);
	float temperatureValue = 25; // default = 25^C

private:
	int temppin;
	bool check_null = false;
	//static OneWire oneWire = OneWire(13);&oneWire
	DallasTemperature* tempsensors;
};

class pHsensor
{
	public:
		pHsensor(int pin, int led);
		/*
			call calulatepH() at loop() before getpH() or getVoltage()
		*/
		void calculatepH(void);
		float getVoltage(void);
		float getpH(void);

	private:
		int pH_pin;
		int LED;

		float Offset = 0.00;            //deviation compensate
		int samplingInterval = 20;
		int printInterval = 800;          // 800 millisec
		const static int ArrayLenth = 40;            //times of collection
		int pHArray[ArrayLenth];                                      //Store the average value of the sensor feedback
		int pHArrayIndex = 0;
		float pHValue;
		float voltage;

		double avergearray(int* arr, int number);
};

//saturation dissolved oxygen concentrations at various temperatures
const PROGMEM float SaturationValueTab[] = {
	14.46, 14.22, 13.82, 13.44, 13.09,
	12.74, 12.42, 12.11, 11.81, 11.53,
	11.26, 11.01, 10.77, 10.53, 10.30,
	10.08, 9.86,  9.66,  9.46,  9.27,
	9.08,  8.90,  8.73,  8.57,  8.41,
	7.56,  7.43,  7.30,  7.18,  7.07,
	6.95,  6.84,  6.73,  6.63,  6.53,
	6.41,
};

class o2sensor
{


	public:
		//o2sensor(int pin);
		o2sensor(int pin, OneWire* onewire);
		/*
			call calulateO2() at loop() before getpH() or getVoltage()
		*/
		void readDoCharacteristicValues(void);
		void calculateO2(void);

		float getO2(void);
		float getTemperature(void);
		float getVoltage(void);

		void doCalibration(byte mode);

	private:
		int o2_pin;

		int VREF = 5000;    //for arduino uno, the ADC reference is the AVCC, that is 5000mV(TYP)
		float doValue;                                                //current dissolved oxygen value, unit; mg/L
		float temperature = 25;                                       //default temperature is 25^C, you can use a temperature sensor to read it
		
		const static int ReceivedBufferLength = 20;
		char receivedBuffer[ReceivedBufferLength + 1];        // store the serial command
		byte receivedBufferIndex = 0;
	
		const static int SCOUNT = 30;          // sum of sample point
		int analogBuffer[SCOUNT];    //store the analog value in the array, readed from ADC
		int analogBufferTemp[SCOUNT];
		int analogBufferIndex = 0, copyIndex = 0;

		float SaturationDoVoltage, SaturationDoTemperature;
		float averageVoltage;

		int getMedianNum(int bArray[], int iFilterLen);
		byte uartParse();
		boolean serialDataAvailable(void);

		bool temp_check = false;
		temperaturesensor* tempsensor;
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
		void Sonar(int trigPin, int echoPin);

	private:
		int trig_pin;
		int echo_pin;

		long distance;
		long duration;
};



#endif

