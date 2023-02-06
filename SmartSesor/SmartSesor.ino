#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>


#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

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

#include "DHT.h"
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
#define DHTPin 3
DHT dht(DHTPin, DHTTYPE);


int valA;
int valB;
int valC;
uint8_t Btn_SELECT = D3;
uint8_t Btn_Enter = D4;

int buttonState = LOW; //this variable tracks the state of the button, low if not pressed, high if pressed
int ledState = -1; //this variable tracks the state of the LED, negative if off, positive if on

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 150;    // the debounce time; increase if the output flick

byte clk;
byte menuCount   = 1;
byte wifiSelector = 1;

byte dir = 0;
bool runState = false;
byte page = 0;
bool search = true;
String networks[4] = {"not found", "not found", "not found", "not found"};

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

bool wificon = false;
bool moveptr = false;
bool toggle = true;
bool page4wastoggle = false;
bool page5wastoggle = false;
float Temperature;
float Humidity;

int enableAction = 1;





//SD  (GND) -- Ground
//SD  (3.3v) --3v
//SD  (MISO) --D6.
//SD (MOSI) -- D7
//SD  (SCK/clock) -- D5
//SD  (CS) -- D8


//action hours
///////////////////////////año-mes-dia,horas,minutos,segundos
RtcDateTime previousTime;



bool wasError(const char* errorTopic = "")
{
  uint8_t error = Rtc.LastError();
  if (error != 0)
  {
    // we have a communications error
    // see https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
    // for what the number means
    Serial.print("[");
    Serial.print(errorTopic);
    Serial.print("] WIRE communications error (");
    Serial.print(error);
    Serial.print(") : ");

    switch (error)
    {
      case Rtc_Wire_Error_None:
        Serial.println("(none?!)");
        break;
      case Rtc_Wire_Error_TxBufferOverflow:
        Serial.println("transmit buffer overflow");
        break;
      case Rtc_Wire_Error_NoAddressableDevice:
        Serial.println("no device responded");
        break;
      case Rtc_Wire_Error_UnsupportedRequest:
        Serial.println("device doesn't support request");
        break;
      case Rtc_Wire_Error_Unspecific:
        Serial.println("unspecified error");
        break;
      case Rtc_Wire_Error_CommunicationTimeout:
        Serial.println("communications timed out");
        break;
    }
    return true;
  }
  return false;
}



void UploadData () {
  // re-open the file for reading:
  char* buf = "data.txt";
  String line;
  display.clearDisplay();
  display.setCursor(0, 30);
  display.println("Enviando data a la Nube");
  File SdFile;
  SdFile = SD.open(buf);
  if (SdFile) {
    Serial.println("data.txt:");
    while (SdFile.available()) {
      line = SdFile.readStringUntil('\n');
      Serial.println(line);
      display.print(".");
      display.display();

      String str  =  line;
      int index = 0;
      char str_copy[str.length() + 1];
      strcpy(str_copy, str.c_str());

      char *p = strtok(str_copy, ",");

      int sectdata = 0;

      String date;
      String hour;
      float temp;
      float hum;
      while (p != NULL) {
        if (sectdata == 0) {
          date = String(p);
        } else if (sectdata == 1) {
          hour = String(p);
        } else if (sectdata == 2) {
          temp = atof(String(p).c_str());
        } else {
          hum = atof(String(p).c_str());
        }
        sectdata = sectdata + 1;
        p = strtok(NULL, ",");
      }
      postTemp(temp , hum , date, hour);
      delay(500);
    }
    SdFile.close();
  } else {
    Serial.println("error opening data.txt");
  }



  SD.remove(buf);


  delay (800);

}


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
  display.println("Set Clock");

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
      } else if (menuCount == 4) {
        page = 4;
      } else if (menuCount == 2) {
        page = 5;
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
    } else  if (page == 4) {
      Page1();
      page4wastoggle = true;
    } else if (page ==  5) {
      Page1();
      page5wastoggle = true;
    }
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

  // others page selection
  if (page4wastoggle) {
    page = 4;
  }
  if (page5wastoggle) {

    UploadData();
    page = 5;
  }

  if (page == 4) {

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
    setenv("TZ", "CST6CDT,M4.1.0,M10.5.0", 1);
    get_Hour_First();

    Serial.print(get_Hour());

    String str  =  get_Hour();
    int numbers[6];

    int index = 0;
    char str_copy[str.length() + 1];
    strcpy(str_copy, str.c_str());

    char *p = strtok(str_copy, ",");
    while (p != NULL) {
      numbers[index++] = atoi(p);
      p = strtok(NULL, ",");
    }

    Rtc.Begin();
#if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(3000 /* us */, true /* reset_on_timeout */);
#endif
    ///////////////////////////año-mes-dia,horas,minutos,segundos
    RtcDateTime compiled = RtcDateTime(numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], numbers[5]);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid())
    {
      if (!wasError("setup IsDateTimeValid"))
      {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");

        // following line sets the RTC to the date & time this sketch was compiled
        // it will also reset the valid flag internally unless the Rtc device is
        // having an issue

        Rtc.SetDateTime(compiled);
      }
    }

    if (!Rtc.GetIsRunning())
    {
      if (!wasError("setup GetIsRunning"))
      {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
      }
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (!wasError("setup GetDateTime"))
    {
      if (now < compiled)
      {
        Serial.println("RTC is older than compile time, updating DateTime");
        Rtc.SetDateTime(compiled);
      }
      else if (now > compiled)
      {
        Serial.println("RTC is newer than compile time, this is expected");
      }
      else if (now == compiled)
      {
        Serial.println("RTC is the same as compile time, while not expected all is still fine");
      }
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    wasError("setup Enable32kHzPin");
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    wasError("setup SetSquareWavePin");

  } else {

    Rtc.Begin();
#if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(3000 /* us */, true /* reset_on_timeout */);
#endif
    Rtc.Enable32kHzPin(false);
    wasError("setup Enable32kHzPin");
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    wasError("setup SetSquareWavePin");
    previousTime = Rtc.GetDateTime();
  }

  pinMode(DHTPin, INPUT);
  dht.begin();
}



