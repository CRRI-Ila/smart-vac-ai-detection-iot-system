#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "credentials.h"
#include "parameters.h"
#include "profile.h"
#include "cert.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <AIAP_Project2_inferencing.h>

// Pin Config
#define LD3 13
#define LD2 15
#define LD1 12
#define SPK 2
#define PB1 0

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MPU6050
Adafruit_MPU6050 mpu;

// WiFi + MQTT
#if defined(HIVEMQS) || defined(MOSQUITTOS)
WiFiClientSecure espClient;
#else
WiFiClient espClient;
#endif

PubSubClient client(espClient);
char publish_topic[100];
char subscribe_topic[100];
unsigned long lastPublishTime = 0;

// States
String top_label = "";
String action_state = "waiting";
String received_command = "";

// Debounce
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
bool lastButtonState = HIGH;
bool buttonPressed = false;

// OLED Display
void oleddisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);  display.println("State: " + top_label);
  display.setCursor(0, 20); display.println("What's happening: " + received_command);
  display.display();
}

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  received_command = "";
  for (int i = 0; i < length; i++) received_command += (char)payload[i];

  if (String(topic).equalsIgnoreCase(subscribe_topic)) {
    action_state = received_command;

    if (received_command.equalsIgnoreCase("Charging")) {
      digitalWrite(LD1, LOW); 
      digitalWrite(LD2, HIGH); 
      digitalWrite(LD3, HIGH);
     
    }
    else if (received_command.equalsIgnoreCase("It is ON")) {
      digitalWrite(LD1, HIGH); 
      digitalWrite(LD2, LOW); 
      digitalWrite(LD3, LOW);
      
    }
    else if (received_command.equalsIgnoreCase("Clogged") 
    ) {
      digitalWrite(LD1, LOW); 
      digitalWrite(LD2, LOW); 
      digitalWrite(LD3, LOW);
     
    }
     else if ( received_command.equalsIgnoreCase("150% Power activated")) {
      playAlertTone();
      digitalWrite(LD1, LOW); 
      digitalWrite(LD2, LOW); 
      digitalWrite(LD3, LOW);
      playAlertTone();

     
    }
    else if (received_command.equalsIgnoreCase("It is turned off")) {
      digitalWrite(LD1, HIGH); 
      digitalWrite(LD2, HIGH); 
      digitalWrite(LD3, HIGH);
     
    }
  }
}

// WiFi Setup
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

// MQTT Setup
void set_topics() {
  sprintf(publish_topic, "%s/%s/%s", appname, clientid, device);
  sprintf(subscribe_topic, "DL/%s/%s/%s", appname, clientid, device);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientid, mqtt_username, mqtt_password)) {
      client.subscribe(subscribe_topic, QoS);
    } else {
      delay(5000);
    }
  }
}

void playAlertTone() {
  tone(SPK, 1000, 200);  // 1000 Hz for 200 ms
  delay(250);
  tone(SPK, 1500, 200);  // 1500 Hz for 200 ms
  delay(250);
  tone(SPK, 1200, 300);  // 1200 Hz for 300 ms
  delay(350);
  noTone(SPK);           // Make sure to stop tone
}
// Setup
void setup() {
  Serial.begin(115200);
  pinMode(LD1, OUTPUT); 
  pinMode(LD2, OUTPUT); 
  pinMode(LD3, OUTPUT);
  pinMode(SPK, OUTPUT); 
  pinMode(PB1, INPUT_PULLUP);

  digitalWrite(LD1, LOW); 
  digitalWrite(LD2, LOW); 
  digitalWrite(LD3, LOW); 
  digitalWrite(SPK, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) while (true);
  if (!mpu.begin()) while (true);

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  setup_wifi();

  #ifdef HIVEMQS
    espClient.setCACert(root_ca1);
  #endif
  #ifdef MOSQUITTOS
    espClient.setCACert(root_ca2);
  #endif

  client.setServer(mqtt_server, mqtt_port);
  client.setKeepAlive(keepalive);
  client.setCallback(callback);
  set_topics();

  oleddisplay();
}

// Main Loop
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};
  sensors_event_t a, g, temp;

  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += 3) {
    mpu.getEvent(&a, &g, &temp);
    buffer[i] = a.acceleration.x;
    buffer[i + 1] = a.acceleration.y;
    buffer[i + 2] = a.acceleration.z;
    delay(5);
  }

  signal_t signal;
  numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) return;

  top_label = result.classification[0].label;
  float top_score = result.classification[0].value;
  for (size_t ix = 1; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > top_score) {
      top_score = result.classification[ix].value;
      top_label = result.classification[ix].label;
    }
  }

  // Debounce PB1
  int reading = digitalRead(PB1);
  if (reading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) buttonPressed = (reading == LOW);
  lastButtonState = reading;

  // MQTT Publish
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime >= mintxinterval) {
    StaticJsonDocument<200> doc;
    doc["label"] = top_label;
    doc["button"] = buttonPressed;

    char jsonBuffer[200];
    size_t length = serializeJson(doc, jsonBuffer);

    if (length < sizeof(jsonBuffer)) {
      if (client.publish(publish_topic, (const uint8_t*)jsonBuffer, length, false)) {
        Serial.print("Published: ");
        Serial.println(jsonBuffer);
      } else {
        Serial.println("Publish failed");
      }
    }

    lastPublishTime = currentTime;
  }

  oleddisplay();
  delay(100);
}
