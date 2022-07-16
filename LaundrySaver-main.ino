#include "M5Atom.h"
#include "AtomSocket.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"

ATOMSOCKET ATOM;
HardwareSerial AtomSerial(2);

WiFiClient wifi_client;
void mqtt_sub_callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_host, mqtt_port, mqtt_sub_callback, wifi_client);

#define RXD 22
#define RELAY 23

float last_v = 0.0f; //volts
float last_a = 0.0f; //amps
float last_w = 0.0f; //watts

unsigned long last_t_energy = 0;
unsigned long last_t_voltage = 0;
unsigned long last_cycle = 0;
unsigned long last_clear = 0;

int wifi_timeout = 90000; //200 ms delay |  1800 x 5 | 30 minutes 
int laundryStatus = 0;

bool timerReset = 0;
bool voltage_push_triggered = 0;
bool relay_status = true;

char mqtt_payload[100];

String mqtt_str; 

void setup() {
  Serial.begin(115200);
  M5.begin(true, false, true);
  ATOM.Init(AtomSerial, RELAY, RXD);
  M5.dis.drawpix(0, 0x00ff00); // green
  ATOM.SetPowerOn();
  // Wifi
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  int count = 0;
  int count_timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    if(count_timeout > wifi_timeout){
      reboot();
    }
    switch (count) {
      case 0:
        Serial.println("|");
        break;
      case 1:
        Serial.println("/");
        break;
      case 2:
        Serial.println("-");
        break;
      case 3:
        Serial.println("\\");
        break;
    }
    count = (count + 1) % 4;
    count_timeout++;
  }
  Serial.println("WiFi connected!");
  WiFi.setSleep(false);
  delay(1000);
  
  // MQTT
  bool rv = false;
  if (mqtt_use_auth == true) {
    rv = mqtt_client.connect(mqtt_client_id, mqtt_username, mqtt_password);
  }
  else {
    rv = mqtt_client.connect(mqtt_client_id);
  }
  if (rv == false) {
    Serial.println("mqtt connecting failed...");
    reboot();
  }
  Serial.println("MQTT connected!");
  delay(1000);

  // start timers for power measurement
  last_t_energy = millis();
  last_t_voltage = millis();
}

void loop() {
  if (!mqtt_client.connected()) {
    Serial.println("MQTT disconnected...");
    reboot();
  }
  mqtt_client.loop();

  ATOM.SerialReadLoop();
  if (ATOM.SerialRead == 1)
  {
    last_v = ATOM.GetVol();
    last_a = ATOM.GetCurrent();
    last_w = ATOM.GetActivePower();
  }
#ifdef VOLTAGE_UPDATE
  voltage_update();
#endif
#ifdef ENERGY_UPDATE
  energy_update();
#endif

  // Detect laundry cycles and send status via MQTT
  switch (laundryStatus) {
    case 0: //clear
      laundryStatus = last_w > 50 ? 1 : 0;
      if(laundryStatus == 1){
        if (mqtt_client.connected()) { 
          Serial.println( "status changed to cycle");
          char msg[120];
          snprintf(msg, 120, "status changed to cycle", last_w);
          mqtt_client.publish(mqtt_update_topic, msg);
          laundryStatus = 1;
          last_clear = millis();
        }
      }
      delay(50);
      break;
    case 1: //running
      if(last_w > 50){
        if (millis() - last_clear > 60 * 1000) { //change back to 60
          if (millis() - last_clear < 120 * 1000) { //change back to 120
            mqtt_str = "{\"idx\":" + String(STATUS_IDX) + ",\"nvalue\": 1}"; 
            mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
            mqtt_client.publish(mqtt_publish_topic, mqtt_payload);
            laundryStatus = 2;
          }
                  
        }else if( millis() - last_clear > 120 * 1000){ //change back to 120 * 1000
          laundryStatus = 0;
        }
      }else if(last_w < 50){
        laundryStatus = 0;
      }
      delay(50);  
      break;      
      
    case 2: //done
        if(last_w > 50){
          last_clear = millis();
            if(!timerReset){ 
              Serial.printf( "last millis reset: %.2f W ", last_w);
              timerReset = true;
            }
        }else{
          timerReset = false;
          if (millis() - last_clear > 600 * 1000) { //change back to 600 * 1000
            laundryStatus = 0;
            if (mqtt_client.connected()) { 
              Serial.println( "status changed to clear");
                mqtt_str = "{\"idx\":" + String(STATUS_IDX) + ",\"nvalue\": 0}"; 
                mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
                mqtt_client.publish(mqtt_publish_topic, mqtt_payload);
            }
          }
        }
      break;      
  }

    if (M5.Btn.wasPressed()) {
      relay_status = !relay_status;
      Serial.println("Button was pressed");
    }
  
    if (relay_status) {
      M5.dis.drawpix(0, 0x00ff00); // green
      ATOM.SetPowerOn();
  //    Serial.println("relay powered on!");
    } else {
      ATOM.SetPowerOff();
  //    Serial.println("relay powered off for some reason!");
      M5.dis.drawpix(0, 0x0000ff); // red
    }
    M5.update();
}

