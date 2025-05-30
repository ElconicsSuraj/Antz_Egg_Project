#include "task_serial_json.h"
#include <ArduinoJson.h>
#include "globals.h"
#include "nextion_display.h"

void Task2(void *pvParameters) {
  pinMode(2, OUTPUT);

  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (Serial.available()) {
      digitalWrite(2, HIGH);

      String input = "";
      while (Serial.available()) {
        input += (char)Serial.read();
        delay(2);
      }
      input.trim();

      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, input);

      if (!error) {
        float length = doc["length"];
        float breadth = doc["breadth"];
        float weight = doc["eggpresent"];

        storedLength = String(length, 0);
        storedBreadth = String(breadth, 0);
        storedWeight = weight;

        if (xSemaphoreTake(lcdSemaphore, portMAX_DELAY)) {
          t0.setText(storedLength.c_str());
          t1.setText(storedBreadth.c_str());
         // t2.setText(String(storedWeight, 0).c_str());
          xSemaphoreGive(lcdSemaphore);
        }

        Serial.println("ACK");
      } else {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
      }

      digitalWrite(2, LOW);
    }
  }
}

void serialEvent() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(Task2Handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
