#ifndef NEXTION_DISPLAY_H
#define NEXTION_DISPLAY_H

#include <Nextion.h>

extern NexText t0;
extern NexText t1;
extern NexText t2;
extern NexTouch *nex_listen_list[];

void initNextionTextFields();

#endif
