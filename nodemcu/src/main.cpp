#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define SDA_PIN D1
#define SCL_PIN D2
const uint8_t I2C_MASTER = 0x42;
const uint8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 60;

const char *ssid = "Peppsi";
const char *password = "figoliniho";
const char *host = "192.168.1.106";
// const char *ssid = "HONOR";
// const char *password = "123454321";
// const char *host = "192.168.43.97";
const uint16_t port = 80;
WiFiClient wifi;
boolean wifiConnected = false;
unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 3000L; // delay between updates, in milliseconds

DHT dht;
const int dhtPin = D3;
const int ledPin = D4;
const int lightPin = A0;
bool ledStatus;

const size_t numberOfUnits{3};
size_t unitsId[numberOfUnits];
String unitsProductName[numberOfUnits];
long unitsProductPrice[numberOfUnits];

boolean connectWifi(); // connect to wifi – returns true if successful or false if not
String httpCommunication(const String &uri, const String &body = "");
void handleWireResponse(char buffer[]);
void readCommands();
void readUnits();
void updateUnits();
void updateAmbient();

void setup()
{
	dht.setup(dhtPin); // data pin 2
	Serial.begin(115200);
	Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER); // join i2c bus (address optional for master)
	pinMode(ledPin, OUTPUT);
	wifiConnected = connectWifi();
}

void loop()
{
	// if (!wifiConnected)
	// 	return;

	if (millis() - lastConnectionTime > postingInterval)
	{
		lastConnectionTime = millis();
    	ledStatus = !ledStatus;
		digitalWrite(ledPin, ledStatus);

		readUnits();
		delay(20);
		readCommands();
		delay(20);
		updateUnits();
		delay(20);
		updateAmbient();
		
		delay(20);
		Serial.println("--------- End of Loop ---------");
	}
}

boolean connectWifi()
{
	boolean state = true;
	int i = 0;

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.println("");
	Serial.println("Connecting to WiFi");

	// Wait for connection
	Serial.print("Connecting ...");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		if (i > 100)
		{
			state = false;
			break;
		}
		i++;
	}

	if (state)
	{
		Serial.print("Connected to: ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}
	else
	{
		Serial.println("");
		Serial.println("Connection failed.");
	}

	return state;
}

String httpCommunication(const String &uri, const String &body)
{
	HTTPClient http;
	int httpCode;
	String response;
	Serial.printf("Connecting to: %s%s\n", host, uri.c_str());
	http.begin(wifi, host, port, uri);

	if (body == "") // Send Get Request
	{
		httpCode = http.GET();
		Serial.print("Get -> ");
	}
	else // Send Post Request
	{
		http.addHeader("Content-Type", "application/json");
		httpCode = http.POST(body);
		Serial.print("Post -> ");
	}

	if (httpCode > 0) // HTTP header has been send and Server response header has been handled
	{
		Serial.printf("code: %d\n", httpCode);
		if (httpCode == HTTP_CODE_OK) // file found at server
		{
			response = http.getString();
			Serial.println("response:");
			Serial.println(response);
		}
	}
	else
		Serial.printf("failed -> %s\n", http.errorToString(httpCode).c_str());
	http.end();
	return response;
}

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

void readUnits()
{
	Serial.println(">>>>>>>>> Read Units ");
	const String uri{"/api/read_units1.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;

	const size_t capacity = 512;
	StaticJsonDocument<capacity> doc;

	DeserializationError error = deserializeJson(doc, response);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	for (size_t i = 0; i < numberOfUnits; i++)
	{
		Wire.beginTransmission(I2C_SLAVE);
		Wire.write('p');
		serializeJson(doc["units"][i], Wire);
		Wire.endTransmission();
	}

	// for (size_t i = 0; i < numberOfUnits; i++)
	// {
	// 	int id{doc["units"][i]["id"].as<int>()};
	// 	String name{doc["units"][i]["name"].as<char *>()};
	// 	long price{doc["units"][i]["price"].as<long>()};
	// 	// String msg = String("id: ") + String(id) + String("   name :") + name + String("   price :") + price;
	// 	// Serial.println(msg);
		
	// 	if(name != unitsProductName[i] || price!= unitsProductPrice[i])
	// 	{
	// 		Serial.print(name);
	// 		Serial.println(" was changed");
	// 		unitsId[i] = id;
	// 		unitsProductName[i] = name;
	// 		unitsProductPrice[i] = price;

	// 		Wire.beginTransmission(I2C_SLAVE);
	// 		Wire.write('p');
	// 		serializeJson(doc["units"][i], Wire);
	// 		Wire.endTransmission();
	// 	}
	// }
	Serial.println(" Read Units <<<<<<<<<");
}

void readCommands()
{
	Serial.println(">>>>>>>>> Read Commands ");
	String uri{"/api/read_commands.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;

	const size_t capacity = 200;
	DynamicJsonDocument responseDoc(capacity);
	// Parse JSON object
	DeserializationError error = deserializeJson(responseDoc, response);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	// Extract values
	String lamp1Status{responseDoc["commands"]["lamp1"].as<char *>()};
	String lamp2Status{responseDoc["commands"]["lamp2"].as<char *>()};
	String buzzerStatus{responseDoc["commands"]["buzzer"].as<char *>()};
	// Serial.println(responseDoc["time"].as<long>());
	// Serial.println(responseDoc["data"][0].as<float>(), 6);

	const size_t capacity2 = JSON_OBJECT_SIZE(9) + 50;
	DynamicJsonDocument doc(capacity2);
	doc["l1"] = lamp1Status == "1" ? 1 : 0;
	doc["l2"] = lamp2Status == "1" ? 1 : 0;
	doc["b"] = buzzerStatus == "1" ? 1 : 0;
	serializeJson(doc, Serial);
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('c');
	serializeJson(doc, Wire);
	Wire.endTransmission();
	Serial.println(" Read Commands <<<<<<<<<");
}

void updateUnits()
{
	Serial.println(">>>>>>>>> Update Units ");
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('u');
	Wire.endTransmission();
	Wire.requestFrom(I2C_SLAVE, I2C_BUFFER_LENGHT);
	char buffer[I2C_BUFFER_LENGHT];
	handleWireResponse(buffer);
	String uri{"/api/update_units.php"};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
	Serial.println(" Update Units <<<<<<<<<");
}


void updateAmbient()
{
	Serial.println(">>>>>>>>> Update Ambient ");
	// Serial.println(dht.getMinimumSamplingPeriod());

	float humidity = dht.getHumidity();;
	float temperature = dht.getTemperature();
	String status = dht.getStatusString();
	if(status != "OK")
	{
		Serial.print(F("Error while reading DHT sensor. status: "));
		Serial.println(status);
		return;
	}
	int light = analogRead(lightPin) / 10;
	// outputValue = map(sensorValue, 0, 1023, 0, 255);

	char buffer[100];

	const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
	DynamicJsonDocument doc(capacity);
	doc["temperature"] = temperature;
	doc["humidity"] = humidity;
	doc["light"] = light;
	serializeJson(doc, buffer, 100);

	String uri{"/api/update_ambient.php"};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
	Serial.println(" Update Ambient <<<<<<<<<");
}
