#include "task_scale.h"
#include "HX711.h"
#include "globals.h"
#include "nextion_display.h"


const int LOADCELL_SCK_PIN = 4;
const int LOADCELL_DOUT_PIN = 16;
const int BUTTON_PIN = 5;
const unsigned long DEBOUNCE_DELAY = 50;
volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

HX711 scale;

void Task1(void *pvParameters) {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(2166.37);
  scale.tare();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  while (1) {
    if (buttonPressed) {
      tare();
      buttonPressed = false;
    }

    scale.power_down();
    delay(500);
    scale.power_up();

    double weight = scale.get_units(10);     // random(1, 100);
        if (abs(weight) < 0.5) weight = 0;
    storedWeight = weight;

    if (xSemaphoreTake(lcdSemaphore, portMAX_DELAY)) {
      t2.setText(String(weight, 0).c_str());
      xSemaphoreGive(lcdSemaphore);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
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
