
#include <ESP8266.h>
#include <SoftwareSerial.h>
#include <Adafruit_BME280.h>


#define SSID        "gclab"
#define PASSWORD    "029105108"
#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT   (80)
#define API_KEY     "A20AU1GO0POTWYWW"
#define WAIT        (1000)

SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

unsigned int PM_CF10;
unsigned int PM_CF25;
unsigned int PM_CF100;
unsigned int PM_AT10;
unsigned int PM_AT25;
unsigned int PM_AT100;

Adafruit_BME280 bme; // I2C


void setup(void)
{
  bool status;
  status = bme.begin();  
    
  Serial.begin(9600);
  Serial.print("setup begin\r\n");

  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print("to station + softap ok\r\n");
  } else {
    Serial.print("to station + softap err\r\n");
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP:\r\n");
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }

  Serial.print("setup end\r\n");
}

void loop(void)
{
  int IDX = 0;
  unsigned char readData;
  unsigned char RAW;

  while( Serial.available() ) {
      readData = Serial.read();
      if( (IDX==0 && readData!=0x42) || (IDX==1 && readData!=0x4d) ) {
        break;
      }
      if( IDX > 15 ) {
        break;
      }
      else if( IDX==4 || IDX==6 || IDX==8 || IDX==10 || IDX==12 || IDX==14 ) {
         RAW = readData;
      } else if( IDX==5 ) {
        PM_CF10 = 256*RAW+readData;
      } else if( IDX==7 ) {
        PM_CF25=256*RAW+readData;
      } else if( IDX==9 ) {
        PM_CF100=256*RAW+readData;
      } else if( IDX==11 ) {
        PM_AT10=256*RAW+readData;
      } else if( IDX==13 ) {
        PM_AT25=256*RAW+readData;
      } else if( IDX==15 ) {
        PM_AT100=256*RAW+readData;
      }
      IDX++;
    }

    // Prepare next data
    while( Serial.available() ) {
        Serial.read();   
    }
    
    if(PM_AT25<=0 && PM_AT10<=0){
      delay(10000);
    }
    
    Serial.print("\n temp : ");
    Serial.print(int(bme.readTemperature()));
    Serial.print("\n humid : ");
    Serial.print(int(bme.readHumidity()));
    Serial.print("\n PM2.5 : ");
    Serial.print(PM_AT25);
    Serial.print("\n PM10.0 : ");
    Serial.print(PM_AT100);
    Serial.print("\n");

  uint8_t buffer[128] = {0};

  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
    Serial.print("create tcp ok\r\n");
  } else {
    Serial.print("create tcp err\r\n");
  }


    int temp = int(bme.readTemperature());
    int humid = int(bme.readHumidity());
    int pm25 = PM_AT25;
    int pm10 = PM_AT100;
    
    char *paramTpl = "?api_key=%s&field1=%d&field2=%d&field3=%d&field4=%d";
    char param[80];
    sprintf(param, paramTpl, API_KEY, temp, humid, pm25, pm10);
    
      // This will send the request to the server
    char *headerTpl = "GET /update%s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Cache-Control: no-cache\r\n"
                 "Content-length: 0\r\n\r\n";
    char header[200];
    sprintf(header, headerTpl, param, HOST_NAME);
  
    wifi.send((const uint8_t*)header, strlen(header));


  uint32_t len = wifi.recv(buffer, sizeof(buffer), 256);
  if (len > 0) {
    Serial.print("Received:[");
    for (uint32_t i = 0; i < len; i++) {
      Serial.print((char)buffer[i]);
    }
    Serial.print("]\r\n");
  }

  if (wifi.releaseTCP()) {
    Serial.print("release tcp ok\r\n");
  } else {
    Serial.print("release tcp err\r\n");
  }

  Serial.print("----------------------------------------------");
  delay(WAIT);
}
