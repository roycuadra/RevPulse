#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const char *ssid = "ESP-Relay";
const char *password = "admin12345";

const int relayPin = 2; // GPIO pin connected to the relay

ESP8266WebServer server(80);

int onDelay = 1000; // Default on delay in milliseconds
int offDelay = 1000; // Default off delay in milliseconds

bool isBlinking = false;

String readHtmlFile(String filename) {
  String content = "";
  File file = SPIFFS.open(filename, "r");
  
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }

  while (file.available()) {
    content += file.readStringUntil('\n');
  }

  file.close();
  return content;
}

void handleRoot() {
  String html = readHtmlFile("/index.html");
  server.send(200, "text/html", html);
}

void handleStart() {
  isBlinking = true;
  server.send(200, "text/plain", "Blinking started");
}

void handleStop() {
  isBlinking = false;
  digitalWrite(relayPin, LOW); // Turn off relay when blinking is stopped
  server.send(200, "text/plain", "Blinking stopped");
}

void handleSlider() {
  onDelay = server.arg("delay").toInt();
  offDelay = onDelay; // Set both on and off delay to the same value
  server.send(200, "text/plain", "Delay updated");
}

void setup() {
  Serial.begin(115200);

  // Set up ESP8266 as an access point
  WiFi.softAP(ssid, password);

  // IP Address of the access point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Hotspot IP Address: ");
  Serial.println(myIP);

  // Setup relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Initialize relay as OFF

  // Initialize SPIFFS
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS initialized successfully");
  } else {
    Serial.println("Failed to initialize SPIFFS");
  }

  // Setup web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/start", HTTP_GET, handleStart);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/slider", HTTP_GET, handleSlider);
  server.begin();
}

void loop() {
  server.handleClient();

  if (isBlinking) {
    digitalWrite(relayPin, HIGH);
    delay(onDelay);
    digitalWrite(relayPin, LOW);
    delay(offDelay);
  }
}
