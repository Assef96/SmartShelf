#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include "SoftwareI2C.h"
#include <dht11.h>
#include <HX711.h>
// #define Serial SerialUSB
// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
SoftwareI2C WireS1;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2); // Change to (0x27,16,2) for 16x2 LCD.

HX711 scale;

//  SDA_PIN A4 and SCL_PIN A5
const int8_t I2C_MASTER = 0x42;
const int8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 60;
char command;

unsigned long wakeUpTime = 5000L; 
unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1000L; // delay between updates, in milliseconds

bool ledStatus;

// const int rxPin = 0;
// const int txPin = 1;
const int trigPin = 2;
const int echoPin = 3;
const int scalePinDT = 4; // or 
const int scalePinSCK = 5; // vice versa
const int lcdPinSCK = 6;
const int lcdPinSDA = 7;
const int buzzerPin = 8;
const int lamp2Pin = 9;
const int lamp1Pin = 10;
const int photoModulePin = 11;
const int irModulePin = 12;
const int ledPin = 13;

const int photo1Pin = A0;
const int photo2Pin = A1;
const int irPin = A2;
const int irDistancePin = A3;
// const int sdaPin = A4;
// const int sckPin = A5;

bool photoPinLastStatus;
const size_t numberOfUnits{9};
int unitsStatus[numberOfUnits];
int unitsLastStatus[numberOfUnits];
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];

const bool debug = true;
const bool debugSensors = false;
// bool FirstTime = true;
bool isReady = false;
bool buzzerOn = true;

void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void handleRequests();
void readCommands(DynamicJsonDocument& doc);
void readSwitches(DynamicJsonDocument& doc);
void updateUnits();
void readUnits(DynamicJsonDocument& doc);
void updateUnitsStatus();
int detectChangedUnit();
void displayChangedUnit();
float oldDistance = 0;

