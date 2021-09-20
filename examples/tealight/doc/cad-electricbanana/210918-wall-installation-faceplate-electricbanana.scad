$fn=30;
pcb_w = 166;
pcb_h = 83;
pcb_depth = 30.0;
box_w = 7.5 * 25.4;
box_h = 4.0 * 25.4;
box_depth = (2+1/8) * 25.4;
// box_wall_thickness = 2;
overlapx = 2.0;
overlapy = 5.0;
faceplate_thickness = 7;
skew_angle = 10;
skew_h = 3;
skew_box_w = 40;
cut_w = box_w - 40;
cut_h = box_h - 38;
cut_depth = 0.0;
rounding = 2;
holerounding = 1.0;

module screwhole()
{
    cylinder(1.5*faceplate_thickness, d=3.5, center=true);
    translate([0,0,4+faceplate_thickness/2]) cylinder(faceplate_thickness, d=7.7, center=true);
}


module screwholes()
{
    ngauge = 4;
    screw_x_step = (1 + 13/16) * 25.4;
    screw_delta_y = 83.3;

    for(x = [0 : ngauge-1])
    {
        translate([(x - (ngauge-1)/2) * screw_x_step, screw_delta_y/2, 0]) screwhole();
        translate([(x - (ngauge-1)/2) * screw_x_step, -screw_delta_y/2, 0]) screwhole();
    }
}

module backscrewhole(x, y)
{
    translate([x, y, 0]) cylinder(2.0*faceplate_thickness-1, d=2.5, center=true);
}

module backscrewholes()
{
    real_pcb_w = 140.2;
    real_pcb_h = 80.2;
    hole_dx = 15;
    hole_dy = 5;
    pcb_center_x = (pcb_w - real_pcb_w)/2 - 2;

    union() {
        backscrewhole(pcb_center_x + real_pcb_w/2 - hole_dx, real_pcb_h/2 - hole_dy);
        backscrewhole(pcb_center_x - real_pcb_w/2 + hole_dx, real_pcb_h/2 - hole_dy);
        backscrewhole(pcb_center_x + real_pcb_w/2 - hole_dx, -real_pcb_h/2 + hole_dy);
        backscrewhole(pcb_center_x - real_pcb_w/2 + hole_dx, -real_pcb_h/2 + hole_dy);

        hole_dx2 = 25;
        backscrewhole(pcb_center_x + real_pcb_w/2 - hole_dx2, real_pcb_h/2 - hole_dy);
        backscrewhole(pcb_center_x - real_pcb_w/2 + hole_dx2, real_pcb_h/2 - hole_dy);
        backscrewhole(pcb_center_x + real_pcb_w/2 - hole_dx2, -real_pcb_h/2 + hole_dy);
        backscrewhole(pcb_center_x - real_pcb_w/2 + hole_dx2, -real_pcb_h/2 + hole_dy);
    }
}


module bar(a)
{
    rotate([0,0,a]) cube([sqrt(cut_w*cut_w + cut_h*cut_h),5-2*holerounding,faceplate_thickness-cut_depth-2*holerounding], center=true);
}

module smoothbar(a)
{
    minkowski()
    {
        bar(a);
        sphere(holerounding);
    }
}

module bars()
{
    xn = 11;
    xstep = 1.21*cut_w / (xn-1);

    for(x = [0 : xn-1])
    {
        xx = (x - (xn-1)/2) * xstep;
        translate([xx, 0, faceplate_thickness/2-cut_depth]) {
            smoothbar(45);
            smoothbar(-45);
        }
    }
}

module cutbars()
{
    intersection() {
        cube([cut_w+0.1, cut_h+0.1, 2*faceplate_thickness], center=true);
        bars();
    }
}


module angularbaseblock()
{
    difference() 
    {
        cube([box_w + 2*overlapx - 2*rounding, box_h + 2*overlapy - 2*rounding, faceplate_thickness - 2*rounding], center=true);
        translate([0,box_h/2+overlapy,faceplate_thickness - skew_h - rounding]) rotate([-skew_angle,0,0]) cube([box_w + 2*overlapx, skew_box_w, faceplate_thickness], center=true);
        translate([0,-box_h/2-overlapy,faceplate_thickness - skew_h - rounding]) rotate([skew_angle,0,0]) cube([box_w + 2*overlapx, skew_box_w, faceplate_thickness], center=true);
        translate([box_w/2+overlapx,0,faceplate_thickness - skew_h - rounding]) rotate([0,skew_angle,0]) cube([skew_box_w, box_h + 2*overlapy, faceplate_thickness], center=true);
        translate([-box_w/2-overlapx,0,faceplate_thickness - skew_h - rounding]) rotate([0,-skew_angle,0]) cube([skew_box_w, box_h + 2*overlapy, faceplate_thickness], center=true);
    } 
}

module smoothbaseblock()
{
    translate([0,0,faceplate_thickness/2 - cut_depth/2]) 
    minkowski()
    {
        angularbaseblock();
        sphere(rounding);
    }
}


module baseblock()
{
    difference()
    {
        smoothbaseblock();
        screwholes();
        backscrewholes();
        cube([cut_w, cut_h, 2.1*faceplate_thickness], center=true);
    }
}

module baseblockwithbars()
{
    union()
    {
        baseblock();
        cutbars();
    }
}

lcd_pcb_w = 28.5;
lcd_pcb_h = 39.8;
lcd_visible_w = 25.2;
lcd_visible_h = 28.5;
lcd_framed_w = lcd_pcb_w + 4;
lcd_framed_h = lcd_pcb_h + 4;
lcs_pox_x = 62;
lcs_pox_y = -11.5;

module lcdblock()
{
    minkowski()
    {
        cube([lcd_framed_w,lcd_framed_h,faceplate_thickness-2*holerounding], center=true);
        sphere(holerounding);
    }
}

module lcdblock_with_cuts()
{
    union() {
        difference()
        {
            lcdblock();
            translate([0, 0, -1]) cube([lcd_pcb_w, lcd_pcb_h, faceplate_thickness], center=true);
            cube([lcd_visible_w, lcd_visible_h, faceplate_thickness+0.1], center=true);
        }
        translate([0, lcd_pcb_h/2, faceplate_thickness/2 - 2 - 3.75]) cube([lcd_framed_w, 2.5, 2.5], center=true);
    }
}

module baseblockwithlcd()
{
    union()
    {
        difference()
        {
            baseblockwithbars();
            translate([lcs_pox_x, lcs_pox_y, faceplate_thickness/2]) cube([lcd_pcb_w, lcd_pcb_h, faceplate_thickness+0.1], center=true);
        }

        translate([lcs_pox_x, lcs_pox_y, faceplate_thickness/2]) lcdblock_with_cuts();
    }
}

module finalblock()
{
    text = "";
    font = "Liberation Sans";
 
    difference() {
        intersection()
        {
            translate([0, 0, -0.8]) baseblockwithlcd();
            translate([0, 0, 25]) cube([2*pcb_w, 2*pcb_h, 50], center=true);
        }
        translate([0, 0, faceplate_thickness-1.25]) linear_extrude(height = 0.6) {
           translate([74.4,-32,0]) text(text = str(text, "iocafe"), font = font, size = 5, valign = "center", halign = "right");
           translate([-60.5,-35.5,0]) text(text = str(text, "electric banana"), font = font, size = 3, valign = "center", halign = "left");
        }
    }
}

finalblock();
// backscrewholes();


