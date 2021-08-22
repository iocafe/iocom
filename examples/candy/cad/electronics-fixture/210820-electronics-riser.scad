module riser(x=0,y=0,z=0, height=9.2,diam=7,holed=1.75)
{
    translate([x,y,z]) {
        difference() {
            color ("blue") { cylinder(h=height, d=diam, $fn=24); }
            translate([0,0,-0.01]) color ("black"){cylinder(h=height+0.02, d=holed, $fn=20); }
        }
    }
}
