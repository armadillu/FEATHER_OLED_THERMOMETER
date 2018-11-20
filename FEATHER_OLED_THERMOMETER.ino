#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DisplaySetup.h"

//temp sensor 
#include "DHT.h"
#define DHTPIN 14     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// GLOBALS /////////////////////////////////////////////////////////////////////////////////

float tempCelcius = 0.0f;
float humidity = 0.0f;

//global devices
Adafruit_SSD1306 display = Adafruit_SSD1306();
DHT dht(DHTPIN, DHTTYPE);

// WIFI ////////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "WifiPass.h" //define wifi SSID & pass
//const char* ssid = "XXXXX";
//const char* password = "XXXXX";

ESP8266WebServer server(80);

void handleRoot() {
	digitalWrite(LED_BUILTIN, LOW); //note on the huzza LED low is high!
	static char json[512];
	sprintf(json, "{\"temperature\":%f, \"humidity\":%f}", tempCelcius, humidity);
	server.send(200, "application/json", json);
	digitalWrite(LED_BUILTIN, HIGH);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

	pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
	digitalWrite(LED_BUILTIN, HIGH);

	Serial.begin(115200);
	Serial.println("Temperature Test");
	Serial.println("----------------------------------------------\n");

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);	// initialize with the I2C addr 0x3C (for the 128x32)
	Serial.println("OLED setup");

	// Clear the buffer.
	display.display();
	delay(1000);
	display.clearDisplay();
	display.display();
	Serial.println("Display cleared");

	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.println("Starting up...");
	display.display(); // actually display all of the above

	dht.begin();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) { // Wait for connection
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin("tempSensor")) {
		Serial.println("MDNS responder started");
	}

	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);

	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");

}


void loop() {

	delay(100);

	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humidity = dht.readHumidity();
	tempCelcius = dht.readTemperature();

	updateDisplay();

	server.handleClient();
}


void updateDisplay(){
	
	float textSize = 2;
	
	display.clearDisplay();
	static char msg[2][40];
	display.setTextSize(textSize);
	display.setTextColor(WHITE);

	// Check if any reads failed and exit early (to try again).
	if (isnan(humidity) || isnan(tempCelcius) ) {
		sprintf(msg[0], "Temp err!");
		sprintf(msg[1], "Hum err!");
	} else {
		sprintf(msg[0], "T: %0.1fc", tempCelcius);
		sprintf(msg[1], "H: %0.1f%%", humidity);
	}

	//print Temp and H (2 lines)
	int y = 0;
	display.setCursor(0, y);
	display.print(msg[0]);
	y += 9 * textSize;
	display.setCursor(0, y); 
	display.print(msg[1]);
	display.display(); // actually display all of the above

}
