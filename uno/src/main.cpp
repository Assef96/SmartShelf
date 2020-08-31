/////////////////// Libraries: /////////////////////

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SoftwareI2C.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <dht11.h>
#include <HX711.h>

////////////////// Global Objects and Variables: //////////////////////
const bool debug = false;
const bool debugSensors = false;
bool isReady = false;
bool buzzerOn = true;
unsigned long wakeUpTime = 3000; 
unsigned long lastLoopTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long loopPeriod = 50; // delay between updates, in milliseconds
bool ledStatus;

SoftwareI2C WireS;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);
HX711 scale;

const size_t numberOfUnits = 5;
const size_t numberOfSensors = 12;
const size_t sensorsUnits[12] = {1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 4, 5};
int sensorsStatus[numberOfSensors];
int sensorsLastStatus[numberOfSensors];
int unitsStatus[numberOfUnits];
String unitsProductName[numberOfUnits];
int unitsProductPrice[numberOfUnits];
int unitsProductId[numberOfUnits];
int lightIntensity;

const int8_t i2cMaster = 0x42;
const int8_t i2cSlave = 0x08;
const uint8_t i2cBufferLength = 80;
char command;


///////////////////// Pin Map: ////////////////////
// const int rxPin = 0;
// const int txPin = 1;
const int trigPin = 2;
const int echoPin = 3;
const int scalePinDT = 4; // or 
const int scalePinSCK = 5; // vice versa
const int lcdPinSCK = 6;
const int lcdPinSDA = 7;
const int buzzerPin = 8;
const int fanPin = 9;
const int lampPin = 10;
const int photoModulePin = 11;
const int irModulePin = 12;
const int ledPin = 13;
const int photo2Pin = A0;
const int photo1Pin = A1;
const int irPin = A2;
const int irDistancePin = A3;
// const int sdaPin = A4;
// const int sckPin = A5;

///////////////////// Function Definition: /////////////////////
void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void handleRequests();
void readCommands(DynamicJsonDocument& doc);
void readSwitches(DynamicJsonDocument& doc);
void updateSensors();
void readUnitsProducts(DynamicJsonDocument& doc);
void updateSensorsStatus();
int detectChangedUnit();
void displayChangedUnit();
void calibrateScale();

void setup()
{
	Serial.begin(9600);

	Wire.begin(i2cSlave);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	scale.begin(scalePinDT, scalePinSCK);
	// calibrateScale();
	scale.set_offset(89975);
	scale.set_scale(22968);

	lcd.init(&WireS, lcdPinSDA, lcdPinSCK);
	lcd.backlight();
	lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
	lcd.print(F("$ Smart  Shelf $"));
	lcd.setCursor(0, 1);
	lcd.print(F(" Enjoy Shopping "));

	pinMode(buzzerPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(lampPin, OUTPUT);
	pinMode(fanPin, OUTPUT);
	pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
	pinMode(echoPin, INPUT); // Sets the echoPin as an Input
	digitalWrite(lampPin, HIGH);
	digitalWrite(fanPin, HIGH);
}

void loop()
{
	if (millis() - lastLoopTime > loopPeriod)
	{
		if (lastLoopTime > wakeUpTime)
			isReady = true;
		
		lastLoopTime = millis();
		ledStatus = !ledStatus;
		digitalWrite(ledPin, ledStatus);
		displayChangedUnit();

		Serial.println(F("--------- End of Loop ---------"));
	}
}

void receiveEvent(int numBytes)
{
	if (debug)
	{
		Serial.print(F("Incoming Event"));
		Serial.print(F("  ->  Number of Bytes: "));
		Serial.print(numBytes);
	}
	command = Wire.read();
	if (debug)
	{
		Serial.print(F("  ->  command: "));
		Serial.println(command);
	}
	if (numBytes > 1)
	{
		char buffer[i2cBufferLength];
		size_t index{};
		while(Wire.available())
		{
			char x = Wire.read();
			buffer[index++] = x;
		}
		buffer[index] = '\0';
		if (debug)
		{
			Serial.print(F("input buffer: "));
			Serial.println(buffer);
		}
		const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
		DynamicJsonDocument doc(capacity);
		DeserializationError error = deserializeJson(doc, buffer, index);
		if (error)
		{
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.c_str());
			return;
		}
		
		switch (command)
		{
		case 'c':
			readCommands(doc);
			break;
		case 'p':
			readUnitsProducts(doc);
			break;
		case 's':
			readSwitches(doc);
			break;
		default:
			break;
		}
	}
}

void requestEvent()
{
	if (debug)
		Serial.println(F("Incoming Request"));
	switch (command)
	{

	case 'u':
		updateSensors();
	default:
		break;
	}
}

