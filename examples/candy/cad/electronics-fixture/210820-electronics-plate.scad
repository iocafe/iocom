include <210820-electronics-riser.scad>;
include <210820-electronics-cross.scad>;

module plate(xd=91.8,yd=62.0,extra=5.5, thick=2.5, holed=3.4) 
{
    difference() {
        color ("blue") { cube([xd+2*extra,yd+2*extra,thick],center=true); }
       
        translate([0,0,-0.01]) union() {
            color ("black"){cube([xd-2*extra,yd-2*extra,thick+0.03],center=true); }
            for (xx = [2:8]) {
               translate([xx*5,yd/2+1.8,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
               translate([-xx*5,yd/2+1.8,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
               translate([xx*5,-yd/2-1.8,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
               translate([-xx*5,-yd/2-1.8,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
            }
            for (yy = [2:3]) {
               translate([xd/2+2.5,yy*5,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
	    }
            for (yy = [1:2]) {
               translate([xd/2+2.5,-yy*5-2.5,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
	    }
            for (yy = [2:5]) {
               translate([-xd/2-2.5,yy*5,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
               translate([-xd/2-2.5,-yy*5,0.01]) color ("red") {cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
            }
        }
    }

    translate([0,0,thick-0.01]) difference() {
        color ("green") { cube([xd-2*extra+3*thick,yd+-2*extra+3*thick,3*thick],center=true); }
         translate([0,0,-0.01]) color ("black"){cube([xd-2*extra+thick,yd-2*extra+thick,3*thick+0.03],center=true); }
    }

    cross(0,0,thick/2,xd+1.75*extra,yd+1.75*extra);

    riser(xd/2,yd/2, thick/2-0.02);
    riser(xd/2,-yd/2, thick/2-0.02);
    riser(-xd/2,yd/2, thick/2-0.02);
    riser(-xd/2,-yd/2, thick/2-0.02);
}
