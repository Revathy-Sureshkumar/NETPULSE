#include <WiFi.h>
#include <esp_wifi.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>

const char* AP_SSID = "NETPULSE";
const char* AP_PASSWORD = "NetPulse123";
#define GREEN_LED 4
#define RED_LED 5
#define BUZZER 18
#define DHT_PIN 15
#define DHT_TYPE DHT11
#define LCD_SDA 21
#define LCD_SCL 22
#define MAX_DEVICES 3
const float TEMP_LIMIT = 35.0;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);
uint8_t registeredMAC[MAX_DEVICES][6];
bool deviceRegistered[MAX_DEVICES] = {false,false,false};
bool deviceOnline[MAX_DEVICES] = {false,false,false};
const char* deviceNames[MAX_DEVICES] = {"LAP 1","LAP 2","LAP 3"};
float temperature=0, humidity=0;
bool temperatureFault=false, deviceFault=false, systemFault=false, previousFault=false;
unsigned long buzzerStartTime=0, lastLCDUpdate=0;
bool buzzerActive=false;
const unsigned long BUZZER_DURATION=5000, LCD_INTERVAL=2000;
int lcdPage=0;

void printMAC(uint8_t* mac){for(int i=0;i<6;i++){if(mac[i]<16)Serial.print("0");Serial.print(mac[i],HEX);if(i<5)Serial.print(":");}}
bool macEqual(uint8_t* a,uint8_t* b){for(int i=0;i<6;i++)if(a[i]!=b[i])return false;return true;}
int findRegisteredDevice(uint8_t* mac){for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i]&&macEqual(mac,registeredMAC[i]))return i;return -1;}
void registerNewDevice(uint8_t* mac){for(int i=0;i<MAX_DEVICES;i++){if(!deviceRegistered[i]){memcpy(registeredMAC[i],mac,6);deviceRegistered[i]=true;deviceOnline[i]=true;Serial.print("REGISTERED ");Serial.print(deviceNames[i]);Serial.print(" : ");printMAC(mac);Serial.println();return;}}Serial.println("Device limit reached.");}

void checkConnectedDevices(){
  wifi_sta_list_t stationList;
  memset(&stationList,0,sizeof(stationList));
  esp_err_t result=esp_wifi_ap_get_sta_list(&stationList);
  if(result!=ESP_OK){Serial.println("Failed to get station list.");return;}
  for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i])deviceOnline[i]=false;
  for(int i=0;i<stationList.num;i++){
    uint8_t* mac=stationList.sta[i].mac;
    Serial.print("CONNECTED MAC: ");printMAC(mac);Serial.println();
    int index=findRegisteredDevice(mac);
    if(index==-1){registerNewDevice(mac);index=findRegisteredDevice(mac);}
    if(index>=0)deviceOnline[index]=true;
  }
}
int getOnlineCount(){int count=0;for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i]&&deviceOnline[i])count++;return count;}
bool checkDeviceFault(){for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i]&&!deviceOnline[i])return true;return false;}
int getOfflineDevice(){for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i]&&!deviceOnline[i])return i;return -1;}

void readEnvironment(){
  float t=dht.readTemperature(), h=dht.readHumidity();
  if(!isnan(t))temperature=t;
  if(!isnan(h))humidity=h;
  temperatureFault=(temperature>=TEMP_LIMIT);
}

void updateFaultState(){
  deviceFault=checkDeviceFault();
  systemFault=deviceFault||temperatureFault;
  if(systemFault&&!previousFault){
    Serial.println();Serial.println("==============================");Serial.println("       NETPULSE FAULT");Serial.println("==============================");
    if(deviceFault){int offlineDevice=getOfflineDevice();if(offlineDevice>=0){Serial.print("DEVICE OFFLINE: ");Serial.println(deviceNames[offlineDevice]);}}
    if(temperatureFault){Serial.print("HIGH TEMPERATURE: ");Serial.print(temperature,1);Serial.println(" C");}
    Serial.println("==============================");
    digitalWrite(BUZZER,HIGH);buzzerActive=true;buzzerStartTime=millis();
  }
  if(systemFault){digitalWrite(RED_LED,HIGH);digitalWrite(GREEN_LED,LOW);}else{digitalWrite(RED_LED,LOW);digitalWrite(GREEN_LED,HIGH);}
  previousFault=systemFault;
}
void updateBuzzer(){if(buzzerActive&&millis()-buzzerStartTime>=BUZZER_DURATION){digitalWrite(BUZZER,LOW);buzzerActive=false;Serial.println("5-second buzzer completed.");}}

