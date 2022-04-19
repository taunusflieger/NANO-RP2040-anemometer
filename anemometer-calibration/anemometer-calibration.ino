
#include <Adafruit_GPS.h>
#include <STM32SD.h>

// what's the name of the hardware serial port?
//#define GPSSerial Serial1
#define GPSSerial Serial3

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'SD.begin()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false
#define SENTENCE_BUFFER_SIZE 2000

uint32_t timer = millis();
int buf_pos = 0;

void setup()
{
  File dataFile;
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  while (!Serial);  // uncomment to have the sketch wait until Serial is ready

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println("============================================");
  Serial.println("Anemometer Calibration Tool v0.1");
  Serial.println("============================================");

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  while (!SD.begin(SD_DETECT_PIN))
  {
    delay(10);
  }
  delay(100);
  Serial.println("card initialized.");


  Serial.println("Creating / opening datalog.txt");
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, seek to last position
  if (dataFile) {
    dataFile.seek(dataFile.size());
    dataFile.println("============================================");
    dataFile.println("Anemometer Calibration Tool v0.1");
    dataFile.println("============================================");
    dataFile.flush();
    dataFile.close();
 }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }


  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);



}

void loop() // run over and over again
{
  File dataFile;
  char datasetNMEA[SENTENCE_BUFFER_SIZE];
  char line_buffer[100];
  int line_len;
  const int max_sentence = 20;
  int led_state = LOW;
  
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) { // this also sets the newNMEAreceived() flag to false
      Serial.println("Parse failed");
      return; // we can fail to parse a sentence in which case we should just wait for another
    }
 

    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    if (buf_pos + 100 >= SENTENCE_BUFFER_SIZE) {
      buf_pos = 0;
      dataFile = SD.open("datalog1.txt", FILE_WRITE);
      // if the file is available, seek to last position
      if (dataFile) {
        // append data to existing file
        dataFile.seek(dataFile.size());
        dataFile.print(datasetNMEA);
        dataFile.flush();
        dataFile.close();
      }
      else {
        // if the file isn't open, pop up an error:
         Serial.println("error opening datalog.txt");
      }

   }


    strcpy(line_buffer, GPS.lastNMEA());
    line_len = strlen(line_buffer);
    strcpy(&datasetNMEA[buf_pos], line_buffer);
    buf_pos += line_len;

    Serial.print(line_buffer);

  }

  digitalWrite(LED_BUILTIN, led_state);

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    if (led_state == HIGH)
       led_state = LOW;
    else
       led_state = HIGH;
    
    timer = millis(); // reset the timer
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
}
