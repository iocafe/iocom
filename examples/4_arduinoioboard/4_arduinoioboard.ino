#include <Arduino.h>
#include <eosalx.h>
#include <iocomx.h>

/*
  4_arduinoioboard
  IO board example uging Arduino IDE. Connection co control computer, select:
  - Ethernet/Wiznet
  - Ethernet/LWIP
  - Serial port.
 */

/* How control computer connects to this IO board. One of IOBOARD_CTRL_LISTEN_SOCKET,
   IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_LISTEN_SERIAL or IOBOARD_CTRL_CONNECT_SERIAL.
   This can be overridden in build.
 */
#ifndef IOBOARD_CTRL_CON
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET
#endif

/* Maximum number of connections. This can be overridden in build. If listening for socket,
   we allow two connections. If actively connecting socket to control computer or communicating
   through serial port, we need only one.
 */
#ifndef IOBOARD_MAX_CONNECTIONS
#define IOBOARD_MAX_CONNECTIONS (IOBOARD_CTRL_CON == IOBOARD_CTRL_LISTEN_SOCKET ? 2 : 1)
#endif

 /* Defaults for IO memory block sizes. Minimum IO memory block size
    is sizeof(osalStaticMemBlock). The minimum size is needed for static
    memory allocation.
  */
#ifndef IOBOARD_EXPORT_MBLK_SZ
#define IOBOARD_EXPORT_MBLK_SZ 256
#endif
#ifndef IOBOARD_IMPORT_MBLK_SZ
#define IOBOARD_IMPORT_MBLK_SZ 256
#endif

/* We set up static memory pool for the IO board, even if we would be running on system
   with dynamic memory allocation. This is useful since we are writing and testing
   functionality which needs to run on micro controller.
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        IOBOARD_EXPORT_MBLK_SZ, IOBOARD_IMPORT_MBLK_SZ)];


#ifdef STM32L476xx
#define N_LEDS 8
const os_int leds[] = {PB8, PA10, PB3, PB5, PB4, PA8, PA9, PC7}; 
#endif

#ifdef STM32F429xx
#define N_LEDS 3
const os_int leds[N_LEDS] = {PB0, PB14, PB7};
#endif

#ifndef N_LEDS
#define N_LEDS 3
const os_int leds[N_LEDS] = {PB0, PB14, PB7};
#endif

static os_int prev_command;

static void ioboard_callback(
    struct iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void toggle_leds(void);


void setup() 
{
    ioboardParams prm;
    os_int lednr;

    // initialize the digital pin as an output.
    for (lednr = 0; lednr < N_LEDS; lednr++)
    {
        pinMode(leds[lednr], OUTPUT);
    }

    // Set up serial port for trace output.
    Serial.begin(115200);
    while (!Serial)
    {
        // toggle_leds();
    }
    osal_console_write("Starting...\n");

    // Initialize OS abtraction layer and start flashes on socket.
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_socket_initialize(OS_NULL, 0);

    /* Set up parameters for the IO board. This is necessary since
       we are using static memory pool.
     */
    os_memclear(&prm, sizeof(prm));
    prm.ctrl_type = IOBOARD_CTRL_CON;
    // prm.socket_con_str = "127.0.0.1";
    prm.socket_con_str = "192.168.1.220";
    prm.serial_con_str = "COM3,baud=115200";
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = IOBOARD_EXPORT_MBLK_SZ;
    prm.receive_block_sz = IOBOARD_IMPORT_MBLK_SZ;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.auto_synchronization = OS_TRUE;

    /* Start communication.
     */
    ioboard_start_communication(&prm);
    prev_command = 0x10000;

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_import, ioboard_callback, OS_NULL);
}


void loop()
{
    os_int command;
   
    /* Keep the communication alive. The IO board uses one thread model, thus
       we need to call this function repeatedly.
     */
    ioc_run(&ioboard_communication);

    /* If we receive a "command" as 16 bit value in address 2. The command could start
       some operation of IO board. The command is eached back in address 2 to allow
       controller to know that command has been regognized.
     */
    command = ioc_getp_short(&ioboard_import, 10);
    if (command != prev_command) {
        // toggle_leds();
        /* if (command == 1) {
            osal_console_write("Command 1.\n");
        }
        if (command == 2) {
            osal_console_write("Command 2.\n");
        }
        if (command == 3) {
            osal_console_write("Command 3.\n");
        }
         */
        prev_command = command;
        // ioc_setp_short(&ioboard_export, 2, command);
    }
}


/**
****************************************************************************************************

  @brief Callback function.

  The iocontroller_callback() function is called when changed data is received from connection
  or when connection status changes. This is used to control 7 segment display LEDs in my 
  STM32L476 test.

  No heavy processing or printing data should be placed in callback. The callback should return
  quickly. The reason is that the communication must be able to process all data it receives,
  and delays here will cause connection buffers to fill up, which at worst could cause time shift
  like delay in communication.

  @param   mblk Pointer to the memory block object.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void ioboard_callback(
    struct iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    os_int s, e, i, n;
    os_uchar buf[N_LEDS];

    /* Get connection status changes.
     */
    if (end_addr >= 0 && start_addr < N_LEDS)
    {
        s = start_addr;
        e = end_addr;
        if (s < 0) s = 0;
        if (e >= N_LEDS) e = N_LEDS - 1;
        n = e - s + 1;

        ioc_read(mblk, s, buf, n);
        for (i = 0; i<n; i++)
        {
          digitalWrite(leds[s + i], buf[i] ? HIGH : LOW);
        }
    }
}


static void toggle_leds(void)
{
#if N_LEDS > 1
    static os_int lednr = 0;
    digitalWrite(leds[lednr], LOW);
    if (++lednr >= N_LEDS) lednr = 0;
    digitalWrite(leds[lednr], HIGH);
#else    
    static os_int state = 0;
    state = !state;
    digitalWrite(leds[0], state ? HIGH : LOW);
#endif
}

