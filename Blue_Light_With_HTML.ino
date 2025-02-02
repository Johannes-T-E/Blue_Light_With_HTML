#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Miss me";
const char* password = "with that one";

// LED configuration
const int blueLedPin = 2; // Confirm GPIO2 is correct for your board
bool ledState = false;

// Define LED active state
const bool LED_ACTIVE_HIGH = true; // Set to 'true' if LED is active HIGH, 'false' otherwise

// Flashing variables
bool isFlashing = false;
unsigned long previousMillis = 0;
unsigned long onDuration = 1000;  // Default 1 second
unsigned long offDuration = 1000; // Default 1 second

// Initialize Web Server and WebSocket Server
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); // Port 81 for WebSockets

// Function declarations
void handleFileRequest();
String getContentType(String filename);
void handleToggle();
void handleStartFlash();
void handleStopFlash();
void handleSetFlash();
void handleStatus();
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

// Function to broadcast LED state
void broadcastLedState() {
  String state = ledState ? "ON" : "OFF";
  String message = "{\"status\":\"" + state + "\"}";
  webSocket.broadcastTXT(message);
  Serial.println("Broadcasting LED state: " + state);
}

void setup() {
  Serial.begin(115200);
  pinMode(blueLedPin, OUTPUT);
  // Initialize LED to OFF based on active state
  digitalWrite(blueLedPin, LED_ACTIVE_HIGH ? LOW : HIGH);
  Serial.println("Starting ESP32 LED Control");

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error: Failed to mount SPIFFS!");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize OTA
  ArduinoOTA.begin();
  Serial.println("OTA ready");

  // Initialize WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started on port 81");

  // Define routes
  server.on("/", handleFileRequest);                     // Serve index.html
  server.on("/style.css", handleFileRequest);            // Serve CSS file
  server.on("/script.js", handleFileRequest);            // Serve JS file
  server.on("/toggle", HTTP_POST, handleToggle);          // Handle LED toggle (POST)
  server.on("/startFlash", HTTP_POST, handleStartFlash);  // Start flashing (POST)
  server.on("/stopFlash", HTTP_POST, handleStopFlash);    // Stop flashing (POST)
  server.on("/setFlash", HTTP_POST, handleSetFlash);      // Set flash durations (POST)
  server.on("/status", HTTP_GET, handleStatus);           // Get LED status (GET) - Optional

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();

  // Handle flashing
  if (isFlashing) {
    unsigned long currentMillis = millis();
    if (ledState && (currentMillis - previousMillis >= onDuration)) {
      ledState = false;
      digitalWrite(blueLedPin, LED_ACTIVE_HIGH ? LOW : HIGH); // Turn LED OFF
      previousMillis = currentMillis;
      Serial.println("LED OFF (Flashing)");
      // Do not broadcast during flashing
    }
    else if (!ledState && (currentMillis - previousMillis >= offDuration)) {
      ledState = true;
      digitalWrite(blueLedPin, LED_ACTIVE_HIGH ? HIGH : LOW); // Turn LED ON
      previousMillis = currentMillis;
      Serial.println("LED ON (Flashing)");
      // Do not broadcast during flashing
    }
  }
}

void handleFileRequest() {
  String path = server.uri();
  if (path == "/") path = "/index.html"; // Default file

  Serial.println("Handling file request for: " + path);

  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    String contentType = getContentType(path);
    Serial.println("Serving file: " + path + " with content type: " + contentType);
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "404: File Not Found");
    Serial.println("File not found: " + path);
  }
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}

// Toggle LED state
void handleToggle() {
  Serial.println("Received /toggle request");

  if (isFlashing) {
    Serial.println("Stopping flashing before toggling");
    isFlashing = false;
  }

  ledState = !ledState;
  digitalWrite(blueLedPin, ledState ? (LED_ACTIVE_HIGH ? HIGH : LOW) : (LED_ACTIVE_HIGH ? LOW : HIGH)); // Active HIGH/LOW
  String state = ledState ? "ON" : "OFF";
  server.send(200, "application/json", "{\"status\":\"" + state + "\"}");
  Serial.println("LED is " + state);
  
  // Broadcast the new state to all connected WebSocket clients
  broadcastLedState();
}

// Start flashing LED
void handleStartFlash() {
  Serial.println("Received /startFlash request");

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Bad Request\"}");
    Serial.println("No body in /startFlash request");
    return;
  }

  // Parse JSON using ArduinoJson
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    Serial.println("Invalid JSON in /startFlash request");
    return;
  }

  if (!doc.containsKey("onDuration") || !doc.containsKey("offDuration")) {
    server.send(400, "application/json", "{\"error\":\"Missing Parameters\"}");
    Serial.println("Missing parameters in /startFlash request");
    return;
  }

  onDuration = doc["onDuration"];
  offDuration = doc["offDuration"];

  // Validate durations
  if (onDuration <= 0 || offDuration <= 0) {
    server.send(400, "application/json", "{\"error\":\"Durations must be positive integers\"}");
    Serial.println("Invalid duration values in /startFlash request");
    return;
  }

  Serial.println("Parsed onDuration: " + String(onDuration) + " ms, offDuration: " + String(offDuration) + " ms");

  isFlashing = true;
  server.send(200, "application/json", "{\"status\":\"Flashing\"}");
  Serial.println("Started flashing");
}

// Stop flashing LED
void handleStopFlash() {
  Serial.println("Received /stopFlash request");

  if (isFlashing) {
    isFlashing = false;
    server.send(200, "application/json", "{\"status\":\"Flashing Stopped\"}");
    Serial.println("Stopped flashing");
    
    // Set LED to OFF after stopping flashing
    ledState = false;
    digitalWrite(blueLedPin, LED_ACTIVE_HIGH ? LOW : HIGH); // Turn LED OFF
  } else {
    server.send(200, "application/json", "{\"status\":\"Not Flashing\"}");
    Serial.println("Flash was not active");
  }
}

// Set flash durations
void handleSetFlash() {
  Serial.println("Received /setFlash request");

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Bad Request\"}");
    Serial.println("No body in /setFlash request");
    return;
  }

  // Parse JSON using ArduinoJson
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    Serial.println("Invalid JSON in /setFlash request");
    return;
  }

  if (!doc.containsKey("onDuration") || !doc.containsKey("offDuration")) {
    server.send(400, "application/json", "{\"error\":\"Missing Parameters\"}");
    Serial.println("Missing parameters in /setFlash request");
    return;
  }

  onDuration = doc["onDuration"];
  offDuration = doc["offDuration"];

  // Validate durations
  if (onDuration <= 0 || offDuration <= 0) {
    server.send(400, "application/json", "{\"error\":\"Durations must be positive integers\"}");
    Serial.println("Invalid duration values in /setFlash request");
    return;
  }

  Serial.println("Set flash durations to ON: " + String(onDuration) + " ms, OFF: " + String(offDuration) + " ms");

  server.send(200, "application/json", "{\"status\":\"Flash Durations Set\"}");
}

// Handle status request (Optional)
void handleStatus() {
  String state = ledState ? "ON" : "OFF";
  String json = "{\"status\":\"" + state + "\"}";
  server.send(200, "application/json", json);
  Serial.println("Status requested: " + state);
}

// WebSocket event handler
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send current LED state upon connection
        String state = ledState ? "ON" : "OFF";
        String message = "{\"status\":\"" + state + "\"}";
        webSocket.sendTXT(num, message);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      // Optionally handle incoming messages from clients
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      break;
    default:
      break;
  }
}
