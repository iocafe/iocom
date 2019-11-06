/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s, gina_init_prm_t *prm)
{
    os_memclear(s, sizeof(gina_t));
    s->up.hdr.handle = up;
    s->up.hdr.n_signals = 12;
    s->up.hdr.first_signal = (iocSignal*)((os_char*)gina + offsetof(struct gina_t, up.dip_switch_3));

 /* dip_switch_3 */
    s->up.dip_switch_3.handle = up;
    s->up.dip_switch_3.ptr = "dip_switch_3";

 /* dip_switch_4 */
    s->up.dip_switch_4.handle = up;
    s->up.dip_switch_4.ptr = "dip_switch_4";

 /* touch_sensor */
    s->up.touch_sensor.handle = up;
    s->up.touch_sensor.ptr = "touch_sensor";

 /* potentiometer */
    s->up.potentiometer.handle = up;
    s->up.potentiometer.ptr = "potentiometer";

 /* A */
    s->up.A.handle = up;
    s->up.A.ptr = "A";

 /* B */
    s->up.B.handle = up;
    s->up.B.ptr = "B";

 /* C */
    s->up.C.handle = up;
    s->up.C.ptr = "C";

 /* D */
    s->up.D.handle = up;
    s->up.D.ptr = "D";

 /* E */
    s->up.E.handle = up;
    s->up.E.ptr = "E";

 /* F */
    s->up.F.handle = up;
    s->up.F.ptr = "F";

 /* G */
    s->up.G.handle = up;
    s->up.G.ptr = "G";

 /* H */
    s->up.H.handle = up;
    s->up.H.ptr = "H";

    s->down.hdr.handle = down;
    s->down.hdr.n_signals = 4;
    s->down.hdr.first_signal = (iocSignal*)((os_char*)gina + offsetof(struct gina_t, down.seven_segment));

 /* seven_segment */
    s->down.seven_segment.handle = down;
    s->down.seven_segment.ptr = "seven_segment";

 /* servo */
    s->down.servo.handle = down;
    s->down.servo.ptr = "servo";

 /* dimmer_led */
    s->down.dimmer_led.handle = down;
    s->down.dimmer_led.ptr = "dimmer_led";

 /* led_builtin */
    s->down.led_builtin.handle = down;
    s->down.led_builtin.ptr = "led_builtin";

};

static const iocMblkSignalHdr *gina_mblk_list[] =
{
  &gina.up.hdr,
  &gina.down.hdr
};

const iocDeviceHdr gina_hdr = {gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
