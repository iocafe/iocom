# 1 "/tmp/tmp232Uw5"
#include <Arduino.h>
# 1 "/coderoot/iocom/examples/gina/code/gina_platformio.ino"
#include <Arduino.h>
#include <eosalx.h>
#include <iocom.h>
#include <devicedir.h>
#include <pins.h>
#include <FreeRTOS.h>
# 15 "/coderoot/iocom/examples/gina/code/gina_platformio.ino"
void setup();
void loop();
#line 15 "/coderoot/iocom/examples/gina/code/gina_platformio.ino"
void setup()
{


    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Gina IO board starting...");



    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);
}



void loop()
{


    if (osal_loop(osal_application_context)) osal_reboot(0);



#ifdef ESP_PLATFORM
    os_sleep(3);
#endif
}