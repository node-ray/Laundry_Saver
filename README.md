# Laundry_Saver
Laundry notifications (using [Domoticz](https://domoticz.com/)/[Pushover](https://pushover.net/)) and power monitoring with the [M5stack ATOM Socket](https://shop.m5stack.com/products/atom-socket-kit-hlw8023-jp-us) based on [yoggy/mqtt-m5atom-socket](https://github.com/yoggy/mqtt-m5atom-socket)


![Laundry_Saver](https://github.com/node-ray/Laundry_Saver/blob/main/Laundry_Saver.png)

# Usage

1. Clone the repository or download and extract the zip file for the repository
2. Open LaundrySaver-main.ino in [Arduino IDE](https://www.arduino.cc/en/software)
3. Add the URL https://dl.espressif.com/dl/package_esp32_index.json to the list of board managers under **File -> Preferences** to add support for ESP32
4. Install the M5Atom library from **Tools -> Library Manager**
5. Select your the ATOM Lite board from **Tools -> Board**
6. Select the COM port from **Tools -> Port**. If no COM port is listed, install the [USB to Serial drivers](https://ftdichip.com/drivers/vcp-drivers/) 
7. In the config.h file, configure the wifi network and password, MQTT server IP, username, and password
8. Additionally, configure the Domoticz IDXs to be used for status (virtual switch), energy (energy virtual sensor), and voltage (voltage virtual sensor)
9. Upload Arduino sketch to the to the ATOM Lite connected to the ATOM socket unit. If everything is configured correctly, emergy/voltage readings and laundry status
10. Configure desired notifications in Domoticz for laundry status, and configure your notification client of choice (such as Pushover or Telegram)

Additional information can be found [here](https://www.node-ray.com/LaundrySaver)
