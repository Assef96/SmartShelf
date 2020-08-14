#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "DHT.h"


const uint8_t I2C_MASTER = 0x42;
const uint8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 80;

const char *ssid = "Peppsi";
const char *password = "figoliniho";
const char *host = "192.168.1.104";
// const char *ssid = "HONOR";
// const char *password = "123454321";
// const char *host = "192.168.43.97";
const uint16_t port = 80;
WiFiClient wifi;
boolean wifiConnected = false;
unsigned long lastConnectionTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 200L; // delay between updates, in milliseconds

DHT dht;
const int lightPin = A0;
const int ledPin = D0;
const int switch1Pin = D1;
const int switch2Pin = D2;
const int switch3Pin = D3;
const int switch4Pin = D4;
const int dhtPin = D5;
#define SDA_PIN D6
#define SCL_PIN D7
bool ledStatus;

const bool debug = true; 

const size_t numberOfSensors{12};
size_t sensorsId[numberOfSensors];
String sensorsProductName[numberOfSensors];
long sensorsProductPrice[numberOfSensors];
int lightIntensity;

boolean connectWifi(); // connect to wifi – returns true if successful or false if not
String httpCommunication(const String &uri, const String &body = "");
bool WireRequest(char buffer[], const char command);
void readCommands();
void readSensorsProducts();
void readSwitches();
void updateSensors();
void updateAmbient();

void setup()
{
	dht.setup(dhtPin); // data pin 2
	Serial.begin(115200);
	Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER); // join i2c bus (address optional for master)
	pinMode(ledPin, OUTPUT);
	pinMode(switch1Pin, INPUT_PULLUP);
	pinMode(switch2Pin, INPUT_PULLUP);
	pinMode(switch3Pin, INPUT_PULLUP);
	pinMode(switch4Pin, INPUT_PULLUP);
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

		readSensorsProducts();
		delay(100);
		readCommands();
		delay(100);
		updateSensors();
		delay(100);
		updateAmbient();
		delay(100);
		readSwitches();
		delay(100);

		Serial.println("--------- End of Loop ---------");
	}
	readSwitches();
	delay(100);
	
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
		if (i > 30)
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
	if(debug)
		Serial.printf("Connecting to: %s%s\n", host, uri.c_str());
	http.begin(wifi, host, port, uri);

	if (body == "") // Send Get Request
	{
		httpCode = http.GET();
		if (debug)
			Serial.print("Get -> ");
	}
	else // Send Post Request
	{
		http.addHeader("Content-Type", "application/json");
		httpCode = http.POST(body);
		if (debug)
			Serial.print("Post -> ");
	}

	if (httpCode > 0) // HTTP header has been send and Server response header has been handled
	{
		if(debug)
			Serial.printf("code: %d\n", httpCode);
		if (httpCode == HTTP_CODE_OK) // file found at server
		{
			response = http.getString();
			if (debug)
			{
				Serial.println("response:");
				Serial.println(response);
			}
		}
	}
	else
		Serial.printf("failed -> %s\n", http.errorToString(httpCode).c_str());
	http.end();
	return response;
}

bool WireRequest(char buffer[], const char command)
{	
	bool success = false;
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write(command);
	Wire.endTransmission();
	Wire.requestFrom(I2C_SLAVE, I2C_BUFFER_LENGHT);
	size_t index{};
	while (Wire.available())
	{
		char x = Wire.read();
		buffer[index++] = x;
	}
	if(buffer[0] == '{')
		success = true;
	if(debug)
	{
		Serial.print("buffer: ");
		Serial.println(buffer);
	}
	return success;
}

void readSensorsProducts()
{
	if(debug)
		Serial.println(">>>>>>>>> Read SensorsProducts ");
	const String uri{"/api/read_sensors_products.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;

	const size_t capacity = 2048;
	StaticJsonDocument<capacity> doc;

	DeserializationError error = deserializeJson(doc, response);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	for (size_t i = 0; i < numberOfSensors; i++)
	{
		Wire.beginTransmission(I2C_SLAVE);
		Wire.write('p');
		serializeJson(doc["sensors"][i], Wire);
		Wire.endTransmission();
	}

	if(debug)
		Serial.println(" Read SensorsProducts <<<<<<<<<");
}

void readSwitches()
{
	if(debug)
		Serial.println(">>>>>>>>> Read Switches ");

	const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
	DynamicJsonDocument doc(capacity);
	char buffer[I2C_BUFFER_LENGHT];
	doc["a"] = int(!digitalRead(switch1Pin));
	doc["b"] = int(!digitalRead(switch2Pin));
	doc["c"] = int(!digitalRead(switch3Pin));
	doc["d"] = int(!digitalRead(switch4Pin));
	lightIntensity = analogRead(lightPin);
	doc["p"] = lightIntensity;
	serializeJson(doc, buffer, 100);

	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('s');
	serializeJson(doc, Wire);
	Wire.endTransmission();

	if(debug)
		Serial.println(" Read Switches <<<<<<<<<");
}

void readCommands()
{
	if(debug)
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
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('c');
	serializeJson(doc, Wire);
	Wire.endTransmission();
	if(debug)
		Serial.println(" Read Commands <<<<<<<<<");
}

void updateSensors()
{
	if(debug)
		Serial.println(">>>>>>>>> Update Sensors ");
	char buffer[I2C_BUFFER_LENGHT];
	if(!WireRequest(buffer, 'u'))
	{
		Serial.println("Error getting information from Arduino");
		return;
	}
	String uri{"/api/update_sensors.php"};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
	if(debug)
		Serial.println(" Update Sensors <<<<<<<<<");
}

void updateAmbient()
{
	if(debug)
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
	if(debug)
		Serial.println(" Update Ambient <<<<<<<<<");
}
