#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SoftwareI2C.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <dht11.h>
#include <HX711.h>

////////////////// Global Objects and Variables: //////////////////////
SoftwareI2C WireS1;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);
HX711 scale;

const int8_t I2C_MASTER = 0x42;
const int8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 80;
char command;

unsigned long wakeUpTime = 3000L; 
unsigned long lastLoopTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long loopPeriod = 3000L; // delay between updates, in milliseconds

bool ledStatus;

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

bool photoPinLastStatus;
const size_t numberOfSensors{12};
int sensorsStatus[numberOfSensors];
int sensorsLastStatus[numberOfSensors];
size_t sensorsId[numberOfSensors];
String sensorsProductName[numberOfSensors];
long sensorsProductPrice[numberOfSensors];
int lightIntensity;

const bool debug = false;
const bool debugSensors = true;
bool isReady = false;
bool buzzerOn = true;

///////////////////// Function Definition: /////////////////////
void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void handleRequests();
void readCommands(DynamicJsonDocument& doc);
void readSwitches(DynamicJsonDocument& doc);
void updateSensors();
void readSensorsProducts(DynamicJsonDocument& doc);
void updateSensorsStatus();
int detectChangedUnit();
void displayChangedUnit();

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
	pinMode(lampPin, OUTPUT);
	pinMode(fanPin, OUTPUT);
	digitalWrite(lampPin, HIGH);
	digitalWrite(fanPin, HIGH);
	pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
	pinMode(echoPin, INPUT); // Sets the echoPin as an Input


	scale.set_offset(89831);
	scale.set_scale(72673);

	// Serial.print("Offset: \t\t");
	// long offset = scale.read_average(20);
	// Serial.println(offset);
	// scale.set_offset(offset);
	// Serial.println("Set one unit");
	// for (size_t i = 0; i < 10; i++)
	// {
	// 	delay(400);
	// 	Serial.print('.');
	// }
	// Serial.println();
	// Serial.print("One unit: \t\t");
	// double scaleUnit = scale.get_value(20);
	// Serial.println(scaleUnit);  	// print the average of 20 readings from the ADC
	// scale.set_scale(scaleUnit);
	// Serial.print("Sensors: \t\t");
	// Serial.println(scale.get_units(20));			// print a raw reading from the ADC
	// Serial.println("Calibration done.");

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
			readSensorsProducts(doc);
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
	int lampStatus{doc["l"].as<int>()};
	int fanStatus{doc["f"].as<int>()};
	buzzerOn = doc["b"].as<int>();
	digitalWrite(lampPin, !lampStatus);
	digitalWrite(fanPin, !fanStatus);
}

void readSensorsProducts(DynamicJsonDocument& doc)
{
	int index{doc["id"].as<int>() - 1};
	String name{doc["name"].as<char *>()};
	long price{doc["price"].as<long>()};
	sensorsProductName[index] = name;
	sensorsProductPrice[index] = price;
}

void readSwitches(DynamicJsonDocument& doc)
{
	sensorsStatus[0] = doc["a"].as<int>();
	sensorsStatus[1] = doc["b"].as<int>();
	sensorsStatus[2] = doc["c"].as<int>();
	sensorsStatus[3] = doc["d"].as<int>();
	lightIntensity = doc["p"].as<int>();
}

void updateSensors()
{
	char buffer[I2C_BUFFER_LENGHT];
	const size_t capacity = JSON_OBJECT_SIZE(12) + 50;
	DynamicJsonDocument doc(capacity);

	for (size_t i = 0; i < numberOfSensors; i++)
		doc[String(i + 1)] = sensorsStatus[i];
	serializeJson(doc, buffer, I2C_BUFFER_LENGHT);
	sendResponse(buffer);
}

