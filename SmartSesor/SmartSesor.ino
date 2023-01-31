#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>


#define FIREBASE_HOST "fivi-31d19-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "NdNnSqss4xEny1gR6cVnrioqTaHEcVZieFNPCNLo"

FirebaseData fbdo;

FirebaseJson json;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <ESP8266WiFi.h>

#include <SD.h>     // incluye libreria para tarjetas SD


File archivo;     // objeto archivo del tipo File




int valA;
int valB;
int valC;
uint8_t Btn_SELECT = D3;
uint8_t Btn_Enter = D4;

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
    if ( buttonState == LOW) {
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
    if ( buttonState == LOW) {
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
  display.println("WIFI STREAM");


  display.setCursor(10, 30);
  display.println("Wifi SAVE");


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
    if ( buttonState == LOW) {
      menuCount = menuCount + 1; //turn LED on
      lastDebounceTime = millis(); //set the current time
    }
  }//close if(time buffer)

}


void enterCheck() {

  buttonState = digitalRead(Btn_Enter);

  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if ( buttonState == LOW) {
      if (menuCount == 1) {
        page = 1;
      } else if (menuCount == 3) {
        page = 3;
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

void displayTempWIFI(float temp) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("   Temp/Hum   WIFI   ");
  display.println("---------------------");
  display.setCursor(28, 27);


  display.setTextSize(3);
  display.print(temp, 1);
  display.print((char)247);
  display.display();
}

void displayTempSD(float temp) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("   Temp/Hum    SD    ");
  display.println("---------------------");
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

  while (!SD.begin(10)) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 40);
    display.println("  SD NOT FOUND   ");
    display.display();
    delay(300);
  }


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
    } else if (page == 3) {
      toggle = false ;
    }
    delay(100);
  }

  if (page == 2) {
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


    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    fbdo.setBSSLBufferSize(1024, 1024);
    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo.setResponseSize(1024);
    //Set database read timeout to 1 minute (max 15 minutes)
    Firebase.setReadTimeout(fbdo, 1000 * 60);
    delay(2000);
  }
}




void postTemp() {

  int Temperature = 4; // Gets the values of the temperature
  int Humidity = 4; // Gets the values of the humidity
  Serial.print(Temperature);
  Serial.println(Humidity);
  String tim = "11";
  String hou = "24";

  FirebaseJson json;
  FirebaseJson json2;
  FirebaseJson json3;

  json.set("temp", Temperature);
  json.set("hum", Humidity);
  json.set("time", tim);
  json.set("hour", hou);


  if (Firebase.pushJSON(fbdo, "/DHT", json)) {

    Serial.println(fbdo.dataPath());

    Serial.println(fbdo.pushName());

    Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());

  } else {
    Serial.println(fbdo.errorReason());
  }

}

void postSD() {

  archivo = SD.open("data.txt", FILE_WRITE);  // apertura para lectura/escritura de archivo prueba.txt
  if (archivo) {

    float temperature = 23;
    float humedad = 23;
    //DateTime fecha = rtc.now(); // funcion que devuelve fecha y horario en formato
    // DateTime y asigna a variable fecha
    //archivo.print(fecha.day());     // funcion que obtiene el dia de la fecha completa
    //archivo.print("/");       // caracter barra como separador
    //archivo.print(fecha.month());     // funcion que obtiene el mes de la fecha completa
    //archivo.print("/");       // caracter barra como separador
    //archivo.print(fecha.year());      // funcion que obtiene el año de la fecha completa
    //archivo.print(", ");       // caracter espacio en blanco como separador
    //archivo.print(fecha.hour());      // funcion que obtiene la hora de la fecha completa
    //archivo.print(":");       // caracter dos puntos como separador
    //archivo.print(fecha.minute());      // funcion que obtiene los minutos de la fecha completa
    //archivo.print(":");       // caracter dos puntos como separador
    //archivo.print(fecha.second());    // funcion que obtiene los segundos de la fecha completa
    //archivo.print(", ");

    archivo.print(2);     // funcion que obtiene el dia de la fecha completa
    archivo.print("/");       // caracter barra como separador
    archivo.print(1);     // funcion que obtiene el mes de la fecha completa
    archivo.print("/");       // caracter barra como separador
    archivo.print(2023);      // funcion que obtiene el año de la fecha completa
    archivo.print(", ");       // caracter espacio en blanco como separador
    archivo.print(24);      // funcion que obtiene la hora de la fecha completa
    archivo.print(":");       // caracter dos puntos como separador
    archivo.print(13);      // funcion que obtiene los minutos de la fecha completa
    archivo.print(":");       // caracter dos puntos como separador
    archivo.print(12);    // funcion que obtiene los segundos de la fecha completa
    archivo.print(", ");
    archivo.print(String(temperature)); // escritura de una linea de texto en archivo
    archivo.print(", ");
    archivo.println(String(humedad)); // escritura de una linea de texto en archivo
    archivo.close();        // cierre del archivo
    Serial.println("escritura correcta"); // texto de escritura correcta en monitor serie
  } else {
    Serial.println("error en apertura de prueba.txt");  // texto de falla en apertura de archivo
  }

}


void loop() {

  if (page == 2) {
    float mainTemp = random(18, 25);
    float decimalTemp = random(0, 99) / 100.0;
    float temp = mainTemp + decimalTemp;
    displayTempWIFI(temp);
    postTemp();
    delay(2000);
  }

  if (page == 3) {
    float mainTemp = random(18, 25);
    float decimalTemp = random(0, 99) / 100.0;
    float temp = mainTemp + decimalTemp;
    displayTempSD(temp);
    postSD();
    delay(2000);
  }



}