void showNormalScreen(){lcd.clear();lcd.setCursor(0,0);lcd.print("NETPULSE");lcd.setCursor(0,1);lcd.print("Devices:");lcd.print(getOnlineCount());lcd.print(" ONLINE");}
void showDeviceScreen(){lcd.clear();lcd.setCursor(0,0);if(deviceRegistered[0]){lcd.print("L1:");lcd.print(deviceOnline[0]?"ON ":"OFF");}else lcd.print("L1:---");lcd.print(" ");if(deviceRegistered[1]){lcd.print("L2:");lcd.print(deviceOnline[1]?"ON":"OFF");}else lcd.print("L2:---");lcd.setCursor(0,1);if(deviceRegistered[2]){lcd.print("L3:");lcd.print(deviceOnline[2]?"ONLINE":"OFFLINE");}else lcd.print("L3:---");}
void showDeviceFault(){int d=getOfflineDevice();lcd.clear();lcd.setCursor(0,0);lcd.print("DEVICE FAULT");lcd.setCursor(0,1);if(d>=0){lcd.print(deviceNames[d]);lcd.print(" OFFLINE");}}
void showTemperature(){lcd.clear();lcd.setCursor(0,0);lcd.print("TEMP:");lcd.print(temperature,1);lcd.print(" C");lcd.setCursor(0,1);lcd.print("HUM:");lcd.print(humidity,0);lcd.print("%");}
void showHighTemperature(){lcd.clear();lcd.setCursor(0,0);lcd.print("HIGH TEMP!");lcd.setCursor(0,1);lcd.print("TEMP:");lcd.print(temperature,1);lcd.print(" C");}

void updateLCD(){
  if(millis()-lastLCDUpdate<LCD_INTERVAL)return;
  lastLCDUpdate=millis();
  if(systemFault){
    if(deviceFault&&temperatureFault){if(lcdPage==0){showDeviceFault();lcdPage=1;}else{showHighTemperature();lcdPage=0;}}
    else if(deviceFault)showDeviceFault();
    else if(temperatureFault)showHighTemperature();
    return;
  }
  if(lcdPage==0){showNormalScreen();lcdPage=1;}else if(lcdPage==1){showDeviceScreen();lcdPage=2;}else{showTemperature();lcdPage=0;}
}

void printStatus(){
  Serial.println();Serial.println("========== NETPULSE STATUS ==========");
  Serial.print("Temperature: ");Serial.print(temperature,1);Serial.println(" C");
  Serial.print("Humidity: ");Serial.print(humidity,0);Serial.println(" %");
  Serial.print("Connected stations: ");Serial.println(WiFi.softAPgetStationNum());Serial.println();
  for(int i=0;i<MAX_DEVICES;i++)if(deviceRegistered[i]){Serial.print(deviceNames[i]);Serial.print(" : ");Serial.println(deviceOnline[i]?"ONLINE":"OFFLINE");Serial.print("MAC: ");printMAC(registeredMAC[i]);Serial.println();}
  Serial.println();Serial.print("Temperature fault: ");Serial.println(temperatureFault?"YES":"NO");Serial.print("Device fault: ");Serial.println(deviceFault?"YES":"NO");Serial.print("System fault: ");Serial.println(systemFault?"YES":"NO");Serial.println("=====================================");
}

void setup(){
  Serial.begin(115200);delay(1000);
  pinMode(GREEN_LED,OUTPUT);pinMode(RED_LED,OUTPUT);pinMode(BUZZER,OUTPUT);
  digitalWrite(GREEN_LED,LOW);digitalWrite(RED_LED,LOW);digitalWrite(BUZZER,LOW);
  Wire.begin(LCD_SDA,LCD_SCL);lcd.init();lcd.display();lcd.clear();lcd.setCursor(0,0);lcd.print("NETPULSE");lcd.setCursor(0,1);lcd.print("Starting...");
  dht.begin();
  WiFi.mode(WIFI_AP);
  bool apStarted=WiFi.softAP(AP_SSID,AP_PASSWORD);
  if(apStarted){Serial.println();Serial.println("=====================================");Serial.println("          NETPULSE STARTED");Serial.println("=====================================");Serial.print("SSID: ");Serial.println(AP_SSID);Serial.print("PASSWORD: ");Serial.println(AP_PASSWORD);Serial.print("ESP32 AP IP: ");Serial.println(WiFi.softAPIP());Serial.println();Serial.println("Connect laptops to NETPULSE.");Serial.println("First 3 devices will be registered.");Serial.println("=====================================");}else Serial.println("ERROR: Wi-Fi AP failed!");
  delay(2000);lcd.clear();lcd.setCursor(0,0);lcd.print("NETPULSE");lcd.setCursor(0,1);lcd.print("WiFi Ready");delay(2000);digitalWrite(GREEN_LED,HIGH);
}

void loop(){checkConnectedDevices();readEnvironment();updateFaultState();updateBuzzer();updateLCD();static unsigned long lastStatus=0;if(millis()-lastStatus>=5000){lastStatus=millis();printStatus();}delay(500);}
