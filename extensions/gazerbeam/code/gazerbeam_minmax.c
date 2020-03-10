/**

  @file    selectwifi.c
  @brief   Set wifi network name and password over blue tooth or serial port.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.2.2020

  Configure microcontroller WiFi, etc, over blue tooth connection.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "gazerbeam.h"


/**
****************************************************************************************************

  @brief Find out minimum or maximum value of the last N samples.

  Keep sliding minimum of N last values. The filtering window is "coarse" in sense that
  individual values are lost and time when value stops effecting is window. This simplification
  is done for computational speed, we want to find minimum of a lot of values within interrupt
  handler routing.

  Output doesn't respond immediately to input, output comes with delay dependent on N. Still it
  can keep track of sliding minimum/maximum value of an analog input. For example to keep minimum
  of 256 values from A/D converter, maximum loop is 8 times and RAM buffer needed for 16 values.
  Time window length is square of processor load and memory use, what is good: Fast and small
  enough to run easy in interrupt handler, like on small microcontroller on 20kHz.

    #define MAX_GAZERBEAM_LAYERS 10
    #define GAZERBEAM_VALUE_TYPE os_int

    typedef struct GazerbeamBuffer
    {
        GAZERBEAM_VALUE_TYPE x[MAX_GAZERBEAM_LAYERS];
        GAZERBEAM_VALUE_TYPE z[MAX_GAZERBEAM_LAYERS];
        os_int run_count;
        os_int nro_layers;
        os_boolean find_max;
    }
    GazerbeamBuffer;

  @param   db Pointer to the Gazerbeam structure.
  @param   x New signal value.
  @return  Minimum value among the memorized values.

****************************************************************************************************
*/
GAZERBEAM_VALUE_TYPE gazerbeam_minmax(
    GazerbeamBuffer *gbb,
    GAZERBEAM_VALUE_TYPE x)
{
    GAZERBEAM_VALUE_TYPE *X, *Z;
    os_int max_layer, i, n;

    #define GB_MIN(a,b) (a < b ? a : b)
    #define GB_MAX(a,b) (a > b ? a : b)

    static int run_count = 0;

    X = gbb->x;
    Z = gbb->z;
    max_layer = gbb->nro_layers - 1;

    for (n = 0; n < max_layer; n++)
        if ((run_count & (1 << n)) == 0) break;

    if (gbb->find_max)
    {
        for (i = n; i > 0; i--)
        {
            Z[i] = X[i];
            X[i] = GB_MAX(X[i - 1], Z[i - 1]);
        }
    }
    else
    {
        for (i = n; i > 0; i--)
        {
            Z[i] = X[i];
            X[i] = GB_MIN(X[i - 1], Z[i - 1]);
        }
    }

    Z[0] = X[0];
    X[0] = x;

    if (++run_count >= (1 << max_layer)) run_count = 0;

    if (gbb->find_max)
    {
        return GB_MAX(X[max_layer], Z[max_layer]);
    }
    else
    {
        return GB_MIN(X[max_layer], Z[max_layer]);
    }
}


