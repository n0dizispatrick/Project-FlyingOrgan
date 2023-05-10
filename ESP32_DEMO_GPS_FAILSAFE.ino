#include <Adafruit_SHT31.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <BH1750.h>
#include <Battery18650Stats.h>
#include <WiFi.h>
#include <HTTPClient.h>
// GPRS Credentials
const char apn[]      = "m2mglobal"; // APN 
const char gprsUser[] = ""; 
const char gprsPass[] = ""; 
const char simPIN[]   = ""; 

const char* ssid     = "Jarrett Laptop";
const char* password = "Greenwood";



// Server details
const char server[] = "flyingorgan.com"; // domain name
const char resource[] = "/post-data.php";         // resource path
const int  port = 80;                             // server port number

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
const char* serverName = "http://flyingorgan.com/post-data.php";
String apiKeyValue = "tPmAT5Ab3j7F9";

int cellularFlag = 0;
#include "FS.h"
#include <SD.h>
#include <SPI.h>


//SD Card Pins
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
SPIClass sdSPI(VSPI);

String dataMessage;
float accelGyro[] = {0, 0, 0, 0, 0, 0};
int flag = 0;
float Xoffset = 0;
float Yoffset = 0;
float Zoffset = 0;

// TSIM pins
#define MODEM_PWKEY          4
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22
//#define ADC_PIN 35 
//#define CONVERSION_FACTOR    1.8 
//#define READS                20 

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM7000      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_SHT31 sht31probe = Adafruit_SHT31();
Adafruit_BME280 bme280;
Adafruit_MPU6050 mpu6050;
BH1750 lightSensor(0x23);
Battery18650Stats battery(35, 1.79, 20);

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

#include <Adafruit_Sensor.h>


// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

#define uS_TO_S_FACTOR 1000000UL   /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600        /* Time ESP32 will go to sleep (in seconds) 3600 seconds = 1 hour */
int i = 0; //iteration counter for data sending