void setup()
{
	Serial.begin(9600);

	scale.begin(scalePinDT, scalePinSCK);

	lcd.init(&WireS1, lcdPinSDA, lcdPinSCK);
	lcd.backlight();
	lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
	lcd.print(F("$ Smart  Shelf $")); // Print the string "Hello World!"
	lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
	lcd.print(F(" Enjoy Shopping "));

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);


	pinMode(buzzerPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(lamp1Pin, OUTPUT);
	pinMode(lamp2Pin, OUTPUT);
	digitalWrite(lamp1Pin, HIGH);
	digitalWrite(lamp2Pin, HIGH);
	pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
	pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop()
{
	if (millis() - lastConnectionTime > postingInterval)
	{
		if (lastConnectionTime > wakeUpTime)
			isReady = true;
		
		lastConnectionTime = millis();
		ledStatus = !ledStatus;
		// digitalWrite(ledPin, ledStatus);
		// displayChangedUnit();


		Serial.println(F("--------- End of Loop ---------"));
	}
		// Serial.print("Val: ");
		// Serial.println(analogRead(irPin));
		// delay(1000);

		// long duration;
		// float distance;
		// digitalWrite(trigPin, LOW);
		// delayMicroseconds(2);
		// digitalWrite(trigPin, HIGH);
		// delayMicroseconds(10);
		// digitalWrite(trigPin, LOW);
		// duration = pulseIn(echoPin, HIGH);
		// distance= duration*0.034/2;
		// if(distance != oldDistance)
		// {
		// 	lcd.clear();
		// 	lcd.print("distance: ");
		// 	lcd.setCursor(0, 1);
		// 	lcd.print(distance);
		// 	oldDistance = distance;
		// }
		// delay(500);
		// int value = analogRead(A0);
		// lcd.clear();
		// lcd.print("value: ");
		// lcd.setCursor(0, 1);
		// lcd.print(value);
		// delay(200);

}

void receiveEvent(int numBytes)
{
	if (debug)
	{
		Serial.print(F("Incoming Event"));
		Serial.print(F("  ->  howMany: "));
		Serial.print(numBytes);
	}
	command = Wire.read(); // receive byte as a character
	if (debug)
	{
		Serial.print(F("  ->  command: "));
		Serial.println(command);
	}
	if (numBytes > 1)
	{
		char buffer[I2C_BUFFER_LENGHT];
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
			readUnits(doc);
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
		updateUnits();
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
	for (size_t i = 0; i < I2C_BUFFER_LENGHT; i++)
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
	int lamp1Status{doc["l1"].as<int>()};
	int lamp2Status{doc["l2"].as<int>()};
	buzzerOn = doc["b"].as<int>();
	digitalWrite(lamp1Pin, !lamp1Status);
	digitalWrite(lamp2Pin, !lamp2Status);
}

void readUnits(DynamicJsonDocument& doc)
{
	int index{doc["id"].as<int>() - 1};
	String name{doc["name"].as<char *>()};
	long price{doc["price"].as<long>()};
	unitsProductName[index] = name;
	unitsProductPrice[index] = price;
}

void readSwitches(DynamicJsonDocument& doc)
{
	// unitsStatus[0] = doc["a"].as<int>();
	// unitsStatus[1] = doc["b"].as<int>();
	// unitsStatus[0] = doc["c"].as<int>();
	// unitsStatus[1] = doc["d"].as<int>();
}

void updateUnits()
{
	char buffer[I2C_BUFFER_LENGHT];
	const size_t capacity = JSON_OBJECT_SIZE(9) + 50;
	DynamicJsonDocument doc(capacity);
	for (size_t i = 0; i < numberOfUnits; i++)
		doc[String(i + 1)] = unitsLastStatus[i];
	serializeJson(doc, buffer, I2C_BUFFER_LENGHT);
	sendResponse(buffer);
}

void updateUnitsStatus()
{
	// 	float h = 35;
	// 	float t = 22;
	// 	float b = analogRead(A0);
	// 	if (isnan(h) || isnan(t) || isnan(b))
	// 	{
	// 		Serial.println(F("Failed to read from DHT sensor!"));
	// 		return;
	// 	}

	bool irModuleStatus = !digitalRead(irModulePin);
	unitsStatus[2] = irModuleStatus;

	int photo1Value = analogRead(photo1Pin);
	int photo2Value = analogRead(photo2Pin);
	unitsStatus[3] = photo1Value > 500 ? true : false;
	unitsStatus[4] = photo2Value > 500 ? true : false;

	long duration;
	int distance;
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);
	duration = pulseIn(echoPin, HIGH);
	distance= duration*0.034/2;
	if (distance > 45)
	{
		unitsStatus[5] = 0;
	}
	else
	{
		unitsStatus[5] = int((45 - distance) / 5);
	}
	

	int irValue1 = analogRead(irPin);
	unitsStatus[6] = irValue1 < 700 ? true : false;

	int irValue2 = analogRead(irDistancePin);
	unitsStatus[7] = irValue2 < 700 ? true : false;

	if (scale.is_ready())
	{
		long reading = scale.read();
		Serial.print("HX711 reading: ");
		Serial.println(reading);
	}
	else
	{
		Serial.println("HX711 not found.");
	}
	unitsStatus[8] = 0;


	if (debugSensors)
	{
		Serial.print(F("Switches status: "));
		Serial.print(unitsStatus[0]);
		Serial.println(unitsStatus[1]);
		Serial.print(F("Distance: "));
		Serial.println(distance);
		Serial.print(F("IR Value 1 : "));
		Serial.println(irValue1);
		Serial.print(F("IR Value 2 : "));
		Serial.println(irValue2);
		Serial.print(F("Photo 1 Value: "));
		Serial.println(photo1Value);
		Serial.print(F("Photo 2 Value: "));
		Serial.println(photo2Value);
		Serial.print(F("IR Module Value: "));
		Serial.println(irModuleStatus);
	}
	
}

int detectChangedUnit()
{
	int unitIndex{}; // + for putting down and - for picking up
	for (size_t i = 0; i < numberOfUnits; i++)
	{
		if (unitsStatus[i] < unitsLastStatus[i]) // picked up
			unitIndex = -(i + 1);
		else if (unitsStatus[i] > unitsLastStatus[i]) // put down
			unitIndex = +(i + 1);
		unitsLastStatus[i] = unitsStatus[i];
	}	
	return unitIndex;
}

void displayChangedUnit()
{
	updateUnitsStatus();
	int unitIndex = detectChangedUnit();
	if(unitIndex == 0) // Nothing was picked up or put down
		return;
	if(!isReady)
		return;
	int unitId = abs(unitIndex);
	bool pickedUp = (unitIndex < 0) ? true : false;

	Serial.print(F("unit "));
	Serial.print(unitId);
	Serial.print(F(" -> "));
	Serial.print(unitsProductName[unitId - 1]);
	Serial.print(F(" was "));
	Serial.println(pickedUp ? " picked up" : " put down");
	if (buzzerOn)
	{
		if (pickedUp)
		{
			tone(buzzerPin, 7000, 70);
		}
		else
		{
			tone(buzzerPin, 2000, 100);
		}
	}
	
	
	
	
	if(pickedUp)
	{
		lcd.clear();
		lcd.print(unitId);
		lcd.print(F(" -> "));
		lcd.print(unitsProductName[unitId - 1]);
		lcd.setCursor(0, 1);
		lcd.print(F("Price: "));
		lcd.print(unitsProductPrice[unitId - 1]);
	}
}
