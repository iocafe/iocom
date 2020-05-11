/* This file is gerated by parameters-to-c.py script, do not modify. */

  {
    {"persistent", handle, 6, CANDY_PERSISTENT_PBLK_SZ, (iocparameter*)&candy.persistent.size},
    {0, 10, OS_STR, handle}, /* size */
    {11, 1, OS_UCHAR, handle}, /* quality */
    {13, 1, OS_UCHAR, handle}, /* brightness */
    {15, 1, OS_UCHAR, handle}, /* contrast */
    {17, 1, OS_UCHAR, handle}, /* hue */
    {19, 1, OS_UCHAR, handle} /* saturation */
  },

  {
    {"volatile", handle, 1, CANDY_VOLATILE_PBLK_SZ, (iocparameter*)&candy.volatile.on},
    {0, 1, OS_BOOLEAN, handle} /* on */
  },

