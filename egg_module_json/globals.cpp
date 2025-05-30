#include "globals.h"

TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
SemaphoreHandle_t lcdSemaphore = NULL;

double storedWeight = 0;
String storedLength = "";
String storedBreadth = "";
