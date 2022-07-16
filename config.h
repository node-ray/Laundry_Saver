#define wifi_ssid "wifi_network"
#define wifi_password "wifi_password"
#define mqtt_host "192.168.1.42"
#define mqtt_port 1883

#define ENERGY_UPDATE 1 // whether or not to update Domoticz energy IDX. set to 0 to disable
#define ENERGY_IDX 47 // Domoticz IDX to monitor energy usage
#define VOLTAGE_IDX 160 // Domoticz IDX to monitor energy usage
#define STATUS_IDX 48 // Domoticz IDX to signal when Laundry is running/done
#define ENERGY_UPDATE_TIMER 30 // how often (in seconds) to updat energy IDX

#define VOLTAGE_UPDATE 1 // whether or not to update Domoticz voltage IDX. set to 0 to disable
#define VOLTAGE_STANDARD_VALUE 120 // set this to 110, 120, 127 volts, or whatever works for your location
#define VOLTAGE_PUSH_THRESHOLD_LOW 5 // negative voltage deviance to alert on (push immediate update)
#define VOLTAGE_PUSH_THRESHOLD_HIGH 5 // postive voltage deviance to alert on (push immediate update)
#define VOLTAGE_PUSH_TIMER 5 // how often to (in seconds)check for (successive) out of range (under/overvolt) voltage events
#define VOLTAGE_UPDATE_TIMER 30 // how often (in seconds) to update the voltage IDX


#define mqtt_client_id "mqtt-m5atom-socket-0002"
#define mqtt_use_auth true
#define mqtt_username "mqtt_user"
#define mqtt_password "mqtt_password"

#define mqtt_publish_topic "domoticz/in" // MQTT topic to publish Domoticz values to
#define mqtt_update_topic "test/laundry" // MQTT topic to publish cycle changes to for troubleshooting or monitoring
