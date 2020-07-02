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

//  SDA_PIN A4 and SCL_PIN A5
const int8_t I2C_MASTER = 0x42;
const int8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 60;
char command;

unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 2L * 1000L; // delay between updates, in milliseconds

bool ledStatus;
const int ledPin = LED_BUILTIN;
const int lcdPinSDA = 2;
const int lcdPinSCK = 3;
const int buzzerPin = 4;
const int trigPin = 5;
const int echoPin = 6;
const int unitsPin[3] = {8, 9, 10}; //
const int lamp1Pin = 11;
const int lamp2Pin = 12;
const int photo1Pin = A0;
const int photo2Pin = A1;
const int irReceiver1Pin = A2;
const int irReceiver2Pin = A3;
// const int sdaPin = A4;
// const int sckPin = A5;

bool photoPinLastStatus;
const size_t numberOfUnits{9};
int unitsStatus[numberOfUnits];
int unitsLastStatus[numberOfUnits];
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];

const bool debug = false;
bool FirstTime = true;
bool buzzerOn = false;

void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void handleRequests();
void readCommands(DynamicJsonDocument& doc);
void updateUnits();
void readUnits(DynamicJsonDocument& doc);
void updateUnitsStatus();
int detectChangedUnit();
void displayChangedUnit();

void setup()
{
	Serial.begin(9600);

	lcd.init(&WireS1, lcdPinSDA, lcdPinSCK);
	lcd.backlight();
	lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
	lcd.print(F("$ Smart  Shelf $")); // Print the string "Hello World!"
	lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
	lcd.print(F(" Enjoy Shopping "));

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	for (size_t i = 0; i < 3; i++)
	{
		pinMode(unitsPin[i], INPUT_PULLUP);
	}
	
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
		lastConnectionTime = millis();
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
	// defines variables
	long duration;
	int distance;
	// Clears the trigPin
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);
	// Sets the trigPin on HIGH state for 10 micro seconds
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);
	// Reads the echoPin, returns the sound wave travel time in microseconds
	duration = pulseIn(echoPin, HIGH);
	// Calculating the distance
	distance= duration*0.034/2;
	// Prints the distance on the Serial Monitor
	Serial.print("Distance: ");
	Serial.println(distance);

	Serial.print("IR Value 1 : ");
	int irValue1 = analogRead(irReceiver1Pin);
	Serial.println(irValue1);
	unitsStatus[7] = irValue1 < 700 ? true : false;

	Serial.print("IR Value 2 : ");
	int irValue2 = analogRead(irReceiver2Pin);
	Serial.println(irValue2);
	unitsStatus[8] = irValue2 < 700 ? true : false;


	
	// 	float h = 35;
	// 	float t = 22;
	// 	float b = analogRead(A0);
	// 	if (isnan(h) || isnan(t) || isnan(b))
	// 	{
	// 		Serial.println(F("Failed to read from DHT sensor!"));
	// 		return;
	// 	}
	
	int photo1Value = analogRead(photo1Pin);
	Serial.println(photo1Value);
	int photo2Value = analogRead(photo2Pin);
	Serial.println(photo2Value);
	unitsStatus[0] = digitalRead(unitsPin[0]);
	unitsStatus[1] = digitalRead(unitsPin[1]);
	unitsStatus[3] = photo1Value > 300 ? true : false;
	unitsStatus[4] = photo2Value > 300 ? true : false;
	unitsStatus[5] = int(distance / 20);
}

int detectChangedUnit()
{
	int unitIndex{}; // + for putting down and - for picking up
	for (size_t i = 0; i < numberOfUnits; i++)
	{
		if (unitsStatus[i] < unitsLastStatus[i]) // picked up
		{
			unitIndex = -(i + 1);
			unitsLastStatus[i] = unitsStatus[i];
			break;
		}
		else if (unitsStatus[i] > unitsLastStatus[i]) // put down
		{
			unitIndex = +(i + 1);
			unitsLastStatus[i] = unitsStatus[i];
			break;
		}
	}	
	if (FirstTime)
	{
		FirstTime = false;
		return 0;
	}
	else
		return unitIndex;
}

void displayChangedUnit()
{
	updateUnitsStatus();
	int unitIndex = detectChangedUnit();
	if(unitIndex == 0) // Nothing was picked up or put down
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
			tone(buzzerPin, 5000, 50);
			delay(100);
			tone(buzzerPin, 5000, 50);
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
