module switchfixture(x=0,y=0,z=0,lip=14,w=18,h=55,holed=12.2,thick=4)
{
    translate([x,y,z]) {
        
        difference() {
            union() {
                color ("yellow") { translate([0,0,0]) cube([lip,w,2.8*thick],center=true); }
                color ("yellow") { translate([-lip/2+thick/2,0,h/2]) cube([thick,w,h],center=true); }
            }

            union() {
                translate([-lip/2+thick/2,0, h-holed/2-3.01]) rotate([90,90,90]) color ("red"){cylinder(h=thick+0.02, d=holed, $fn=20,center=true); }
            }
        }

    }
}
