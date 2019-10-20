#include <Arduino.h>
// #include <FreeRTOS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <eosal.h>
#include <eosalx.h>

/*
  iodomainctrl
  .ino file to build basic IO domain contoller with Visual Studio Code + Platform IO.
*/
void setup() 
{
    /* Set up serial port for trace output.
     */
    Serial.begin(115200);
    while (!Serial);
    Serial.println("IO domain controller starting (Visual Studio Code + Platform IO + Arduino mode)...");

    /* Start the very simple server.
     */
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);
}

/* The loop function is called repeatedly while the device runs.
 */
void loop()
{
    if (osal_loop(osal_application_context)) osal_reboot(0);
}