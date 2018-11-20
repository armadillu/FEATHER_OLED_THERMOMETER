#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DisplaySetup.h"

#include "DHT.h"
#define DHTPIN 14     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

/////////////////////////////////////////////////////////////////////////////////////////////

Adafruit_SSD1306 display = Adafruit_SSD1306();
DHT dht(DHTPIN, DHTTYPE);

/////////////////////////////////////////////////////////////////////////////////////////////

#include "Utils.h"

float tempCelcius = 0;
float humidity = 0;

void setup(){
  
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
	display.setCursor(0,0);
	display.println("Starting up...");
	display.display(); // actually display all of the above

	dht.begin();
}


void loop() {

	delay(2000);

	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humidity = dht.readHumidity();
	tempCelcius = dht.readTemperature();

	float textSize = 2;
	
	display.clearDisplay();
	static char msg[2][40];
	display.setTextSize(textSize);
	display.setTextColor(WHITE);

	// Check if any reads failed and exit early (to try again).
	if (isnan(humidity) || isnan(tempCelcius) ) {
		sprintf(msg[0], "Temp err!");
		sprintf(msg[1], "Hum err!");
	}else{
		sprintf(msg[0], "T: %0.1fc", tempCelcius);
		sprintf(msg[1], "H: %0.1f%%", humidity);
	}

	//print 2 lines
	int y = 0;
	display.setCursor(0,y); display.print(msg[0]);
	y += 9 * textSize;
	display.setCursor(0,y); display.print(msg[1]);
	
	display.display(); // actually display all of the above
}
