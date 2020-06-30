#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#define Serial SerialUSB

// Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//  SDA_PIN A4 and SCL_PIN A5
const int8_t I2C_MASTER = 0x42;
const int8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 250;
char command;

unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 2L * 1000L; // delay between updates, in milliseconds

bool ledStatus;
const int ledPin = LED_BUILTIN;
const int lightPin = 22;
const int alarmPin = 12;
const int unitsPin[3] = {8, 9, 10};
const int photoPin = A0;
// bool photoPinLastStatus;
bool unitsLastStatus[3] = {false, false, false};
const size_t numberOfUnits{3};
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];

void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void changeLightStatus(const String &lightStatus);
void changeAlarmStatus(const String &alarmStatus);
void handleRequests();
void readActuators(char buffer[], size_t size);
void updateUnits(char buffer[], const size_t &bufferSize);
void readUnits(char buffer[], size_t size);
int detectChangedUnit();
void displayChangedUnit();

void setup()
{
	Serial.begin(115200);

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	// Print a message to the LCD.
	lcd.print("hello, world!");

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	for (size_t i = 0; i < 3; i++)
	{
		pinMode(unitsPin[i], INPUT);
	}
	
	pinMode(ledPin, OUTPUT);
	pinMode(lightPin, OUTPUT);
	// pinMode(alarmPin, OUTPUT);
}

void loop()
{

	if (millis() - lastConnectionTime > postingInterval)
	{
		lastConnectionTime = millis();
		ledStatus = !ledStatus;
		digitalWrite(ledPin, ledStatus);

		Serial.println("--------- End of Loop ---------");
	}
	

	displayChangedUnit();
	delay(200);
}

void receiveEvent(int numBytes)
{
	Serial.print("howMany: ");
	Serial.println(numBytes);
	command = Wire.read(); // receive byte as a character
	Serial.print("command: ");
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
		Serial.print("buffer: ");
		Serial.println(buffer);
		switch (command)
		{
		case 'a':
			readActuators(buffer, index);
			break;
		case 'u':
			readUnits(buffer, index);
		default:
			break;
		}
	}
}

void requestEvent()
{
	char buffer[I2C_BUFFER_LENGHT];
	size_t index{};
	switch (command)
	{

	case 'b':
		updateUnits(buffer, index);
		sendResponse(buffer);
	default:
		break;
	}
}

void sendResponse(char buffer[])
{
	bool endOfResponse = false;
	for (size_t i = 0; i < I2C_BUFFER_LENGHT; i++)
	{
		char ch = buffer[i];
		if (ch == '\0')
		{
			endOfResponse = true;
			// Serial.println(i);
		}
		if (endOfResponse)
		{
			Wire.write('\0');
		}
		else
		{
			Wire.write(ch);
			Serial.print(ch);
		}
	}
	Serial.println();
}

void changeLightStatus(const String &lightStatus)
{
	if (lightStatus == "on")
	{
		digitalWrite(lightPin, HIGH);
		// Serial.println("Light is On..!");
	}
	if (lightStatus == "off")
	{
		digitalWrite(lightPin, LOW);
		// Serial.println("Light is Off..!");
	}
}

void changeAlarmStatus(const String &alarmStatus)
{
	if (alarmStatus == "on")
	{
		digitalWrite(alarmPin, HIGH);
		// Serial.println("Alarm is On..!");
	}
	if (alarmStatus == "off")
	{
		digitalWrite(alarmPin, LOW);
		// Serial.println("Alarm is Off..!");
	}
}

void readActuators(char buffer[], size_t bufferSize)
{
	const size_t capacity = 200;
	DynamicJsonDocument responseDoc(capacity);
	// Parse JSON object
	DeserializationError error = deserializeJson(responseDoc, buffer, bufferSize);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	// Extract values
	String lightStatus{responseDoc["led"][0]["status"].as<char *>()};
	String alarmStatus{responseDoc["led"][0]["alarm"].as<char *>()};
	// Serial.println(responseDoc["time"].as<long>());
	// Serial.println(responseDoc["data"][0].as<float>(), 6);
	Serial.print("light status:");
	Serial.println(lightStatus);
	changeLightStatus(lightStatus);
	changeAlarmStatus(alarmStatus);
}

void readUnits(char buffer[], size_t bufferSize)
{
	const size_t capacity = 4*(JSON_ARRAY_SIZE(8) + 9 * JSON_OBJECT_SIZE(2) + 136);
	DynamicJsonDocument doc(capacity);

	// Parse JSON object
	DeserializationError error = deserializeJson(doc, buffer, bufferSize);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	for (size_t i = 0; i < numberOfUnits; i++)
	{
		int id{doc["units"][i]["id"].as<int>()};
		String name{doc["units"][i]["name"].as<char *>()};
		long price{doc["units"][i]["price"].as<long>()};
		String msg = String("id: ") + String(id) + String("   name :") + name + String("   price :") + price;
		Serial.println(msg);
		unitsId[i] = id;
		unitsProductName[i] = name;
		unitsProductPrice[i] = price;
	}
}

void updateUnits(char buffer[], const size_t &size)
{
	// 	float h = 35;
	// 	float t = 22;
	// 	float b = analogRead(A0);
	// 	if (isnan(h) || isnan(t) || isnan(b))
	// 	{
	// 		Serial.println("Failed to read from DHT sensor!");
	// 		return;
	// 	}
	StaticJsonDocument<200> unitsDoc;
	JsonArray unitsArr = unitsDoc.createNestedArray("units");
	for (size_t i = 0; i < 3; i++)
	{
		bool status = unitsLastStatus[i];
		const size_t CAPACITY = 50;
		StaticJsonDocument<CAPACITY> productDoc;
		JsonObject productObj = productDoc.to<JsonObject>();
		productObj["id"] = i + 1;
		productObj["status"] = (status == HIGH ? "on" : "off");
		unitsArr.add(productObj);
	}
	serializeJson(unitsDoc, buffer, I2C_BUFFER_LENGHT);
}

int detectChangedUnit()
{
	int unitIndex{}; // + for putting down and - for picking up
	bool unitsStatus[3];
	for (size_t i = 0; i < 3; i++)
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

	Serial.print("unit ");
	Serial.print(unitId);
	Serial.print(" -> ");
	Serial.print(unitsProductName[unitId - 1]);
	Serial.print(" was ");
	Serial.println(pickedUp ? " picked up" : " put down");
	if(pickedUp)
	{
		lcd.clear();
		lcd.print("unit ");
		lcd.print(unitId);
		lcd.print(" -> ");
		lcd.print(unitsProductName[unitId - 1]);
		lcd.setCursor(0, 1);
		lcd.print("Price: ");
		lcd.print(unitsProductPrice[unitId - 1]);
	}
}