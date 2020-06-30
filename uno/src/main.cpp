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
const int trigPin = 5;
const int echoPin = 6;
const int unitsPin[3] = {8, 9, 10};
const int lamp1Pin = 11;
const int lamp2Pin = 12;
const int photoPin = A0;
bool photoPinLastStatus;
bool unitsLastStatus[3] = {false, false, false};
const size_t numberOfUnits{3};
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];

void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void handleRequests();
void readCommands(char buffer[], size_t size);
void updateUnits(char buffer[], const size_t &bufferSize);
void readUnits(char buffer[], size_t size);
int detectChangedUnit();
void displayChangedUnit();

void setup()
{
	Serial.begin(9600);



	lcd.init(&WireS1, 2, 3);
	lcd.backlight();
	lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
	lcd.print(F("Hello World!")); // Print the string "Hello World!"
	lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
	lcd.print(F("LCD tutorial"));

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	for (size_t i = 0; i < 3; i++)
	{
		pinMode(unitsPin[i], INPUT_PULLUP);
	}
	
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

		Serial.println(F("--------- End of Loop ---------"));
	}
	

	displayChangedUnit();
	delay(200);
}

void receiveEvent(int numBytes)
{
	Serial.print(F("Incoming Event"));
	Serial.print(F("  ->  howMany: "));
	Serial.print(numBytes);
	command = Wire.read(); // receive byte as a character
	Serial.print(F("  ->  command: "));
	Serial.println(command);

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
		Serial.print(F("input buffer: "));
		Serial.println(buffer);
		switch (command)
		{
		case 'c':
			readCommands(buffer, index);
			break;
		case 'p':
			readUnits(buffer, index);
		default:
			break;
		}
	}
}

void requestEvent()
{
	Serial.println(F("Incoming Request"));
	char buffer[I2C_BUFFER_LENGHT];
	size_t index{};
	switch (command)
	{

	case 'u':
		updateUnits(buffer, index);
		sendResponse(buffer);
	default:
		break;
	}
}

void sendResponse(char buffer[])
{
	Serial.print(F("output buffer: "));
	bool endOfResponse = false;
	for (size_t i = 0; i < I2C_BUFFER_LENGHT; i++)
	{
		char ch = buffer[i];
		if (ch == '\0')
			endOfResponse = true;
		if (endOfResponse)
			Wire.write('\0');
		else
		{
			Wire.write(ch);
			Serial.print(ch);
		}
	}
	Serial.println();
}

void readCommands(char buffer[], size_t bufferSize)
{
	const size_t capacity = 100;
	DynamicJsonDocument doc(capacity);
	DeserializationError error = deserializeJson(doc, buffer, bufferSize);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}
	int lamp1Status{doc["l1"].as<int>()};
	int lamp2Status{doc["l2"].as<int>()};
	// int buzzerStatus{doc["b"].as<int>()};
	digitalWrite(lamp1Pin, !lamp1Status);
	digitalWrite(lamp2Pin, !lamp2Status);
}

void readUnits(char buffer[], size_t bufferSize)
{
	// Serial.println("reading units");
	const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
	DynamicJsonDocument doc(capacity);
	DeserializationError error = deserializeJson(doc, buffer, bufferSize);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}
	int index{doc["id"].as<int>() - 1};
	String name{doc["name"].as<char *>()};
	long price{doc["price"].as<long>()};
	String msg = String("index: ") + String(index) + String("   name :") + name + String("   price :") + price;
	Serial.println(msg);
	unitsProductName[index] = name;
	unitsProductPrice[index] = price;
}

void updateUnits(char buffer[], const size_t &size)
{
	const size_t capacity = JSON_OBJECT_SIZE(9) + 50;
	DynamicJsonDocument doc(capacity);
	for (size_t i = 0; i < numberOfUnits; i++)
	{
		bool status = unitsLastStatus[i];
		doc[String(i + 1)] = status == true ? 1 : 0;
	}
	serializeJson(doc, buffer, I2C_BUFFER_LENGHT);
}

int detectChangedUnit()
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

	
	// 	float h = 35;
	// 	float t = 22;
	// 	float b = analogRead(A0);
	// 	if (isnan(h) || isnan(t) || isnan(b))
	// 	{
	// 		Serial.println(F("Failed to read from DHT sensor!"));
	// 		return;
	// 	}
	int unitIndex{}; // + for putting down and - for picking up
	bool unitsStatus[3];
	for (size_t i = 0; i < 2; i++)
	{
		if(i < 2)
		{
			unitsStatus[i] = digitalRead(unitsPin[i]);
		}
		if(i == 2)
		{
			int photoValue = analogRead(A0);
			// Serial.println(photoValue);
			unitsStatus[i] = photoValue > 600 ? true : false;
		}

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
	return unitIndex;
}

void displayChangedUnit()
{
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
	if(pickedUp)
	{
		lcd.clear();
		lcd.print(F("unit "));
		lcd.print(unitId);
		lcd.print(F(" -> "));
		lcd.print(unitsProductName[unitId - 1]);
		lcd.setCursor(0, 1);
		lcd.print(F("Price: "));
		lcd.print(unitsProductPrice[unitId - 1]);
	}
}
