#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

#define SDA_PIN D1
#define SCL_PIN D2
const uint8_t I2C_MASTER = 0x42;
const uint8_t I2C_SLAVE = 0x08;
const uint8_t I2C_BUFFER_LENGHT = 250;

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
const unsigned long postingInterval = 5L * 1000L; // delay between updates, in milliseconds

const int ledPin = D4;

boolean connectWifi(); // connect to wifi – returns true if successful or false if not
String httpCommunication(const String &uri, const String &body = "");
void handleWireResponse(char buffer[]);
void readActuators();
void readUnits();
void updateUnits();

void setup()
{
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

		readUnits();
		delay(20);
		readActuators();
		delay(20);
		updateUnits();

		// Wire.beginTransmission(I2C_SLAVE);
		// Wire.write("mizanamet kart beshehamash khoftan!\n");
		// Wire.endTransmission();
		

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

void readActuators()
{
	String uri{"/api/read_actuators.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;
	char buffer[I2C_BUFFER_LENGHT];
	response.toCharArray(buffer, I2C_BUFFER_LENGHT);
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('a');
	Wire.write(buffer);
	Wire.endTransmission();
}

void readUnits()
{
	const String uri{"/api/read_units1.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;
	char buffer[I2C_BUFFER_LENGHT];
	response.toCharArray(buffer, I2C_BUFFER_LENGHT);
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('u');
	Wire.write(buffer);
	Wire.endTransmission();
}

void updateUnits()
{
	Wire.beginTransmission(I2C_SLAVE);
	Wire.write('b');
	Wire.endTransmission();
	Wire.requestFrom(I2C_SLAVE, I2C_BUFFER_LENGHT);
	char buffer[I2C_BUFFER_LENGHT];
	handleWireResponse(buffer);
	String uri{"/api/update_units.php"};
	String response = httpCommunication(uri, buffer);

	if (response == "")
	return;
}