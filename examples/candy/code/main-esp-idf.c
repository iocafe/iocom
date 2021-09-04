#ifdef OSAL_ESPIDF_FRAMEWORK
/*
  main.c
  To build it within Visual Studio Code and PlatformIO.
 */

#include <eosal.h>
#include <eosalx.h>
// #include <iocom.h>
//#include <devicedir.h>
//#include <pins.h>
//#include <FreeRTOS.h>


#ifdef OSAL_ESPIDF_FRAMEWORK

void app_main()
{
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);

    while (OS_TRUE) 
    {
        if (osal_loop(osal_application_context)) osal_reboot(0);

        /* ESP-IDF 3.X/MELIFE test board : We cannot write too fast through WiFi, WiFi will lock up.
        */
        osal_sleep(3);
    }
}

#else

/* The setup routine runs once when you press reset.
 */
void setup()
{
   /* Initialize the eosal library.
    */
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);
}

/* The loop routine runs over and over again forever.
 */
void loop()
{
    /* Forward loop call to osal_loop(). Reboot if osal_loop returns "no success".
     */
    if (osal_loop(osal_application_context)) osal_reboot(0);

    /* ESP-IDF 3.X/MELIFE test board : We cannot write too fast through WiFi, WiFi will lock up.
     */
#ifdef ESP_PLATFORM
    osal_sleep(3);
#endif
}

#endif
#endif