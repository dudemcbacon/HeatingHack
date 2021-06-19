#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BME280
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

#define RED_LED_PIN 0
#define GREEN_LED_PIN 16
#define SDA_PIN 4
#define RELAY_PIN 10

#define DEFAULT_TEMP_BUFFER 5

void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps
  Serial.println("Hello world!");

  //// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //// Show initial display buffer contents on the screen --
  //// the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  //// Clear the buffer
  display.clearDisplay();

  //// Draw a single pixel in white
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(30, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(F("poop"));

  //// Show the display buffer on the screen. You MUST call display() after
  //// drawing commands to make them visible on screen!
  display.display();

  unsigned status;

  // default settings
  status = bme.begin();
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }

  // put your setup code here, to run once:
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // default state
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
}

float convertToF(float celsius) {
 return celsius * 1.8 + 32;
}

float calculateDewpoint(float temperature, float humidity){
{
  float a = 17.271;
  float b = 237.7;
  float temp = (a * temperature) / (b + temperature) + log((double) humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}
}

void printValues(float temp, float humidity, float dewpoint, float buffer) {
    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner

    display.print("Temp: ");
    display.print(convertToF(temp));
    display.println(" F");

    display.print("Pres: ");

    display.print(bme.readPressure() / 100.0F);
    display.println(" hPa");

    display.print("Alt: ");
    display.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    display.println(" m");

    display.print("Hum: ");
    display.print(humidity);
    display.println(" %");

    display.print("DP: ");
    display.print(convertToF(dewpoint));
    display.println(" F");

    display.print("Buffer: ");
    display.print(buffer);
    display.println(" F");

    display.display();
}

void loop() {
  Serial.println("foo");


  float temp = bme.readTemperature();
  float humidity = bme.readHumidity();
  float dewpoint = calculateDewpoint(temp, humidity);
  float buffer = convertToF(dewpoint) + DEFAULT_TEMP_BUFFER;

  Serial.print(buffer);
  Serial.print(" > ");
  Serial.print(temp);

  if (buffer > convertToF(temp)) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  } else {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  }

  printValues(temp, humidity, dewpoint, buffer);
  delay(1000);
}
