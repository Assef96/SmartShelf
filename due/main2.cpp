#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#define Serial SerialUSB

//  SDA_PIN A4 and SCL_PIN A5
const int8_t I2C_MASTER = 0x42;
const int8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 250;
char command;

unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 2L * 1000L; // delay between updates, in milliseconds

const int ledPin = LED_BUILTIN;
bool ledStatus = HIGH;
const int alarmPin = 12;
const int unitsPin[3] = {5, 6, 7};
bool unitsPinLastStatus[3];
const size_t numberOfUnits{3};
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];

void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);
void changeLedStatus(const String &ledStatus);
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

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	for (size_t i = 0; i < 3; i++)
	{
		pinMode(unitsPin[i], INPUT);
	}
	
	pinMode(ledPin, OUTPUT);
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
	delay(20);
}

void receiveEvent(int numBytes)
{
	Serial.print("howMany: ");
	Serial.println(numBytes);
	command = Wire.read(); // receive byte as a character
	Serial.print("command: ");
	Serial.println(numBytes);

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
	Serial.println(buffer);
	bool endOfResponse = false;
	for (size_t i = 0; i < I2C_BUFFER_LENGHT; i++)
	{
		char ch = buffer[i];
		if (ch == '\0')
		{
			endOfResponse = true;
			Serial.println(i);
		}
		if (endOfResponse)
		{
			Wire.write('\0');
		}
		else
		{
			Wire.write(ch);
		}
	}
}

void changeLedStatus(const String &ledStatus)
{
	if (ledStatus == "on")
	{
		digitalWrite(ledPin, HIGH);
		// Serial.println("LED is On..!");
	}
	if (ledStatus == "off")
	{
		digitalWrite(ledPin, LOW);
		// Serial.println("LED is Off..!");
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
	String ledStatus{responseDoc["led"][0]["status"].as<char *>()};
	String alarmStatus{responseDoc["led"][0]["alarm"].as<char *>()};
	// Serial.println(responseDoc["time"].as<long>());
	// Serial.println(responseDoc["data"][0].as<float>(), 6);
	Serial.print("led status:");
	Serial.println(ledStatus);
	changeLedStatus(ledStatus);
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
		int id{doc["shelf"][i]["id"].as<int>()};
		String name{doc["shelf"][i]["name"].as<char *>()};
		// long price{doc["products"][i]["price"].as<long>()};
		// String image{doc["products"][i]["image"].as<char *>()};
		String msg = String("id: ") + String(id) + String("   name :") + name;
		Serial.println(msg);
		unitsId[i] = id;
		unitsProductName[i] = name;
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
	Serial.println("update units.");
	StaticJsonDocument<200> unitsDoc;
	JsonArray unitsArr = unitsDoc.createNestedArray("units");
	for (size_t i = 0; i < 3; i++)
	{
		bool status = digitalRead(unitsPin[i]);
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
	for (size_t i = 0; i < 3; i++)
	{
		if (unitsPinLastStatus[i] > digitalRead(unitsPin[i])) // picked up
		{
			unitsPinLastStatus[i] = digitalRead(unitsPin[i]);
			unitIndex = -(i + 1);
			break;
		}
		else if (unitsPinLastStatus[i] < digitalRead(unitsPin[i])) // put down
		{
			unitsPinLastStatus[i] = digitalRead(unitsPin[i]);
			unitIndex = +(i + 1);
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
}