#define BUF_LEN 16
char buf[BUF_LEN];

void mqtt_sub_callback(char* topic, byte* payload, unsigned int length) {

//  Callback commands disabled because they are not needed
//  int len = BUF_LEN - 1 < length ? (BUF_LEN - 1) : length;
//  memset(buf, 0, BUF_LEN);
//  strncpy(buf, (const char*)payload, len);
//
//  String cmd = String(buf);
//  Serial.print("payload=");
//  Serial.println(cmd);
//
//  if (cmd == "on") {
//    relay_status = true;
//    Serial.println("mqtt command received");
//    publish_status();
//  }
//  else if (cmd == "off") {
//    relay_status = false;
//    Serial.println("mqtt command received");
//    publish_status();
//  }
//  else if (cmd == "toggle") {
//    relay_status = !relay_status;
//    Serial.println("mqtt command received");
//    publish_status();
//  }
}

void reboot() {
  Serial.println("REBOOT!!!!!");
  for (int i = 0; i < 30; ++i) {
    M5.dis.drawpix(0, 0xffff00); // yellow
    delay(50);
    M5.dis.drawpix(0, 0x000000);
    delay(50);
  }

  ESP.restart();
}
void energy_update() {
    if (millis() - last_t_energy > ENERGY_UPDATE_TIMER * 1000) {
      mqtt_str = "{\"idx\":" + String(ENERGY_IDX) + ",\"nvalue\": 1, \"svalue\":\"" + String(last_w) + "\"}"; 
      mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
      mqtt_client.publish(mqtt_publish_topic, mqtt_payload);
      last_t_energy = millis();
    }
}

void voltage_update() {
    if(millis() - last_t_voltage > VOLTAGE_PUSH_TIMER * 1000) {
      if(last_v > VOLTAGE_STANDARD_VALUE + VOLTAGE_PUSH_THRESHOLD_HIGH or last_v < VOLTAGE_STANDARD_VALUE - VOLTAGE_PUSH_THRESHOLD_LOW ){
        // voltage detected outside of defined thresholds; immediate push to Domoticz via MQTT
        Serial.println(last_v);
        Serial.println("1");
        mqtt_str = "{\"idx\":" + String(VOLTAGE_IDX) + ",\"nvalue\": 1, \"svalue\":\"" + String(last_v) + "\"}"; 
        mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
        mqtt_client.publish(mqtt_publish_topic, mqtt_payload);
        voltage_push_triggered = 1; 
        last_t_voltage = millis();

    }else if(voltage_push_triggered){
      // if the voltage push has been triggered, check if reading has been restored within thresholds to immediately push an update
      // ensures best accuracy of the duration recording of an over/under-volt event
      if(last_v < VOLTAGE_STANDARD_VALUE + VOLTAGE_PUSH_THRESHOLD_HIGH or last_v > VOLTAGE_STANDARD_VALUE - VOLTAGE_PUSH_THRESHOLD_LOW ){
        Serial.println(last_v);
        Serial.println("2");
        // reset voltage push triggered
        mqtt_str = "{\"idx\":" + String(VOLTAGE_IDX) + ",\"nvalue\": 1, \"svalue\":\"" + String(last_v) + "\"}"; 
        mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
        mqtt_client.publish(mqtt_publish_topic, mqtt_payload);        
        voltage_push_triggered = 0;
        last_t_voltage = millis();
      }
    }else if(millis() - last_t_voltage > VOLTAGE_UPDATE_TIMER * 1000){
        // update with voltage readings at regular interval
        Serial.println(last_v);
        Serial.println("3");
        mqtt_str = "{\"idx\":" + String(VOLTAGE_IDX) + ",\"nvalue\": 1, \"svalue\":\"" + String(last_v) + "\"}"; 
        mqtt_str.toCharArray(mqtt_payload, mqtt_str.length() + 1);
        mqtt_client.publish(mqtt_publish_topic, mqtt_payload);
        last_t_voltage = millis();      
    }
  }
}
  
