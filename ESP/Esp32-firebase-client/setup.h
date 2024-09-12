#ifndef SETUP_H
#define SETUP_H

// WiFi
#include <WiFi.h>
// Managing time values
#include "time.h"

extern const char* ntpServer;

/*
*@brief Function to setup the ntp connection
*/
void setupNTP();

void setupUART();

#endif