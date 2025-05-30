#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <Nextion.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

extern TaskHandle_t Task1Handle;
extern TaskHandle_t Task2Handle;
extern SemaphoreHandle_t lcdSemaphore;

extern double storedWeight;
extern String storedLength;
extern String storedBreadth;

#endif