void setup() {
  // Set serial monitor debugging window baud rate to 115200
  SerialMon.begin(115200);

  //setup SPI for SD card communication
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)) { //checking SD card connections
    Serial.println("Card Mount Failed");
    return;
  }
  //creating sd card object and checking to see if it's connecting properly
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/SensorData.csv");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/SensorData.csv", "Temperature, Humidity, ProbeTemp, ProbeHumidity, Pressure, Altitude, ax, ay, az, gx, gy, gz, Light, lat, long, batteryvoltage \r\n");
  }
  else {
    Serial.println("File already exists, will append");  
  }

  

  //Turn on modem
  pinMode(MODEM_PWKEY, OUTPUT);
  delay(300);
  digitalWrite(MODEM_PWKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWKEY,HIGH);


  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  SerialMon.println("Initializing modem...");
  modem.init();

  //Setup I2C Sensors
  Wire.begin(I2C_SDA,I2C_SCL);


    if (!sht31probe.begin(0x44)) {
    Serial.println("Could not find SHT31 probe sensor!");
    while (1);
  }

  if (!bme280.begin(0x77)) {
    Serial.println("Could not find BME280 sensor!");
    while (1);
  }

  if (!mpu6050.begin()) {
    Serial.println("Could not find MPU6050 sensor!");
    while (1);
  }

  lightSensor.begin();

  // Configure the wake up source as timer wake up  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  //Get data from sensors
  i = i + 1;
  // float batteryvoltage = battery.getBatteryVolts();
  // float temperature = bme280.readTemperature();
  // float humidity = bme280.readHumidity();

  // float temperatureprobe = sht31probe.readTemperature();
  // float humidityprobe = sht31probe.readHumidity();

  // float pressure = bme280.readPressure() / 100.0F;
  // float altitude = bme280.readAltitude(1013.25);

  sensors_event_t accelEvent, gyroEvent, tempEvent;
  mpu6050.getEvent(&accelEvent, &gyroEvent, &tempEvent);

  float ax = (accelEvent.acceleration.x - Xoffset);
  float ay = (accelEvent.acceleration.y - Yoffset);
  float az = (accelEvent.acceleration.z - Zoffset);

  float gx = gyroEvent.gyro.x;
  float gy = gyroEvent.gyro.y;
  float gz = gyroEvent.gyro.z;

  float accelGyro[] = {ax, ay, az, gx, gy, gz};

  if(ax > accelGyro[0]){
    accelGyro[0] = ax;
  }
  if(ay > accelGyro[1]){
    accelGyro[1] = ay;
  }
  if(az > accelGyro[2]){
    accelGyro[2] = az;
  }
  if(gx > accelGyro[3]){
    accelGyro[3] = gx;
  }
  if(gy > accelGyro[4]){
    accelGyro[4] = gy;
  }
  if(gz > accelGyro[5]){
    accelGyro[5] = gz;
  }
  // Serial.println("ax, ay, az, gx, gy, gz");
  // Serial.println(accelGyro[0]);
  // Serial.println(accelGyro[1]);
  // Serial.println(accelGyro[2]);
  // Serial.println(accelGyro[3]);
  // Serial.println(accelGyro[4]);
  // Serial.println(accelGyro[5]);
  uint16_t lux = lightSensor.readLightLevel();
  //50 = about 10 seconds
  //50000 = 2 minutes and 35 seconds
  //5000 = 23-25 seconds
  //1000 = 10 seconds
  Serial.println(i);
  Serial.println(lux);
  Serial.println(ax);
  Serial.println(ay);
  Serial.println(az);
  if(i == 10000 || lux > 300 || ((ax > 8 || ax < -8) || (ay > 8 || ay < -8) || (az < -8 || az > 8))){// || (gx > 1 || gx < -1) || (gy < -1 || gy > 1) || (gz < -1 || gz > 1))){
    
    Serial.println("Sending Data");
    float batteryvoltage = battery.getBatteryVolts();
    float temperature = bme280.readTemperature();
    float humidity = bme280.readHumidity();
    float temperatureprobe = sht31probe.readTemperature();
    float humidityprobe = sht31probe.readHumidity();

    float pressure = bme280.readPressure() / 100.0F;
    float altitude = bme280.readAltitude(1013.25);
  //Get gps location
    Serial.println("Waiting for gps...");
    Serial.println("Flag is ");
    Serial.println(flag);
    if(flag == 0){
      Xoffset = ax;
      Yoffset = ay;
      Zoffset = az;
      float ax = (accelEvent.acceleration.x - Xoffset);
      float ay = (accelEvent.acceleration.y - Yoffset);
      float az = (accelEvent.acceleration.z - Zoffset);
      Serial.println("X, Y, Z offsets");
      Serial.println(Xoffset);
      Serial.println(Yoffset);
      Serial.println(Zoffset);
      Serial.println("X, Y, Z values no offset");
      Serial.println(ax);
      Serial.println(ay);
      Serial.println(az);
      flag = 1;
    }
    modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(10000L) != 1) {
      DBG(" SGPIO=0,4,1,1 false ");
    }
    modem.enableGPS();
    float lat, lon, lat2, lon2, speed,alt,accuracy;
    int year,month,day,hour,min,sec,vsat,usat;
    // lat = 33.587430;
    // lon = -101.8785668;
    //setting iterations for gps timeout
    int iter = 0;
    while (iter < 60) {
      if (modem.getGPS(&lat2, &lon2, &speed, &alt, &vsat, &usat, &accuracy,
                      &year, &month, &day, &hour, &min, &sec)) {
        
        modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                      &year, &month, &day, &hour, &min, &sec);
        Serial.printf("lat:%f lon:%f\n", lat, lon);
        break;
      } else {
        Serial.print("getGPS ");
        Serial.println(iter);
      }
      delay(500);
      iter += 1;
    }
    // modem.disableGPS(); uncomment this line to reduce power consumption

    modem.sendAT("+SGPIO=0,4,1,0");
    if (modem.waitResponse(10000L) != 1) {
      DBG(" SGPIO=0,4,1,0 false ");
    }

      bool cellConnect = modem.gprsConnect(apn,gprsUser,gprsPass);
      if(cellConnect){
          Serial.println("THIS IS CELLULAR");
          SerialMon.println(" OK");
          SerialMon.print("Connecting to ");
          SerialMon.print(server);
        if (!client.connect(server, port)) {
          SerialMon.println(" fail");
        }
          SerialMon.println(" OK");
        
          // Making an HTTP POST request
          SerialMon.println("Performing HTTP POST request...");
          String httpRequestData = "api_key=" + apiKeyValue + "&AmbientTemp=" + String(bme280.readTemperature()) + "&AmbientHumidity=" + String(bme280.readHumidity()) + "&ProbeTemp=" + String(sht31probe.readTemperature()) + "&ProbeHumidity=" + String(sht31probe.readHumidity()) + "&Pressure=" + String(bme280.readPressure() / 100.0F) + "&Altitude=" + String(bme280.readAltitude(1013.25))+"&Ax="+String(accelGyro[0]) + "&Ay=" + String(accelGyro[1])+ "&Az=" + String(accelGyro[2])+ "&Light=" +String(lux)+"&Latitude="+String(lat,6)+"&Longitude="+String(lon,6)+"&Battery="+String(batteryvoltage);
        
        
          client.print(String("POST ") + resource + " HTTP/1.1\r\n");
          client.print(String("Host: ") + server + "\r\n");
          client.println("Connection: close");
          client.println("Content-Type: application/x-www-form-urlencoded");
          client.print("Content-Length: ");
          client.println(httpRequestData.length());
          client.println();
          client.println(httpRequestData);

          unsigned long timeout = millis();
          while (client.connected() && millis() - timeout < 10000L) {
            // Print available data (HTTP response from server)
            while (client.available()) {
              char c = client.read();
              SerialMon.print(c);
              timeout = millis();
            }
          }
          //SerialMon.println();
        
          // Close client and disconnect
          client.stop();
          //delay(3000);
          
          //SerialMon.println(F("Server disconnected"));
          modem.gprsDisconnect();
      }
      else{
          WiFi.begin(ssid, password);
  Serial.println("Connecting");
  int iter2 = 0;
  while(WiFi.status() != WL_CONNECTED || iter2 < 90) { 
    delay(500);
    Serial.print(".");
    iter2 = iter2 + 1;
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


     if(WiFi.status()== WL_CONNECTED && cellConnect == 0){
    WiFiClient wificlient;
    HTTPClient http;
    Serial.println("THIS IS WIFI");
    
    // Your Domain name with URL path or IP address with path
    http.begin(wificlient, serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&AmbientTemp=" + String(bme280.readTemperature()) + "&AmbientHumidity=" + String(bme280.readHumidity()) + "&ProbeTemp=" + String(sht31probe.readTemperature()) + "&ProbeHumidity=" + String(sht31probe.readHumidity()) + "&Pressure=" + String(bme280.readPressure() / 100.0F) + "&Altitude=" + String(bme280.readAltitude(1013.25))+"&Ax="+String(accelGyro[0]) + "&Ay=" + String(accelGyro[1])+ "&Az=" + String(accelGyro[2])+"&Light=" +String(lux)+"&Latitude="+String(lat,6)+"&Longitude="+String(lon,6)+"&Battery="+String(batteryvoltage);
    //String httpRequestData2= ;
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
      }

          //opening file on sd card
          File file = SD.open("/SensorData.csv");
          //appends data onto next row of .csv file
          logSDCard(temperature, humidity, temperatureprobe, humidityprobe, pressure, altitude, ax, ay, az, gx, gy, gz, lux, lat, lon, batteryvoltage);
      
    
    for(int j = 0; j < sizeof(accelGyro); j++){
      accelGyro[j] = 0;
    }
    //reset interval counter
    i = 0;
    //delay(2000);
  }
  // //Print Sensor Data *NOT NEEDED JUST FOR DEBUG/TROUBLESHOOT
  // Serial.print("Temperature: ");
  // Serial.print(temperature);
  // Serial.println(" °C");

  // Serial.print("Humidity: ");
  // Serial.print(humidity);
  // Serial.println(" %"); 

  // Serial.print("Probe Temperature: ");
  // Serial.print(temperatureprobe);
  // Serial.println(" °C");

  // Serial.print("Probe Humidity: ");
  // Serial.print(humidityprobe);
  // Serial.println(" %");

  // Serial.print("Pressure: ");
  // Serial.print(pressure);
  // Serial.println(" hPa");

  // Serial.print("Altitude: ");
  // Serial.print(altitude);
  // Serial.println(" m");

  // Serial.print("Accelerometer (m/s^2): ");
  // Serial.print(ax);
  // Serial.print(", ");
  // Serial.print(ay);
  // Serial.print(", ");
  // Serial.println(az);

  // Serial.print("Gyroscope (rad/s): ");
  // Serial.print(gx);
  // Serial.print(", ");
  // Serial.print(gy);
  // Serial.print(", ");
  // Serial.println(gz);

  // Serial.print("Light: ");
  // Serial.print(lux);
  // Serial.println(" lx");
 
  // Serial.print("Lat: ");
  // Serial.print(lat,6);
  // Serial.print(", ");
  // Serial.print("Long: ");
  // Serial.println(lon,6);
  // Serial.print("Altidute: ");
  // Serial.println(alt,6);
  // Serial.print("Year: ");
  // Serial.print(year);
  // Serial.print(" Month: ");
  // Serial.print(month);
  // Serial.print(" Day: ");
  // Serial.println(day);
  // Serial.print("Hour: ");
  // Serial.print(hour);
  // Serial.print(" Minute: ");
  // Serial.print(min);
  // Serial.print(" Second: ");
  // Serial.println(sec);    
}

void logSDCard(float temperature, float humidity, float temperatureprobe, float humidityprobe, float pressure, float altitude, float ax, float ay, float az, float gx, float gy, float gz, float lux, float lat, float lon, float batteryvoltage) {
  dataMessage = String(temperature) + "," + String(humidity) + "," + String(temperatureprobe) + "," + String(humidityprobe) + "," + String(pressure) + "," + String(altitude) + "," + String(ax) + "," + String(ay) + "," + String(az) + "," + String(gx) + "," + String(gy) + "," + String(gz) + "," + String(lux) + "," + String(lat,6) + "," + String(lon,6) + "," + String(batteryvoltage) + "\r\n";
  // Serial.print("Save data: ");
  // Serial.println(dataMessage);
  appendFile(SD, "/SensorData.csv", dataMessage.c_str());
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
