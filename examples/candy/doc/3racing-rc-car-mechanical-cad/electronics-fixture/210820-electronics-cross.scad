module cross(x=0,y=0,z=0,xd=91.8,yd=62.0,w=8,h=5.0,holed=3.5)
{
    counter = 6;

    translate([x,y,z]) {
        difference() {
            union() {
                color ("blue") { cube([xd,w,h],center=true); }
                color ("blue"){cube([w,yd,h],center=true); }
            }

            union() {
               for (xx = [0:7]) {
                   translate([xx*5,0,0]) color ("red") {cylinder(h=h+0.02, d=holed, $fn=20,center=true); }
                   translate([-xx*5,0,0]) color ("red") {cylinder(h=h+0.02, d=holed, $fn=20,center=true); }
                   translate([xx*5,0,3.4]) color ("red") {cylinder(h=h+0.02, d=counter, $fn=20,center=true); }
                   translate([-xx*5,0,3.4]) color ("red") {cylinder(h=h+0.02, d=counter, $fn=20,center=true); }
               }
            }
        }
    }
}
