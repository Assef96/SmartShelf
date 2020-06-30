#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

#define SDA_PIN D1
#define SCL_PIN D2
const int I2C_MASTER = 0x42;
const int I2C_SLAVE = 0x20;
const int I2C_BUFFER_LENGHT = 250;
char command;

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
const unsigned long postingInterval = 3L * 1000L; // delay between updates, in milliseconds

char readActuatorsBuffer[I2C_BUFFER_LENGHT];
// char readUnitsBuffer[I2C_BUFFER_LENGHT];
char updateUnitsBuffer[I2C_BUFFER_LENGHT];

const int ledPin = D4;
bool ledStatus;

boolean connectWifi(); // connect to wifi – returns true if successful or false if not
String httpCommunication(const String &uri, const String &body = "");
void readActuators();
// void readUnits();
// void updateUnits();
void receiveEvent(int numBytes);
void requestEvent();
void sendResponse(char buffer[]);

void setup()
{
	Serial.begin(115200);
	Wire.begin(SDA_PIN, SCL_PIN, I2C_SLAVE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);
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

		// readUnits();
		// delay(20);
		readActuators();
		// delay(20);
		// updateUnits();

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

void readActuators()
{
	String uri{"/api/read_actuators.php"};
	String response = httpCommunication(uri);
	if (response == "")
		return;
	response.toCharArray(readActuatorsBuffer, I2C_BUFFER_LENGHT);
}

// void readUnits()
// {
// 	const String uri{"/api/read_units1.php"};
// 	String response = httpCommunication(uri);
// 	if (response == "")
// 		return;
// 	response.toCharArray(readActuatorsBuffer, I2C_BUFFER_LENGHT);
// }

// void updateUnits()
// {
// 	String uri{"/api/update_units.php"};
// 	String response = httpCommunication(uri, updateUnitsBuffer);

// 	if (response == "")
// 	return;
// }

void receiveEvent(int numBytes)
{
	Serial.println(F("Incoming Event"));
	Serial.print("howMany: ");
	Serial.println(numBytes);
	command = Wire.read(); // receive byte as a character
	Serial.print(F("command: "));
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
		Serial.print(F("buffer: "));
		Serial.println(buffer);
		switch (command)
		{
		case '2':
			// updateUnitsBuffer = buffer;
			break;
		default:
			break;
		}
	}
}

void requestEvent()
{
	Serial.println(F("Incoming Request"));
	// char buffer[I2C_BUFFER_LENGHT];
	// size_t index{};
	switch (command)
	{

	case '1':
		sendResponse(readActuatorsBuffer);
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