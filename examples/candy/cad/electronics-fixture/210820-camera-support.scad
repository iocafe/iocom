module camerasupport(x=0,y=0,z=0, w=6,h=45,h2=52,thick=8.0, lip=24, dist=26)
{
    difference() 
    {
        translate([0,0,0]) 
        {
            translate([x,y+dist,z+thick/2]) {
                rotate([10,-10,0]) color ("yellow") { translate([0,0,h/2]) cube([thick,w,h],center=true); }
                color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,1.1*thick],center=true); }
            }
            translate([x,y,z+thick/2]) {
                rotate([10,-10,0]) color ("yellow") { translate([0,0,0.5*h/2]) cube([thick,w,0.5*h],center=true); }
                color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,1.1*thick],center=true); }
            }
            translate([x,y-20,z+thick/2]) {
                rotate([-35,-10,0]) color ("yellow") { translate([0,0,h2/2]) cube([thick,w,h2],center=true); }
                color ("yellow") { translate([-lip/2+thick/2,0,0]) cube([lip,1.5*w,1.1*thick],center=true); }
            }

            translate([x,y,z+thick/2]) {
                color ("yellow") { translate([thick/2-10.12,11,h/2+20]) cube([thick,22,16],center=true); }
            }
        }

        color ("red") { translate([thick/2+dist+lip-6,11,h/2+20]) cube([thick,23,26],center=true); }
        color ("green") { translate([thick/2+dist+lip-6,13,h/2+23]) rotate([90,90,90]) cylinder(30,d=3.4,center=true,$fn=52); }
        color ("blue") { translate([thick/2+dist+lip+5.5,13,h/2+23]) rotate([90,90,90]) cylinder(thick,d=10,center=true,$fn=52); }
   }
}
