#include <Arduino.h>
#include "HX711.h"
#include <freertos/task.h>
#include <Nextion.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

// HX711 load cell
HX711 scale;



// Variable to track the current page
int currentPage = 0; // Default to page 0

// Initialize LED pin for example purposes
const int ledPin = 2;
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
const int BUTTON_PIN = 5;
const unsigned long DEBOUNCE_DELAY = 50;

volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

// Variable to store the entered number
String enteredNumber = "";
double storedWeight;
String storedLength;
String storedBreadth;

// Task handles
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
TaskHandle_t EnterNumberTaskHandle = NULL;
TaskHandle_t SendDataTaskHandle = NULL;
SemaphoreHandle_t lcdSemaphore;


NexText t0 = NexText(1, 3, "a0"); // Length
NexText t1 = NexText(1, 4, "a1"); // Bredth
NexText t2 = NexText(1, 5, "a2"); // Weight


NexTouch *nex_listen_list[] = {

  NULL
};  // Array of Nextion touch objects





void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 15, 17);  // RX pin 16, TX pin 17
  // serial.println("Serial2 initialized");

  // Initialize Nextion display
  nexInit();

  lcdSemaphore = xSemaphoreCreateMutex();

  // Create FreeRTOS tasks
  xTaskCreatePinnedToCore(
    Task1,
    "Task1",
    10000,  // Increased stack size
    NULL,
    1,
    &Task1Handle,
    0 // Run on core 0
  );

  xTaskCreatePinnedToCore(
    Task2,
    "Task2",
    10000,  // Increased stack size
    NULL,
    3,
    &Task2Handle,
    1 // Run on core 1
  );


}

void loop() {
  // Listen for touch events
  nexLoop(nex_listen_list);
}

void Task1(void *pvParameters) {
  (void) pvParameters;

  // Initialize the scale
  // serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(1990);
  scale.tare();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  while (1) {
    // serial.println("Task 1 is running");
    nexLoop(nex_listen_list);  // Check for any button presses on the Nextion display
    if (buttonPressed) {
      tare();
      // serial.println("Button Pressed Tare");
      buttonPressed = false;
    }

    scale.power_down();
    delay(500);
    scale.power_up();

    double weight =  random(1, 100);  //scale.get_units(10); // Get weight in double floating point
    // serial.print("Weight:\t");
    // serial.println(weight, 0); // Print weight with 2 decimal places
    storedWeight = weight;
    if (xSemaphoreTake(lcdSemaphore, portMAX_DELAY)) {
      if (weight < 3.0) {
        weight = 0;
      }

      t2.setText(String(weight, 0).c_str());

      xSemaphoreGive(lcdSemaphore);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


// SEND INPUT IN THIS FORMAT // {"length":10,"breadth":5,"weight":3}

void Task2(void *pvParameters) {
  (void) pvParameters;

  pinMode(2, OUTPUT); // Set pin 2 as output (debug LED)

  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait until notified

    if (Serial.available()) {
      digitalWrite(2, HIGH);

      String input = "";
      while (Serial.available()) {
        char incomingChar = Serial.read();
        input += incomingChar;
        delay(2);  // Allow serial buffer to fill
      }

      input.trim();

      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, input);

      if (!error) {
        float length = doc["length"];
        float breadth = doc["breadth"];
        float weight = doc["weight"];

        Serial.printf("Length: %.2f, Breadth: %.2f, Weight: %.2f\n", length, breadth, weight);

        // Store values
        storedLength = String(length, 0);
        storedBreadth = String(breadth, 0);
        storedWeight = weight;

        // Update Nextion screen
        if (xSemaphoreTake(lcdSemaphore, portMAX_DELAY)) {
          t0.setText(String(length, 0).c_str());
          t1.setText(String(breadth, 0).c_str());
          t2.setText(String(weight, 0).c_str());
          xSemaphoreGive(lcdSemaphore);
        }

        // Send ACK
        Serial.println("ACK");
      } else {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
      }

      digitalWrite(2, LOW);
    }
  }
}




void tare() {
  if (xSemaphoreTake(lcdSemaphore, portMAX_DELAY)) {
    scale.tare();
    t2.setText("Tare");
    delay(1000);
    xSemaphoreGive(lcdSemaphore);
  }
}

void buttonISR() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime >= DEBOUNCE_DELAY) {
    buttonPressed = true;
  }
  lastDebounceTime = currentMillis;
}

// ISR to notify Task2 when serial data is available
void serialEvent() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(Task2Handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