void updateSensorsStatus()
{
	// 	if (isnan(h) || isnan(t) || isnan(b))
	// 	{
	// 		Serial.println(F("Failed to read from DHT sensor!"));
	// 		return;
	// 	}

	double scaleUnits = 0;
	if (scale.is_ready())
		scaleUnits = scale.get_units(5);
	else
		Serial.println("HX711 not found.");
	sensorsStatus[4] = round(scaleUnits);

	int photo1Value = analogRead(photo1Pin);
	int photo2Value = analogRead(photo2Pin);
	sensorsStatus[5] = photo1Value > 300 + 0.5 * lightIntensity ? true : false;
	sensorsStatus[9] = photo2Value > 250 + 0.4 * lightIntensity ? true : false;

	bool photoModuleStatus = digitalRead(photoModulePin);
	sensorsStatus[6] = photoModuleStatus;

	bool irModuleStatus = !digitalRead(irModulePin);
	sensorsStatus[8] = irModuleStatus;

	int irValue = analogRead(irPin);
	sensorsStatus[7] = irValue < 600 ? true : false;

	long duration{};
	float distance{};
	float totalDistance{};
	for (size_t i = 0; i < 10; i++)
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
	distance = totalDistance / 10;
	// Serial.print(distance);
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
	else if (distance >= 10 && distance < 13.65)
	{
		sensorsStatus[10] = 3;
	}
	else
	{
		sensorsStatus[10] = 0;
	}

	float irDistance{};
	float totalIrDistance{};
	for (size_t i = 0; i < 10; i++)
	{
		irDistance = analogRead(irDistancePin);
		totalIrDistance += irDistance;
		delay(1);
		Serial.print(irDistance);
		Serial.print("  ");
	}
	Serial.println();
	irDistance = totalIrDistance / 10;
	Serial.print(irDistance);
	// if (distance > 19)
	// {
	// 	sensorsStatus[10] = 0;
	// }
	// else
	// {
	// 	sensorsStatus[10] = int((19 - distance) / 5);
	// }
	
	int irDistanceValue = analogRead(irDistancePin);
	sensorsStatus[11] = irDistanceValue < 400 ? true : false;

	if (irDistanceValue >= 800)
	{
		sensorsStatus[11] = 0;
	}
	else if (irDistanceValue >= 704 && irDistanceValue < 800)
	{
		sensorsStatus[10] = 1;
	}
	else if (irDistanceValue >= 660 && irDistanceValue < 704)
	{
		sensorsStatus[11] = 2;
	}
	else if (irDistanceValue >= 500 && irDistanceValue < 660)
	{
		sensorsStatus[11] = 3;
	}
	else
	{
		sensorsStatus[11] = 0;
	}

	if (debugSensors)
	{
		Serial.print(F("Switches status: "));
		Serial.print(sensorsStatus[0]);
		Serial.print(sensorsStatus[1]);
		Serial.print(sensorsStatus[2]);
		Serial.println(sensorsStatus[3]);
		Serial.print("Scale Units: ");
		Serial.println(scaleUnits);
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
	int unitIndex{}; // + for putting down and - for picking up
	for (size_t i = 0; i < numberOfSensors; i++)
	{
		if (sensorsStatus[i] < sensorsLastStatus[i]) // picked up
			unitIndex = -(i + 1);
		else if (sensorsStatus[i] > sensorsLastStatus[i]) // put down
			unitIndex = +(i + 1);
		sensorsLastStatus[i] = sensorsStatus[i];
	}	
	return unitIndex;
}

void displayChangedUnit()
{
	updateSensorsStatus();
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
	Serial.print(sensorsProductName[unitId - 1]);
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
	}
	lcd.clear();
	lcd.print(unitId);
	lcd.print(F(" -> "));
	lcd.print(sensorsProductName[unitId - 1]);
	lcd.setCursor(0, 1);
	lcd.print(F("Price: "));
	lcd.print(sensorsProductPrice[unitId - 1]);
	lcd.setCursor(15, 1);
	lcd.print(sensorsStatus[unitId - 1]);
}
