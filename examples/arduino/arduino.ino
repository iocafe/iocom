#define IOCOM_IOBOARD
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SERIAL
#include "iocomx.h"
#include "config/include/generic/signals.h"
#include "config/include/generic/signals_info_mblk.h"

/* Include signal configuration C code.
 */
#include "config/include/generic/signals.c"
#include "config/include/generic/signals_info_mblk.c"

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Use static memory pool. 
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        ARDUINO_EXP_MBLK_SZ, ARDUINO_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)];
        
void setup() 
{
    ioboardParams prm;
    osal_initialize(OSAL_INIT_DEFAULT);

    /* We use quiet mode. Since Arduino UNO has only one serial port, we need it for
       communication. We cannot have any trace, etc. prints to serial port. 
     */
    osal_quiet(OS_TRUE);

    /* Initialize serial communication.
     */
    osal_serial_initialize();

    /* Set up parameters for the IO board.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_IFACE;
    prm.device_name = IOBOARD_DEVICE_NAME;
    prm.device_nr = 1;
    prm.network_name = "cafenet";
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.serial_con_str = "ttyS30";
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.exp_mblk_sz = ARDUINO_EXP_MBLK_SZ;
    prm.imp_mblk_sz = ARDUINO_IMP_MBLK_SZ;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.device_info = ioapp_signals_config;
    prm.device_info_sz = sizeof(ioapp_signals_config);

    /* Start communication.
     */
    ioboard_start_communication(&prm);
}

void loop() 
{
    os_timer ti;
    static os_timer start_t = 0;
    static os_char state = 0;
    os_int timeout_ms;

    /* Keep the communication alive. If data is received from communication, the
       ioboard_callback() will be called. Move data data synchronously
       to incomong memory block.
     */
    os_get_timer(&ti);
    ioc_run(&ioboard_root);
    ioc_receive(&ioboard_imp);

    /* Get inputs we are using.
     */
    int led = ioc_get(&arduino.imp.LED);
    int run = ioc_get(&arduino.imp.RUN);
    int speed = ioc_get(&arduino.imp.SPEED);

    /* Modify state.
     */
    timeout_ms = 1000;
    if (speed > 0) {
        timeout_ms = 1000 / speed;
    }

    if (os_has_elapsed_since(&start_t, &ti, timeout_ms)) {
        if (++state > 3) state = 0;
        start_t = ti;
    }

    /* Set outputs.
     */
    ioc_set(&arduino.exp.SENSOR, state);
    ioc_set(&arduino.exp.SWITCH, !state);
    ioc_set(&arduino.exp.WHATEVER, osal_rand(0, 10000));

    /* Send changed data to iocom.
     */
    ioc_send(&ioboard_exp);
}