void sendResponse(char buffer[])
{
	if (debug)
	{
		Serial.print(F("output buffer: "));
		Serial.println(buffer);
	}
	
	bool endOfResponse = false;
	for (size_t i = 0; i < i2cBufferLength; i++)
	{
		char ch = buffer[i];
		if (ch == '\0')
			endOfResponse = true;
		if (endOfResponse)
			Wire.write('\0');
		else
			Wire.write(ch);
	}
}

void readCommands(DynamicJsonDocument& doc)
{
	buzzerOn = doc[F("b")].as<int>();
	int lampStatus{doc[F("l")].as<int>()};
	int fanStatus{doc[F("f")].as<int>()};
	digitalWrite(lampPin, !lampStatus);
	digitalWrite(fanPin, !fanStatus);
}

void readUnitsProducts(DynamicJsonDocument& doc)
{
	int index{doc[F("id")].as<int>() - 1};
	String name{doc[F("name")].as<char *>()};
	int price{doc[F("price")].as<int>()};
	int productId{doc[F("pid")].as<int>()};
	unitsProductName[index] = name;
	unitsProductPrice[index] = price;
	unitsProductId[index] = productId;
}

void readSwitches(DynamicJsonDocument& doc)
{
	sensorsStatus[0] = doc[F("a")].as<int>();
	sensorsStatus[1] = doc[F("b")].as<int>();
	sensorsStatus[2] = doc[F("c")].as<int>();
	sensorsStatus[3] = doc[F("d")].as<int>();
	lightIntensity = doc[F("p")].as<int>();
}

void updateSensors()
{
	char buffer[i2cBufferLength];
	const size_t capacity = JSON_OBJECT_SIZE(12) + 50;
	DynamicJsonDocument doc(capacity);

	for (size_t i = 0; i < numberOfSensors; i++)
		doc[String(i + 1)] = sensorsStatus[i];
	serializeJson(doc, buffer, i2cBufferLength);
	sendResponse(buffer);
}

void updateSensorsStatus()
{
	// Scale:
	if (scale.is_ready())
		sensorsStatus[4] = round(scale.get_units(5));
	else
		Serial.println(F("HX711 not found."));

	// Photoresistor:
	int photo1Value = analogRead(photo1Pin);
	int photo2Value = analogRead(photo2Pin);
	sensorsStatus[5] = photo1Value > 300 + 0.5 * lightIntensity ? true : false;
	sensorsStatus[9] = photo2Value > 250 + 0.4 * lightIntensity ? true : false;

	// Photoresistor and IR Modules:
	bool photoModuleStatus = digitalRead(photoModulePin);
	sensorsStatus[6] = photoModuleStatus;
	bool irModuleStatus = !digitalRead(irModulePin);
	sensorsStatus[8] = irModuleStatus;

	// IR:
	int irValue = analogRead(irPin);
	sensorsStatus[7] = irValue < 600 ? true : false;

	// Ultrasonic:
	long duration{};
	float distance{};
	float totalDistance{};
	for (size_t i = 0; i < 5; i++)
	{
		noInterrupts();
		digitalWrite(trigPin, LOW);
		delayMicroseconds(2);
		digitalWrite(trigPin, HIGH);
		delayMicroseconds(10);
		digitalWrite(trigPin, LOW);
		duration = pulseIn(echoPin, HIGH);
		interrupts();
		distance = duration * 0.0343 / 2;
		totalDistance += distance;
		delay(10);
		// Serial.print(distance);
		// Serial.print("  ");
	}
	// Serial.println();
	distance = totalDistance / 5;
	Serial.println(distance);
	if (distance >= 18.34)
	{
		sensorsStatus[10] = 0;
	}
	else if (distance >= 16.09 && distance < 18.34)
	{
		sensorsStatus[10] = 1;
	}
	else if (distance >= 13.65 && distance < 16.09)
	{
		sensorsStatus[10] = 2;
	}
	else if (distance >= 2 && distance < 13.65)
	{
		sensorsStatus[10] = 3;
	}
	else
	{
		sensorsStatus[10] = 0;
	}

	// IR Distance:
	float irDistanceValue{};
	float totalIrDistance{};
	for (size_t i = 0; i < 5; i++)
	{
		irDistanceValue = analogRead(irDistancePin);
		totalIrDistance += irDistanceValue;
		delay(1);
		// Serial.print(irDistanceValue);
		// Serial.print(F("  "));
	}
	// Serial.println();
	irDistanceValue = totalIrDistance / 5;
	Serial.println(irDistanceValue);

	if (irDistanceValue >= 900)
	{
		sensorsStatus[11] = 0;
	}
	else if (irDistanceValue >= 714 && irDistanceValue < 900)
	{
		sensorsStatus[11] = 1;
	}
	else if (irDistanceValue >= 685 && irDistanceValue < 714)
	{
		sensorsStatus[11] = 2;
	}
	else if (irDistanceValue >= 200 && irDistanceValue < 685)
	{
		sensorsStatus[11] = 3;
	}
	else
	{
		sensorsStatus[11] = 0;
	}	

	unitsStatus[0] = sensorsStatus[0] + sensorsStatus[1];
	unitsStatus[0] += sensorsStatus[2] + sensorsStatus[3];
	unitsStatus[1] = sensorsStatus[4];
	unitsStatus[2] = sensorsStatus[5] + sensorsStatus[6] + sensorsStatus[7];
	unitsStatus[2] += sensorsStatus[8] + sensorsStatus[9];
	unitsStatus[3] = sensorsStatus[10];
	unitsStatus[4] = sensorsStatus[11];


	if (debugSensors)
	{
		Serial.print(F("Switches status: "));
		Serial.print(sensorsStatus[0]);
		Serial.print(sensorsStatus[1]);
		Serial.print(sensorsStatus[2]);
		Serial.println(sensorsStatus[3]);
		Serial.print(F("Scale Units: "));
		Serial.println(unitsStatus[4]);
		Serial.print(F("Ultrasonic Distance: "));
		Serial.println(distance);
		Serial.print(F("IR Value : "));
		Serial.println(irValue);
		Serial.print(F("IR Distance Value: "));
		Serial.println(irDistanceValue);
		Serial.print(F("Light Intensity: "));
		Serial.println(lightIntensity);
		Serial.print(F("Photo 1 Value: "));
		Serial.println(photo1Value);
		Serial.print(F("Photo 2 Value: "));
		Serial.println(photo2Value);
		Serial.print(F("IR Module Value: "));
		Serial.println(irModuleStatus);
		Serial.print(F("Photo Module Value: "));
		Serial.println(photoModuleStatus);
	}
	
}

