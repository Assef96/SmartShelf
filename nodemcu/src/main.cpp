/////////////////// Libraries: /////////////////////

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>

////////////////// Global Objects and Variables: //////////////////////

const bool debug = false; 
WiFiClient wifi;
boolean wifiConnected = false;
unsigned long lastLoopTime = 0;			  // last time you connected to the server, in milliseconds
const unsigned long loopPeriod = 100; // delay between updates, in milliseconds
const size_t numberOfUnits{5};
DHT dht;
int lightIntensity;
bool ledStatus;

const uint8_t i2cMaster = 0x42;
const uint8_t i2cSlave = 0x08;
const uint8_t i2cBufferLength = 80;
size_t i{};

const uint16_t port = 80;
const char *ssid = "Peppsi";
const char *password = "figoliniho";
const char *host = "192.168.1.104";
// const char *ssid = "HONOR";
// const char *password = "123454321";
// const char *host = "192.168.43.97";

///////////////////// Pin Map: ////////////////////
const int ledPin = D0;
const int switch1Pin = D1;
const int switch2Pin = D2;
const int switch3Pin = D3;
const int switch4Pin = D4;
const int dhtPin = D5;
const int sdaPin = D6;
const int sclPin = D7;
const int lightPin = A0;

///////////////////// Function Definition: /////////////////////

boolean connectWifi(); // Connects to WiFi. Returns true if successful or false if not.
String httpCommunication(const String &uri, const String &body = "");
bool WireRequest(char buffer[], const char command);
void readCommands();
void readUnitsProducts();
void readSwitches();
void updateSensors();
void insertAmbient();

void setup()
{
	Serial.begin(115200);
	Wire.begin(sdaPin, sclPin, i2cMaster);
	dht.setup(dhtPin);
	pinMode(ledPin, OUTPUT);
	pinMode(switch1Pin, INPUT_PULLUP);
	pinMode(switch2Pin, INPUT_PULLUP);
	pinMode(switch3Pin, INPUT_PULLUP);
	pinMode(switch4Pin, INPUT_PULLUP);
	wifiConnected = connectWifi();
}

void loop()
{
	if (millis() - lastLoopTime > loopPeriod)
	{
		lastLoopTime = millis();
    	ledStatus = !ledStatus;
		digitalWrite(ledPin, ledStatus);

		if (wifiConnected)
		{
			i++;
			if(i == 20)
			{
				readUnitsProducts();
				delay(100);
				i = 0;
			}
			readCommands();
			delay(50);
			updateSensors();
			delay(50);
			insertAmbient();
			delay(50);
		}

		Serial.println(F("--------- End of Loop ---------"));
	}
	readSwitches();
	delay(50);
	
}

boolean connectWifi()
{
	boolean status = true;
	int i = 0;

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.println("");
	Serial.println(F("Connecting to WiFi"));

	// Wait for connection
	Serial.print(F("Connecting ..."));
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(F("."));
		if (i > 30)
		{
			status = false;
			break;
		}
		i++;
	}
	Serial.println("");
	if (status)
	{
		Serial.print(F("Connected to: "));
		Serial.println(ssid);
		Serial.print(F("IP address: "));
		Serial.println(WiFi.localIP());
	}
	else
	{
		Serial.println(F("Connection failed."));
	}

	return status;
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
			Serial.print(F("Get -> "));
	}
	else // Send Post Request
	{
		http.addHeader(F("Content-Type"), F("application/json"));
		httpCode = http.POST(body);
		if (debug)
			Serial.print(F("Post -> "));
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
				Serial.println(F("response:"));
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
	Wire.beginTransmission(i2cSlave);
	Wire.write(command);
	Wire.endTransmission();
	Wire.requestFrom(i2cSlave, i2cBufferLength);
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
		Serial.print(F("buffer: "));
		Serial.println(buffer);
	}
	return success;
}

void readUnitsProducts()
{
	if(debug)
		Serial.println(F(">>>>>>>>> Read UnitsProducts "));
	const String uri{F("/api/read_units_products_hardware.php")};
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

	for (size_t i = 0; i < numberOfUnits; i++)
	{
		Wire.beginTransmission(i2cSlave);
		Wire.write('p');
		serializeJson(doc[F("units")][i], Wire);
		Wire.endTransmission();
	}

	if(debug)
		Serial.println(F(" Read UnitsProducts <<<<<<<<<"));
}

void readSwitches()
{
	if(debug)
		Serial.println(F(">>>>>>>>> Read Switches "));

	const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
	DynamicJsonDocument doc(capacity);
	char buffer[i2cBufferLength];
	doc["a"] = int(!digitalRead(switch1Pin));
	doc["b"] = int(!digitalRead(switch2Pin));
	doc["c"] = int(!digitalRead(switch3Pin));
	doc["d"] = int(!digitalRead(switch4Pin));
	lightIntensity = analogRead(lightPin);
	doc["p"] = lightIntensity;
	serializeJson(doc, buffer, 100);

	Wire.beginTransmission(i2cSlave);
	Wire.write('s');
	serializeJson(doc, Wire);
	Wire.endTransmission();

	if(debug)
		Serial.println(F(" Read Switches <<<<<<<<<"));
}

void readCommands()
{
	if(debug)
		Serial.println(F(">>>>>>>>> Read Commands "));
	String uri{F("/api/read_commands.php")};
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
	String lampStatus{responseDoc["commands"]["lamp"].as<char *>()};
	String fanStatus{responseDoc["commands"]["fan"].as<char *>()};
	String buzzerStatus{responseDoc["commands"]["buzzer"].as<char *>()};

	const size_t capacity2 = JSON_OBJECT_SIZE(9) + 50;
	DynamicJsonDocument doc(capacity2);
	doc["l"] = lampStatus == "1" ? 1 : 0;
	doc["f"] = fanStatus == "1" ? 1 : 0;
	doc["b"] = buzzerStatus == "1" ? 1 : 0;
	Wire.beginTransmission(i2cSlave);
	Wire.write('c');
	serializeJson(doc, Wire);
	Wire.endTransmission();
	if(debug)
		Serial.println(F(" Read Commands <<<<<<<<<"));
}

void updateSensors()
{
	if(debug)
		Serial.println(F(">>>>>>>>> Update Sensors "));
	char buffer[i2cBufferLength];
	if(!WireRequest(buffer, 'u'))
	{
		Serial.println(F("Error getting information from Arduino"));
		return;
	}
	String uri{F("/api/update_sensors.php")};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
	if(debug)
		Serial.println(F(" Update Sensors <<<<<<<<<"));
}

void insertAmbient()
{
	if(debug)
		Serial.println(F(">>>>>>>>> Insert Ambient "));
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
	
	int light = analogRead(lightPin);
	int lightInt = map(light, 0, 1023, 10, -20);
	Serial.print("light: ");
	Serial.println(light);
	Serial.print("lightInt: ");
	Serial.println(lightInt);

	char buffer[100];

	const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
	DynamicJsonDocument doc(capacity);
	doc["temperature"] = temperature;
	doc["humidity"] = humidity;
	doc["light"] = lightInt;
	serializeJson(doc, buffer, 100);

	String uri{F("/api/insert_ambient.php")};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
	if(debug)
		Serial.println(F(" Insert Ambient <<<<<<<<<"));
}
