module camerasupport(x=0,y=0,z=0, w=4,h=45,h2=52,thick=4.0, lip=15, dist=26)
{
    translate([x,y+dist,z+thick/2]) {
        rotate([10,0,0]) color ("yellow") { translate([0,0,h/2]) cube([thick,w,h],center=true); }
        color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,thick],center=true); }
    }
    translate([x,y,z+thick/2]) {
        rotate([10,0,0]) color ("yellow") { translate([0,0,0.5*h/2]) cube([thick,w,0.5*h],center=true); }
        color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,thick],center=true); }
    }
    translate([x,y-20,z+thick/2]) {
        rotate([-35,0,0]) color ("yellow") { translate([0,0,h2/2]) cube([thick,w,h2],center=true); }
        color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,thick],center=true); }
    }

    translate([x,y,z+thick/2]) {
        difference() {
            union() {
                color ("yellow") { translate([thick/2-0.75,10,h/2+13]) cube([1.5,20,h/2.5],center=true); }
                color ("yellow") { translate([thick/2-2.0,10,h/2+20]) cube([thick,20,thick],center=true); }
            }
            union() {
                color ("red") { translate([thick/2-2,10.3,h/2+1]) rotate([90,90,90]) cylinder(h=thick+0.1, d=19, $fn=20,center=true); }
            }
        }

    }
}
