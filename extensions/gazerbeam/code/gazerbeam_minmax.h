/**

  @file    common/pins_gazerbeam_minmax.h
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.3.2020

  Keep sliding minimum of N last values.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Filtering buffer size.
 */
#define MAX_GAZERBEAM_LAYERS 8

/* GazerbeamReceiver state structure. Typically allocated as global flat structure.
 */
typedef struct GazerbeamBuffer
{
    /* Buffers for tracking minimum or maximum signal value.
     */
    GAZERBEAM_VALUE x[MAX_GAZERBEAM_LAYERS];
    GAZERBEAM_VALUE z[MAX_GAZERBEAM_LAYERS];

    /* Just internal counter for filling the x and z buffers.
     */
    os_int run_count;

    /* How many AD values are used to keep track maximum and minimum
     * signal levels. N = 2^nro_layers.
     */
    os_int nro_layers;

    /* Looking for maximum or minimum signal value?
     */
    os_boolean find_max;
}
GazerbeamBuffer;


/* Find out minimum or maximum value of the last N samples.
 */
GAZERBEAM_VALUE gazerbeam_minmax(
    GazerbeamBuffer *gbb,
    GAZERBEAM_VALUE x);

/* Initialize whole buffer with specific value.
*/
void gazerbeam_fill_minmax(
    GazerbeamBuffer *gbb,
    GAZERBEAM_VALUE x);
