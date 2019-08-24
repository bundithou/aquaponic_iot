/*
  Aqualib.h - Library for Aquaponic system code.
  Created by Bundit Hou, September 24, 2019.
  Not released into the public domain.
*/

#include "Arduino.h"
#include "WProgram.h"
#include "Aqualib.h"


pHsensor::pHsensor(int pin, int led)
{
	this -> pH_pin = pin;
	this -> LED = led;
	pinMode(LED, OUTPUT);
	pinMode(pH_pin, INPUT);
}

float pHsensor::getpH(void) {
	return pHValue;
}

float pHsensor::getVoltage(void) {
	return voltage;
}

void pHsensor::calulatepH(void)
{
	static unsigned long samplingTime = millis();
	static unsigned long printTime = millis();
	static float pv, vol;
	if (millis() - samplingTime > samplingInterval)
	{
		pHArray[pHArrayIndex++] = analogRead(this->pH_pin);
		if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
		vol = avergearray(pHArray, ArrayLenth)*5.0 / 1024;
		pv = 3.5*vol + Offset;
		samplingTime = millis();
	}
	if (millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
	{
		//Serial.print("Voltage:");
		//Serial.print(voltage, 2);
		//Serial.print("    pH value: ");
		//Serial.println(pHValue, 2);
		pHValue = pv;
		voltage = vol;
		digitalWrite(LED, digitalRead(LED) ^ 1);
		printTime = millis();
	}
}

double pHsensor::avergearray(int* arr, int number) {
	int i;
	int max, min;
	double avg;
	long amount = 0;
	if (number <= 0) {
		Serial.println("Error number for the array to avraging!/n");
		return 0;
	}
	if (number < 5) {   //less than 5, calculated directly statistics
		for (i = 0; i < number; i++) {
			amount += arr[i];
		}
		avg = amount / number;
		return avg;
	}
	else {
		if (arr[0] < arr[1]) {
			min = arr[0]; max = arr[1];
		}
		else {
			min = arr[1]; max = arr[0];
		}
		for (i = 2; i < number; i++) {
			if (arr[i] < min) {
				amount += min;        //arr<min
				min = arr[i];
			}
			else {
				if (arr[i] > max) {
					amount += max;    //arr>max
					max = arr[i];
				}
				else {
					amount += arr[i]; //min<=arr<=max
				}
			}//if
		}//for
		avg = (double)amount / (number - 2);
	}//if
	return avg;
}


// library used for oxygen sensor
#include <avr/pgmspace.h>
#include <EEPROM.h>

// #define used for oxygen sensor
#define SaturationDoVoltageAddress              12          //the address of the Saturation Oxygen voltage stored in the EEPROM
#define SaturationDoTemperatureAddress          16      //the address of the Saturation Oxygen temperature stored in the EEPROM

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

/*
oxygen sensor function
*/
o2sensor::o2sensor(int pin)
{
	this->o2_pin = pin;
	pinMode(o2_pin, INPUT);
	readDoCharacteristicValues();
}

float o2sensor::getO2(void) {
	return doValue;
}

float o2sensor::getTemperature(void) {
	return temperature;
}
float o2sensor::getVoltage(void) {
	return averageVoltage;
}

void o2sensor::calculateO2(void)
{
	static unsigned long analogSampleTimepoint = millis();
	if (millis() - analogSampleTimepoint > 30U)     //every 30 milliseconds,read the analog value from the ADC
	{
		analogSampleTimepoint = millis();
		analogBuffer[analogBufferIndex] = analogRead(o2_pin);    //read the analog value and store into the buffer
		analogBufferIndex++;
		if (analogBufferIndex == SCOUNT)
			analogBufferIndex = 0;
	}

	static unsigned long tempSampleTimepoint = millis();
	if (millis() - tempSampleTimepoint > 500U)  // every 500 milliseconds, read the temperature
	{
		tempSampleTimepoint = millis();
		temperature = readTemperature();  // add your temperature codes here to read the temperature, unit:^C
	}

	static unsigned long printTimepoint = millis();
	if (millis() - printTimepoint > 1000U)
	{
		printTimepoint = millis();
		for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
		{
			analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
		}
		averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the value more stable by the median filtering algorithm
		//Serial.print(F("Temperature:"));
		//Serial.print(temperature, 1);
		//Serial.print(F("^C"));
		doValue = pgm_read_float_near(&SaturationValueTab[0] + (int)(SaturationDoTemperature + 0.5)) * averageVoltage / SaturationDoVoltage;  //calculate the do value, doValue = Voltage / SaturationDoVoltage * SaturationDoValue(with temperature compensation)
		//Serial.print(F(",  DO Value:"));
		//Serial.print(doValue, 2);
		//Serial.print(F("mg/L"));
	}

	if (serialDataAvailable() > 0)
	{
		byte modeIndex = uartParse();  //parse the uart command received
		doCalibration(modeIndex);    // If the correct calibration command is received, the calibration function should be called.
	}

}
boolean o2sensor::serialDataAvailable(void)
{
	char receivedChar;
	static unsigned long receivedTimeOut = millis();
	while (Serial.available() > 0)
	{
		if (millis() - receivedTimeOut > 500U)
		{
			receivedBufferIndex = 0;
			memset(receivedBuffer, 0, (ReceivedBufferLength + 1));
		}
		receivedTimeOut = millis();
		receivedChar = Serial.read();
		if (receivedChar == '\n' || receivedBufferIndex == ReceivedBufferLength)
		{
			receivedBufferIndex = 0;
			strupr(receivedBuffer);
			return true;
		}
		else {
			receivedBuffer[receivedBufferIndex] = receivedChar;
			receivedBufferIndex++;
		}
	}
	return false;
}

byte o2sensor::uartParse()
{
	byte modeIndex = 0;
	if (strstr(receivedBuffer, "CALIBRATION") != NULL)
		modeIndex = 1;
	else if (strstr(receivedBuffer, "EXIT") != NULL)
		modeIndex = 3;
	else if (strstr(receivedBuffer, "SATCAL") != NULL)
		modeIndex = 2;
	return modeIndex;
}

void o2sensor::doCalibration(byte mode)
{
	char *receivedBufferPtr;
	static boolean doCalibrationFinishFlag = 0, enterCalibrationFlag = 0;
	float voltageValueStore;
	switch (mode)
	{
	case 0:
		if (enterCalibrationFlag)
			Serial.println(F("Command Error"));
		break;

	case 1:
		enterCalibrationFlag = 1;
		doCalibrationFinishFlag = 0;
		Serial.println();
		Serial.println(F(">>>Enter Calibration Mode<<<"));
		Serial.println(F(">>>Please put the probe into the saturation oxygen water! <<<"));
		Serial.println();
		break;

	case 2:
		if (enterCalibrationFlag)
		{
			Serial.println();
			Serial.println(F(">>>Saturation Calibration Finish!<<<"));
			Serial.println();
			EEPROM_write(SaturationDoVoltageAddress, averageVoltage);
			EEPROM_write(SaturationDoTemperatureAddress, temperature);
			SaturationDoVoltage = averageVoltage;
			SaturationDoTemperature = temperature;
			doCalibrationFinishFlag = 1;
		}
		break;

	case 3:
		if (enterCalibrationFlag)
		{
			Serial.println();
			if (doCalibrationFinishFlag)
				Serial.print(F(">>>Calibration Successful"));
			else
				Serial.print(F(">>>Calibration Failed"));
			Serial.println(F(",Exit Calibration Mode<<<"));
			Serial.println();
			doCalibrationFinishFlag = 0;
			enterCalibrationFlag = 0;
		}
		break;
	}
}

int o2sensor::getMedianNum(int bArray[], int iFilterLen)
{
	int bTab[iFilterLen];
	for (byte i = 0; i < iFilterLen; i++)
	{
		bTab[i] = bArray[i];
	}
	int i, j, bTemp;
	for (j = 0; j < iFilterLen - 1; j++)
	{
		for (i = 0; i < iFilterLen - j - 1; i++)
		{
			if (bTab[i] > bTab[i + 1])
			{
				bTemp = bTab[i];
				bTab[i] = bTab[i + 1];
				bTab[i + 1] = bTemp;
			}
		}
	}
	if ((iFilterLen & 1) > 0)
		bTemp = bTab[(iFilterLen - 1) / 2];
	else
		bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
	return bTemp;
}

void o2sensor::readDoCharacteristicValues(void)
{
	EEPROM_read(SaturationDoVoltageAddress, SaturationDoVoltage);
	EEPROM_read(SaturationDoTemperatureAddress, SaturationDoTemperature);
	if (EEPROM.read(SaturationDoVoltageAddress) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 1) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 2) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 3) == 0xFF)
	{
		SaturationDoVoltage = 1127.6;   //default voltage:1127.6mv
		EEPROM_write(SaturationDoVoltageAddress, SaturationDoVoltage);
	}
	if (EEPROM.read(SaturationDoTemperatureAddress) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 1) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 2) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 3) == 0xFF)
	{
		SaturationDoTemperature = 25.0;   //default temperature is 25^C
		EEPROM_write(SaturationDoTemperatureAddress, SaturationDoTemperature);
	}
}


/*
	soil noisture function
*/
soilMoisturesensor::soilMoisturesensor(int pin) 
{
	this->soil_pin = pin;
	pinMode(this->soil_pin, INPUT);
}

float soilMoisturesensor::getSoilMoisture(void) {
	return (100.00 - ((analogRead(this->soil_pin) / 1023.00) * 100.00));
}


/*
	ultrasonic
*/

ultrasonicsensor::ultrasonicsensor(int trigPin, int echoPin)
{
	this->trig_pin = trigPin;
	this->echo_pin = echoPin;
	pinMode(this->trig_pin, OUTPUT);
	pinMode(this->echo_pin, INPUT):
}

void ultrasonicsensor::Sonar(int trigPin, int echoPin)
{
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);
	duration = pulseIn(echoPin, HIGH);
	distance = (duration / 2) / 29.1;
}

float ultrasonicsensor::getDistance(void)
{
	Sonar(trig_pin, echo_pin);
	return distance;
}