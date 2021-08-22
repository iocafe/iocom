module legsupport(x=0,y=0,z=0,l=4,r = 25)
{
    p = 18;
    q = -20;
    holed = 12.2;

    difference() {
        union() {
            translate([x+l/2,y,z]) rotate([90,r,90])
            color("yellow", 50) 
            linear_extrude(height = l, center = true, convexity = 10, twist = 0)
            {
                polygon(points=[[3+p*cos(r),2.5+p*sin(r)],[23,30],[-31,30],[-21,19],[-19,14],[-20,6],[-8+q*cos(r),5-2.512+q*sin(r)],[-8+q*cos(r),-2.512+q*sin(r)]]);
            }
        }

        union() {
            translate([x+l/2,y+18,z+11]) rotate([90,90,90]) color ("red"){cylinder(h=l+0.02, d=holed, $fn=20,center=true); }
        }
    }
}
