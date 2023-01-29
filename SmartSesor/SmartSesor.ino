#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <ESP8266WiFi.h>

int valA;
int valB;
int valC;
uint8_t Btn_SELECT = D5;
uint8_t Btn_Enter = D6;

int buttonState = LOW; //this variable tracks the state of the button, low if not pressed, high if pressed
int ledState = -1; //this variable tracks the state of the LED, negative if off, positive if on

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 20;    // the debounce time; increase if the output flick

byte clk;
byte menuCount   = 1;
byte wifiSelector = 1;

byte dir = 0;
bool runState = false;
byte page = 0;
bool search = true;
String networks[4] = {"not found", "not found", "not found", "not found"};
#define WIFI_SSID "IZZI-DF96"
#define WIFI_PASSWORD "F0AF8551DF96"

bool wificon = false;
bool moveptr = false;
bool toggle = true;



void WifiReload() {
  search = true;
}

void WifiCheker () {
  buttonState = digitalRead(Btn_Enter);

  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if ( buttonState == HIGH) {
      if (wifiSelector == 5) {
        WifiReload();
      } else if (moveptr) {
        page = 2;
      }


      lastDebounceTime = millis(); //set the current time
    }
  }

}


void WifiSelectorCheck() {

  buttonState = digitalRead(Btn_SELECT);

  if ( (millis() - lastDebounceTime) > debounceDelay) {
    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
    if ( buttonState == HIGH) {
      wifiSelector = wifiSelector + 1; //turn LED on
      lastDebounceTime = millis(); //set the current time
    }
  }//close if(time buffer)

}

void printWifiNet() {

  for (int i = 0; i < 4; ++i)
  {
    display.setCursor(10, ((i + 1) * 10));
    display.println(networks[i]);
    Serial.println(networks[i]);
  }

  moveptr = true;
}


void ScanWifi() {
  int n = WiFi.scanNetworks();

  if (n == 0) {

    display.setCursor(10, 40);
    display.println("No Networks Found");
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.println(WiFi.SSID(i)); //SSID
      if (i < 4) {
        networks[i] = WiFi.SSID(i);
      }
    }
  }
  Serial.println("");


  search = false;

}
void Page1() {
  WifiCheker ();
  WifiSelectorCheck();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(" ----Select Wifi---- ");
  if (search) {
    ScanWifi();
  }
  printWifiNet();
  display.setCursor(10, 50);
  display.println(" < -reload->");

  if (wifiSelector > 5) {
    wifiSelector = 1;
  }

  display.setCursor(0, (wifiSelector * 10));
  display.println(" > ");
  display.display();


}

void Page2() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println("Conectando");
  display.display();
  toggle = false;
}


void mainMenu() {

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("   SMART SENSOR V1   ");
  //display.println("-------------------- -");

  display.setCursor(10,   20);
  display.println("Wifi SAVE");

  display.setCursor(10, 30);
  display.println("WIFI STREAM");

  display.setCursor(10, 40);
  display.println("SAVE SD");

  display.setCursor(10,   50);
  display.println("Config");

  if (menuCount > 4) {
    menuCount = 1;
  }

  display.setCursor(0, (menuCount * 10) + 10);
  display.println(" > ");

  display.display();
}

void menuCheck() {

  buttonState = digitalRead(Btn_SELECT);

  if ( (millis() - lastDebounceTime) > debounceDelay) {
    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
    if ( buttonState == HIGH) {
      menuCount = menuCount + 1; //turn LED on
      lastDebounceTime = millis(); //set the current time
    }
  }//close if(time buffer)

}


void enterCheck() {

  buttonState = digitalRead(Btn_Enter);

  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if ( buttonState == HIGH) {
      if (menuCount == 1) {
        page = 1;
      } else if (menuCount == 2) {
        page = 2;
      }
      lastDebounceTime = millis(); //set the current time
    }
  }
}


void displayTemp(float temp) {
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("      Temperature    ");
  display.println("-------------------- -");
  display.setCursor(28, 27);
  display.setTextSize(3);
  display.print(temp, 1);
  display.print((char)247);
  display.display();
}




void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(Btn_SELECT, INPUT);
  pinMode(Btn_Enter, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  while (toggle) {
    display.clearDisplay();
    float mainTemp = random(18, 25);
    float decimalTemp = random(0, 99) / 100.0;
    float temp = mainTemp + decimalTemp;

    if (page == 0) {
      enterCheck();
      menuCheck();
      mainMenu();
    } else if (page == 1) {
      Page1();
    } else if (page == 2) {
      Page2();
    }
    delay(100);
  }


  if (networks[wifiSelector - 1] == "IZZI-DF96") {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    display.setCursor(10,   20);
    display.println("Conectando  :)...");
    display.display();
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.println(networks[wifiSelector - 1]);
      Serial.print(".");
      display.print(".");
      display.display();
      delay(300);
    }
    display.clearDisplay();
    delay(2000);
    display.setCursor(0, 0);
    Serial.println();
    display.println("Connected with IP:  ");
    display.println(WiFi.localIP());
    display.println("RED:  ");
    display.println(networks[wifiSelector - 1]);
    display.display();
  } else {

    WiFi.begin(networks[wifiSelector - 1], "");
    display.setCursor(10,   20);
    display.println("Conectando  :)...");
    display.display();
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.println(networks[wifiSelector - 1]);
      Serial.print(".");
      display.print(".");
      display.display();
      delay(300);
    }
    display.clearDisplay();
    delay(2000);
    display.setCursor(0, 0);
    Serial.println();
    display.println("Connected with IP:  ");
    display.println(WiFi.localIP());
    display.println("RED:  ");
    display.println(networks[wifiSelector - 1]);
    display.display();




  }
}

void loop() {

}
