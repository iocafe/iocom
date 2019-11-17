#include <Arduino.h>
#include <eosalx.h>
#include <iocom.h>
#include <devicedir.h>
#include <pins.h>
#include <FreeRTOS.h>


static int prio_set;

/*
  gina_platformio.ino
  To build it within Visual Studio Code and PlatformIO. 
 */
 
/* The setup routine runs once when you press reset.
 */
void setup() 
{
    /* Set up serial port for trace output.
     */
    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Gina IO board starting...");

   /* Initialize the eosal library.
    */
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);
}

/* The loop routine runs over and over again forever.
 */
void loop() 
{
    if (!prio_set) {osal_thread_set_priority(OSAL_THREAD_PRIORITY_TIME_CRITICAL); prio_set=1;}

    /* Forward loop call to osal_loop(). Reboot if osal_loop returns "no success".
     */
    if (osal_loop(osal_application_context)) osal_reboot(0);
}
