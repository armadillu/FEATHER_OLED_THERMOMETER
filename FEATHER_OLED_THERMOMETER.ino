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
static int count = 0;
int countMax = 15; //min - interval to send temp & humidity http updates to server


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

// HTTP /////////////////////////////////////////////////////////////////////////////////////

#include <ESP8266HTTPClient.h>

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

	updateSensorData();
	sendHttpData();
	updateDisplay();
}


void loop() {


	delay(1000); //once per second

	updateSensorData();
	updateDisplay();
	server.handleClient();

	count++;
	if (count >= countMax * 60 || count == 0) { //every ~30 minutes, ping server with data
		count = 1;
		sendHttpData();
	}
}

void updateSensorData(){
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humidity = dht.readHumidity();
	tempCelcius = dht.readTemperature();
}

void sendHttpData() {

	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.println("Send http...");
	display.display(); // actually display all of the above

	bool ok = false;
	HTTPClient http;
	char url[255];
	sprintf(url, "http://uri.cat/thermometer/rx.php?temp=%.1f&hum=%.1f", tempCelcius, humidity);
	//Serial.println(url);
	http.begin(url);
	int httpCode = http.GET();

	// httpCode will be negative on error
	if (httpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		Serial.print("http code: ");
		Serial.println(httpCode);

		// file found at server
		if (httpCode == HTTP_CODE_OK) {
			String payload = http.getString();
			Serial.print("Response: ");
			Serial.println(payload);
			ok = true;
		}
	} else {
		Serial.print("[HTTP] GET... failed, error: ");
		Serial.println(http.errorToString(httpCode).c_str());
	}

	http.end();

	display.setCursor(0, 16);
	char msg[64];
	sprintf(msg, "response: %s", ok ? "ok" : "ko!");
	display.println(msg);
	display.display(); // actually display all of the above
}


void updateDisplay() {

	float textSize = 1;

	display.clearDisplay();
	static char msg[3][40];
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
	int sec = countMax * 60 - count;
	int min = sec / 60;
	int sec60 = sec % 60;
	if(min > 0 )
		sprintf(msg[2], "Next TX: %dmin %dsec", min, sec60 );
	else
		sprintf(msg[2], "Next TX: %dsec", sec60  );
	
	//print Temp and H (2 lines)
	int y = 0;
	display.setCursor(0, y);
	display.print(msg[0]);
	y += 9 * textSize;
	display.setCursor(0, y);
	display.print(msg[1]);
	y += 14 * textSize;
	display.setCursor(0, y);
	display.print(msg[2]);
	display.display(); // actually display all of the above

}
