#include <Arduino.h>
#include "task_scale.h"
#include "task_serial_json.h"
#include "nextion_display.h"
#include "globals.h"

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 15, 17);

  nexInit();
  initNextionTextFields();

  lcdSemaphore = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, &Task1Handle, 0);
  xTaskCreatePinnedToCore(Task2, "Task2", 10000, NULL, 3, &Task2Handle, 1);
}

void loop() {
  nexLoop(nex_listen_list);
}
