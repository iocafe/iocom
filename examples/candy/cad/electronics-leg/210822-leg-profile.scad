module legprofile(x=0,y=0,z=0,l=40,r = 0)
{
    holed = 2.8;
    depth = 15;
    movey = 30;
    cent = -4;

    translate([x+l/2,y,z]) rotate([90, 25,90])
    union() {
        difference() {
            color("blue", 50) 
            linear_extrude(height = l, center = true, convexity = 10, twist = 0)
            {
                polygon(points=[[3,2.5],[0,12],[0,30],[-8,30],[-8,20],[-5,12],[-5,8],[-8,-2.512]]);
            }
            
            for (zz = [0:9]) {
                translate([cent,movey-depth/2, zz*5-22.5]) rotate([90,0,0]) color ("red") {cylinder(h=depth+0.02, d=holed, $fn=20,center=true); }
            }
        }
    }
}