int detectChangedUnit()
{
	int sensorIndex{}; // + for putting down and - for picking up
	for (size_t i = 0; i < numberOfSensors; i++)
	{
		if (sensorsStatus[i] < sensorsLastStatus[i]) // picked up
			sensorIndex = -(i + 1);
		else if (sensorsStatus[i] > sensorsLastStatus[i]) // put down
			sensorIndex = +(i + 1);
		sensorsLastStatus[i] = sensorsStatus[i];
	}	
	return sensorIndex;
}

void displayChangedUnit()
{
	updateSensorsStatus();
	int sensorIndex = detectChangedUnit();
	if(sensorIndex == 0) // Nothing was picked up or put down
		return;
	if(!isReady)
		return;
	int sensorId = abs(sensorIndex);
	bool pickedUp = (sensorIndex < 0) ? true : false;


	Serial.print(F("sensor "));
	Serial.print(sensorId);
	Serial.print(F(" was "));
	Serial.println(pickedUp ? F(" picked up") : F(" put down"));
	if (buzzerOn)
	{
		if (pickedUp)
		{
			tone(buzzerPin, 6000, 180);
		}
		else
		{
			tone(buzzerPin, 2000, 100);
		}
	}
	size_t unitId = sensorsUnits[sensorId - 1];
	lcd.clear();
	lcd.print(unitsProductName[unitId - 1]);
	lcd.setCursor(15, 1);
	lcd.print(unitId);
	lcd.setCursor(0, 1);
	lcd.print(F("P"));
	lcd.print(unitsProductId[unitId - 1]);
	lcd.setCursor(6, 1);
	lcd.print(unitsProductPrice[unitId - 1]);
	lcd.print(F("$"));
	lcd.setCursor(11, 1);
	lcd.print(unitsStatus[unitId - 1]);
	lcd.print(F("left"));
}

void calibrateScale()
{
	Serial.print(F("Offset: \t\t"));
	long offset = scale.read_average(20);
	Serial.println(offset);
	scale.set_offset(offset);
	Serial.println(F("Set one unit"));
	for (size_t i = 0; i < 10; i++)
	{
		delay(400);
		Serial.print('.');
	}
	Serial.println();
	Serial.print(F("One unit: \t\t"));
	double scaleUnit = scale.get_value(20);
	Serial.println(scaleUnit);
	scale.set_scale(scaleUnit);
	Serial.print(F("Sensors: \t\t"));
	Serial.println(scale.get_units(20));
	Serial.println(F("Calibration done."));
}
