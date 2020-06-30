#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#define Serial SerialUSB
// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
// LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2); // Change to (0x27,16,2) for 16x2 LCD.

//  SDA_PIN A4 and SCL_PIN A5
const int I2C_MASTER = 0x42;
const int I2C_SLAVE = 0x20;
const int I2C_BUFFER_LENGHT = 250;

unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 2L * 1000L; // delay between updates, in milliseconds

bool ledStatus;
const int ledPin = LED_BUILTIN;
const int lightPin = 22;
const int alarmPin = 12;
const int unitsPin[3] = {8, 9, 10};
const int photoPin = A0;
bool photoPinLastStatus;
bool unitsLastStatus[3] = {false, false, false};
const size_t numberOfUnits{3};
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];


void changeLightStatus(const String &lightStatus);
void changeAlarmStatus(const String &alarmStatus);
void handleRequests();
void readActuators();
// void updateUnits(char buffer[], const size_t &bufferSize);
// void readUnits(char buffer[], size_t size);
int detectChangedUnit();
void displayChangedUnit();
void handleWireResponse(char buffer[]);

void setup()
{
	Serial.begin(115200);

	// // set up the LCD's number of columns and rows:
	// lcd.begin(16, 2);
	// // Print a message to the LCD.
	// lcd.print("hello, world!");

	// Wire.begin(I2C_MASTER); // join i2c bus (address optional for master)
	Wire.begin();

	for (size_t i = 0; i < 3; i++)
	{
		pinMode(unitsPin[i], INPUT);
	}
	
	pinMode(ledPin, OUTPUT);
	pinMode(lightPin, OUTPUT);
	pinMode(alarmPin, OUTPUT);
}

void loop()
{
	if (millis() - lastConnectionTime > postingInterval)
	{
		lastConnectionTime = millis();
		ledStatus = !ledStatus;
		digitalWrite(ledPin, ledStatus);

		readActuators();

		Serial.println(F("--------- End of Loop ---------"));
	}
	

	// displayChangedUnit();
	delay(200);
}



void changeLightStatus(const String &lightStatus)
{
	if (lightStatus == "on")
	{
		digitalWrite(lightPin, HIGH);
		// Serial.println(F("Light is On..!"));
	}
	if (lightStatus == "off")
	{
		digitalWrite(lightPin, LOW);
		// Serial.println(F("Light is Off..!"));
	}
}

void changeAlarmStatus(const String &alarmStatus)
{
	if (alarmStatus == "on")
	{
		digitalWrite(alarmPin, HIGH);
		// Serial.println(F("Alarm is On..!"));
	}
	if (alarmStatus == "off")
	{
		digitalWrite(alarmPin, LOW);
		// Serial.println(F("Alarm is Off..!"));
	}
}

void readActuators()
{
	Serial.println(F("reading actuators."));
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('1');
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(I2C_SLAVE, I2C_BUFFER_LENGHT);
	char buffer[I2C_BUFFER_LENGHT];
	handleWireResponse(buffer);

	// const size_t capacity = 200;
	// StaticJsonDocument<capacity> responseDoc;
	// // Parse JSON object
	// DeserializationError error = deserializeJson(responseDoc, buffer);
	// if (error)
	// {
	// 	Serial.print(F("deserializeJson() failed: "));
	// 	Serial.println(error.c_str());
	// 	return;
	// }

	// // Extract values
	// String lightStatus{responseDoc["led"][0]["status"].as<char *>()};
	// String alarmStatus{responseDoc["led"][0]["alarm"].as<char *>()};
	// // Serial.println(responseDoc["time"].as<long>());
	// // Serial.println(responseDoc["data"][0].as<float>(), 6);
	// Serial.print(F("light status:"));
	// Serial.println(lightStatus);
	// changeLightStatus(lightStatus);
	// changeAlarmStatus(alarmStatus);
}

// void readUnits(char buffer[], size_t bufferSize)
// {
// 	const size_t capacity = 4*(JSON_ARRAY_SIZE(8) + 9 * JSON_OBJECT_SIZE(2) + 136);
// 	StaticJsonDocument<capacity> doc;

// 	// Parse JSON object
// 	DeserializationError error = deserializeJson(doc, buffer, bufferSize);
// 	if (error)
// 	{
// 		Serial.print(F("deserializeJson() failed: "));
// 		Serial.println(error.c_str());
// 		return;
// 	}

// 	for (size_t i = 0; i < numberOfUnits; i++)
// 	{
// 		int id{doc["units"][i]["id"].as<int>()};
// 		String name{doc["units"][i]["name"].as<char *>()};
// 		long price{doc["units"][i]["price"].as<long>()};
// 		String msg = String("id: ") + String(id) + String("   name :") + name + String("   price :") + price;
// 		Serial.println(msg);
// 		unitsId[i] = id;
// 		unitsProductName[i] = name;
// 		unitsProductPrice[i] = price;
// 	}
// }

// void updateUnits(char buffer[], const size_t &size)
// {
// 	// 	float h = 35;
// 	// 	float t = 22;
// 	// 	float b = analogRead(A0);
// 	// 	if (isnan(h) || isnan(t) || isnan(b))
// 	// 	{
// 	// 		Serial.println(F("Failed to read from DHT sensor!"));
// 	// 		return;
// 	// 	}
// 	StaticJsonDocument<200> unitsDoc;
// 	JsonArray unitsArr = unitsDoc.createNestedArray("units");
// 	for (size_t i = 0; i < 3; i++)
// 	{
// 		bool status = unitsLastStatus[i];
// 		const size_t CAPACITY = 50;
// 		StaticJsonDocument<CAPACITY> productDoc;
// 		JsonObject productObj = productDoc.to<JsonObject>();
// 		productObj["id"] = i + 1;
// 		productObj["status"] = (status == HIGH ? "on" : "off");
// 		unitsArr.add(productObj);
// 	}
// 	serializeJson(unitsDoc, buffer, I2C_BUFFER_LENGHT);

// 	Wire.beginTransmission(I2C_SLAVE);
// 	Wire.write('2');
// 	Wire.write(buffer);
// 	Wire.endTransmission();
// }

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

	Serial.print(F("unit "));
	Serial.print(unitId);
	Serial.print(F(" -> "));
	Serial.print(unitsProductName[unitId - 1]);
	Serial.print(F(" was "));
	Serial.println(pickedUp ? " picked up" : " put down");
	if(pickedUp)
	{
		// lcd.clear();
		// lcd.print("unit ");
		// lcd.print(unitId);
		// lcd.print(" -> ");
		// lcd.print(unitsProductName[unitId - 1]);
		// lcd.setCursor(0, 1);
		// lcd.print("Price: ");
		// lcd.print(unitsProductPrice[unitId - 1]);
	}
}


// void readUnits()
// {
// 	const String uri{"/api/read_units1.php"};
// 	String response = httpCommunication(uri);
// 	if (response == "")
// 		return;
// 	char buffer[I2C_BUFFER_LENGHT];
// 	response.toCharArray(buffer, I2C_BUFFER_LENGHT);
// 	Wire.beginTransmission(I2C_SLAVE);
// 	Wire.write('u');
// 	Wire.write(buffer);
// 	Wire.endTransmission();
// }


void handleWireResponse(char buffer[])
{	
	size_t index{};
	while (Wire.available())
	{
		char x = Wire.read();
		buffer[index++] = x;
	}
	Serial.print("buffer: ");
	Serial.println(buffer);
}