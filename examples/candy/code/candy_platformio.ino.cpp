# 1 "/tmp/tmpF2kFa_"
#include <Arduino.h>
# 1 "/coderoot/iocom/examples/candy/code/candy_platformio.ino"
#include <Arduino.h>
#include <eosalx.h>
#include <iocom.h>
#include <devicedir.h>
#include <pins.h>
#include <FreeRTOS.h>
# 16 "/coderoot/iocom/examples/candy/code/candy_platformio.ino"
void setup();
void loop();
#line 16 "/coderoot/iocom/examples/candy/code/candy_platformio.ino"
void setup()
{


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