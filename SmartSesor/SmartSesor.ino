#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int valA;
int valB;
int valC;
uint8_t Btn_SELECT = D5;

int buttonState = LOW; //this variable tracks the state of the button, low if not pressed, high if pressed
int ledState = -1; //this variable tracks the state of the LED, negative if off, positive if on

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flick

byte clk;
byte menuCount   = 1;
byte dir = 0;
bool runState = false;



void setup() {
  Serial.begin(115200);
  pinMode(Btn_SELECT, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.display();

}

void loop() {
  display.clearDisplay();
  float mainTemp = random(18, 25);
  float decimalTemp = random(0, 99) / 100.0;
  float temp = mainTemp + decimalTemp;
  menuCheck();
  staticMenu();
  delay(200);
}




void staticMenu() {

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(10, 0);
  display.println("Select WIFI");
  //---------------------------------
  display.setTextSize(1);
  display.setCursor(10,   20);
  display.println("Value A:");
  display.setCursor(60, 20);
  display.println(valA);

  display.setCursor(10, 30);
  display.println("Value B:");
  display.setCursor(60,   30);
  display.println(valB);

  display.setCursor(10, 40);
  display.println("Value   C:");
  display.setCursor(60, 40);
  display.println(valC);

  display.setCursor(10,   50);
  display.println("Start:");
  display.setCursor(45, 50);

  if (menuCount > 4){
    menuCount = 1;
    }

  display.setCursor(2, (menuCount * 10) + 10);
  display.println(">");

  display.display();
}

void menuCheck() {

  Serial.println(menuCount);
  buttonState = digitalRead(Btn_SELECT);

  if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
    if ( (buttonState == HIGH) && (ledState < 0) ) {

      menuCount = menuCount + 1; //turn LED on
      ledState = -ledState; //now the LED is on, we need to change the state
      lastDebounceTime = millis(); //set the current time
    }
    else if ( (buttonState == HIGH) && (ledState > 0) ) {

      menuCount = menuCount + 1; //turn LED off
      ledState = -ledState; //now the LED is off, we need to change the state
      lastDebounceTime = millis(); //set the current time
    }

  }//close if(time buffer)


}


void displayTemp(float temp) {
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("      Temperature    ");
  display.println("---------------------");
  display.setCursor(28, 27);
  display.setTextSize(3);
  display.print(temp, 1);
  display.print((char)247);
  display.display();
}
