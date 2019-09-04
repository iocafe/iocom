#include <Arduino.h>
#include <eosalx.h>

/*
  4_ioboard_test.ino
  Include a IO board test app to build it within Arduino IDE. 
 */
 
/* The setup routine runs once when you press reset.
 */
void setup() 
{
    /* Set up serial port for trace output.
     */
    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Arduino starting...");

   /* Initialize the eosal library.
    */
    osal_initialize(OSAL_INIT_DEFAULT);
}

/* The loop routine runs over and over again forever.
 */
void loop() 
{
    /* Start included application.
     */
    osal_main(0,0);
}

/* Include code for the application 
 */
#include "/coderoot/iocom/examples/4_ioboard_test/code/ioboard_example.c"
