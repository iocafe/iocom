/*
  main-espidf-platformio.c, 5.9.2021/pekka
  Entry point when building with Visual Studio Code, PlatformIO and ESP-IDF framework.
 */
#ifdef OSAL_ESPIDF_FRAMEWORK
#include <eosal.h>
#include <eosalx.h>

void app_main()
{
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);

    while (OS_TRUE) 
    {
        if (osal_loop(osal_application_context)) osal_reboot(0);

        /* ESP32: Save some resources for other tasks.
         */
        osal_sleep(3);
    }
}

#endif