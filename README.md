
# ESP32 LED Control and Web Interface

This project runs on an ESP32 to control an LED via HTTP requests and WebSocket communication. The ESP32 serves a web page along with supporting JavaScript and CSS files from SPIFFS, allowing you to toggle the LED and start/stop flashing with customizable durations. The project also supports Over-The-Air (OTA) updates.

## Repository Structure

```
.
├── data
│   ├── index.html
│   ├── script.js
│   └── style.css
└── Blue_Light_With_HTML.ino
```

*Note:* If you are using the Arduino IDE, ensure that your main sketch file is in the root (or adjust the folder structure accordingly).

## Prerequisites

- **Hardware:**  
  - An ESP32 development board  
  - A computer with a USB port

- **Software:**  
  - [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)  
  - [ESP32 Board Package](https://github.com/espressif/arduino-esp32) installed in the Arduino IDE  
  - [ESP32 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin) plugin for Arduino IDE (if using Arduino IDE)  
    > *For PlatformIO, the `data/` folder is automatically detected when using the "Upload File System Image" command.*

- **Dependencies (libraries):**  
  The project uses the following libraries. They are included with the ESP32 board package or can be installed via the Arduino Library Manager:  
  - `WiFi.h`  
  - `WebServer.h`  
  - `WebSocketsServer.h`  
  - `FS.h` and `SPIFFS.h`  
  - `ArduinoOTA.h`  
  - [ArduinoJson](https://arduinojson.org/) (install via Library Manager)

## Setup Instructions

### 1. Clone the Repository

Clone the repository to your local machine:

```bash
git clone https://github.com/Johannes-T-E/Blue_Light_With_HTML.git
cd Blue_Light_With_HTML
```

### 2. Update WiFi Credentials

Before building the sketch, make sure to update the WiFi credentials in the code. Open the main sketch file (e.g., main.cpp or your Arduino sketch file) and change the following lines:

```cpp
// WiFi credentials
const char* ssid = "Miss me";
const char* password = "with that one";
```

Replace "Miss me" and "with that one" with your actual WiFi SSID and password.

### 3. Install Required Tools

#### For Arduino IDE Users

1. **Install the ESP32 Board Package:**
    - Open the Arduino IDE.
    - Go to **File > Preferences**.
    - In the **Additional Boards Manager URLs** field, add:
      ```
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
      ```
    - Go to **Tools > Board > Boards Manager**.
    - Search for **ESP32** and install the package by Espressif Systems.

2. **Install the ESP32 Sketch Data Upload Plugin:**
    - Download the plugin from its [GitHub repository](https://github.com/me-no-dev/arduino-esp32fs-plugin).
    - Follow the installation instructions to place the plugin in the Arduino IDE’s tools folder.
    - Restart the Arduino IDE.

3. **Install the ArduinoJson Library:**
    - In the Arduino IDE, go to **Sketch > Include Library > Manage Libraries**.
    - Search for **ArduinoJson** by Benoit Blanchon and install the latest version.

#### For PlatformIO Users

1. **Install PlatformIO:**
    - Install PlatformIO as a plugin for VSCode or use the PlatformIO IDE.
    - Create a new PlatformIO project for the ESP32 and copy the source code and data folder into your project.

2. **Install the ArduinoJson Library:**
    - Add the library to your `platformio.ini` file:
      ```ini
      lib_deps = 
        bblanchon/ArduinoJson
      ```

### 4. Uploading the Sketch and File System

#### Arduino IDE

1. **Upload the Sketch:**
    - Select the correct Board and Port under **Tools**.
    - Click the **Upload** button to flash the ESP32.

2. **Upload the SPIFFS Data:**
    - After the sketch has been uploaded, go to **Tools > ESP32 Sketch Data Upload**.
    - This will upload the contents of the `data` folder to the ESP32’s SPIFFS.

#### PlatformIO

1. **Upload the Sketch:**
    - Run the upload command (usually via the PlatformIO toolbar or using `pio run --target upload`).

2. **Upload the File System Image:**
    - Run the command:
      ```bash
      pio run --target uploadfs
      ```
    - This will upload the contents of the `data` folder to the ESP32’s SPIFFS.

### 5. Access the Web Interface

- Open the Serial Monitor at 115200 baud to view the debug messages.
- Once connected to WiFi, the Serial Monitor will display the ESP32’s IP address.
- Open a browser and navigate to the IP address (e.g., `http://192.168.x.x`).
- The web interface should load and allow you to toggle the LED, start flashing, or stop flashing.

## OTA Updates

The code supports ArduinoOTA for over-the-air updates. Once OTA is configured and running, you can update your firmware wirelessly.

## Troubleshooting

- **SPIFFS Mount Error:**
  - Ensure that the `data` folder exists and contains `index.html`, `script.js`, and `style.css`. Also, verify that the ESP32 Sketch Data Upload tool is correctly installed.

- **WiFi Connection Issues:**
  - Double-check your WiFi credentials and ensure that your ESP32 is within range of your network.

- **Library Issues:**
  - Ensure all required libraries are installed via the Arduino Library Manager or PlatformIO’s dependency management.