String get_Hour() {
  time_t now;
  time(&now);
  char time_output[30];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  strftime(time_output, 30, "%y,%m,%d,%H,%M,%S", localtime(&now));
  return String(time_output); // returns Sat 20-Apr-19 12:31:45
}


String get_Hour_First() {
  time_t now;
  time(&now);
  char time_output[30];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  strftime(time_output, 30, "%H:%M", localtime(&now));
  display.clearDisplay();
  display.setCursor(3, 10);
  delay(2000);
  display.println("Ajustando           ");
  display.println("Zona Horaria...     ");
  display.display();
  for (int i = 0; i < 10; i++) {
    display.print(".");
    display.display();
    delay(3000);
  }
  strftime(time_output, 30, "%H:%M", localtime(&now));
  return String(time_output); // returns Sat 20-Apr-19 12:31:45
}





void postTemp(float Temperature , float Humidity , String date, String hour) {


  FirebaseJson json;
  FirebaseJson json2;
  FirebaseJson json3;

  json.set("temp", Temperature);
  json.set("hum", Humidity);
  json.set("date", date);
  json.set("hour", hour);


  if (Firebase.pushJSON(fbdo, "/DHT", json)) {

    Serial.println(fbdo.dataPath());

    Serial.println(fbdo.pushName());

    Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());

  } else {
    Serial.println(fbdo.errorReason());
  }

}

void postSD(float temperature, float humedad , String date, String hour) {
  char* buf = "data.txt";

  if (!SD.exists(buf)) {
    while (!SD.exists(buf)) {
      archivo = SD.open(buf, FILE_WRITE);
    }
  }else{
  archivo = SD.open(buf, FILE_WRITE); 
 
  }
  
  // apertura para lectura/escritura de archivo prueba.txt
  if (archivo) {
    archivo.print(date);
    archivo.print(",");
    archivo.print(hour);
    archivo.print(",");
    archivo.print(String(temperature)); // escritura de una linea de texto en archivo
    archivo.print(",");
    archivo.println(String(humedad)); // escritura de una linea de texto en archivo
    archivo.close();        // cierre del archivo
    //Serial.println("escritura correcta"); // texto de escritura correcta en monitor serie
  } else {
    //Serial.println("error en apertura de data.txt");  // texto de falla en apertura de archivo
  }

}


bool hourPassed(const RtcDateTime& dt)
{
  bool passed = false;

  if (dt.Hour() != previousTime.Hour()) {
    passed = true;
    previousTime = dt;
  }

  return passed;
}



void loop() {

  if (page == 2) {

    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity
    displayTempWIFI(Temperature);

    //aqui va la condicional
    RtcDateTime now = Rtc.GetDateTime();
    String date = DateTimeString(now);
    String hour = HourTimeString(now);
    if ( hourPassed(now)) {
      postTemp(Temperature, Humidity, date , hour);
      delay(500);
    }
    delay(2000);
  }

  else if (page == 3) {
    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity
    displayTempSD(Temperature);

    RtcDateTime now = Rtc.GetDateTime();
    if ( hourPassed(now)) {
      postSD(Temperature, Humidity, DateTimeString(now), HourTimeString(now));
      delay(500);
    }

  } else if (page == 4) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Reloj Ajustado :)");
    display.setCursor(0, 30);
    if (!Rtc.IsDateTimeValid())
    {
      if (!wasError("loop IsDateTimeValid"))
      {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
      }
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (!wasError("loop GetDateTime"))
    {
      printDateTime(now);
      Serial.println();
    }

    display.display();
  } else if (page = 5) {
    display.clearDisplay();
    display.setCursor(0, 30);
    display.println("Carga completa:)");
    display.display();

  }
}





#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  display.print(datestring);
}

String DateTimeString (const RtcDateTime& dt)
{
  char datestring[20];
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u"),
             dt.Month(),
             dt.Day(),
             dt.Year());
  return String(datestring);
}

String HourTimeString (const RtcDateTime& dt)
{
  char datestring[20];
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u:%02u"),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  return String(datestring);
}

