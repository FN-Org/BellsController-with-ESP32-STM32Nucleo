#include "setup.h"

const char* ntpServer = "pool.ntp.org";

/*
*@brief Function to setup the ntp connection
*/
void setupNTP() {
  // Configure the NTP client
  configTime(0, 0, ntpServer);
}

void setupUART() {
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
}

