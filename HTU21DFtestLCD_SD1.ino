/*************************************************** 
  This is an example code for the HTU21D-F Humidity & Temp Sensor + LCD screen

  Provided by Mizuho Nita (Virginia Polytech Institute and State Univ.)

  Designed specifically to work with the HTU21D-F sensor from Adafruit
  (https://www.adafruit.com/products/1899)

  The LCD screen has a LCD backpack
  MCP23008 I2C expander
  (https://learn.adafruit.com/i2c-spi-lcd-backpack)
  
   Adafruit Assembled Data Logging shield for Arduino
  https://www.adafruit.com/products/1141?gclid=CNrA1bL2r9ACFRZMDQod_6oBrQ

  These displays use I2C to communicate, 2 pins are required to  
  interface

    The circuit:
 HTU21D-F sensor
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
MCP23008 I2C expander (backpack)
 * Connect Vin to 3-5VDC
 * Connect GND to ground
 * Connect SCL to I2C clock pin (A5 on UNO)
 * Connect SDA to I2C data pin (A4 on UNO)

Note: i2C can share multiple inputs/outputs!!!
 ****************************************************/

// Call necessary libraries
// Wire = i2C library
#include "Wire.h" // for i2C
#include "SD.h" // for card
#include "SPI.h"
#include "RTClib.h" // for real time clock
#include "Adafruit_HTU21DF.h" // for Temp/RH sensor
#include "Adafruit_LiquidCrystal.h" // For LCD
//#include <LiquidCrystal.h> // don't need t

// A simple data logger for the Arduino analog pins
// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL 5000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 10000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()
#define ECHO_TO_SERIAL 1 // echo data to serial port
#define WAIT_TO_START 0 // Wait for serial input in setup()

RTC_DS1307 RTC; // define the Real Time Clock object
// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;
void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
      lcd.setBacklight(HIGH);
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Error: "); 
      lcd.setCursor(1,1);
      lcd.print(str);
      delay(2000);
  while(1);
}


void setup() 
{
  // initialize the serial communications:
  Serial.begin(9600);
  // Set up for serial monitor:
  Serial.println("RH measurement with HTU21D-F");
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  // Sensor error:
  if (!htu.begin()) 
    {
    Serial.println("Couldn't find sensor!");
    while (1);
    }

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
    {
    error("Card failed, or not present");
    // don't do anything more:
    }
  Serial.println("card initialized.");

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) 
  {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // leave the loop!
    }
  }
  
  if (! logfile) 
  {
    error("couldnt create file");
  }
  Serial.print("Logging to: ");
  Serial.println(filename);
      // Then print in the LCD display
      lcd.setBacklight(HIGH);
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Logging to: "); 
      lcd.setCursor(1,1);
      lcd.print(filename);
      delay(2000);
  
 // connect to RTC
  Wire.begin();
    RTC.begin();
      if (!RTC.begin()) 
          {
              logfile.println("RTC failed");
    
      #if ECHO_TO_SERIAL
              Serial.println("RTC failed");
      #endif //ECHO_TO_SERIAL
          }

    logfile.println("millis,datetime,t1,h1");
    #if ECHO_TO_SERIAL
      Serial.println("millis,datetime,t1,h1");
    #endif //ECHO_TO_SERIAL
}

void loop() 
{

  /*
  Serial.print("Temp: "); Serial.print(htu.readTemperature());
  Serial.print("\t\tHum: "); Serial.println(htu.readHumidity());
  delay(5000); 
  */

// LCD Display

  lcd.setBacklight(HIGH);
  lcd.clear();
    lcd.setCursor(1,1);
      lcd.print("Hum = "); lcd.print(htu.readHumidity());
    lcd.setCursor(1,0);
      lcd.print("Temp = "); lcd.print(htu.readTemperature());

DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  // digitalWrite(greenLEDpin, HIGH);

  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m); // milliseconds since start
  logfile.print(", ");

  #if ECHO_TO_SERIAL
    Serial.print(m); // milliseconds since start
    Serial.print(", ");
  #endif

  // fetch the time
  now = RTC.now();

  // log time
  //logfile.print(now.unixtime()); // seconds since 1/1/1970
  //logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
  
  #if ECHO_TO_SERIAL
    //Serial.print(now.unixtime()); // seconds since 1/1/1970
    //Serial.print(", ");
    Serial.print('"');
    Serial.print(now.year(), DEC);
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print('"');
  #endif //ECHO_TO_SERIAL
  
  // Sensors sensor
  float h1 = htu.readHumidity();
  // Read temperature as Celsius
  float t1 = htu.readTemperature();

  // store into SD card
  logfile.print(", ");
  logfile.print(t1);
  logfile.print(", ");
  logfile.println(h1); //ln to change line

    #if ECHO_TO_SERIAL
    Serial.print(", Temp = ");
    Serial.print(t1);
    Serial.print(", RH = ");
    Serial.println(h1);
    #endif //ECHO_TO_SERIAL
  
    logfile.flush(); // to save file each time
}




