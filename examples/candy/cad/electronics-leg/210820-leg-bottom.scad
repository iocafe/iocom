module bottom(x=0,y=0,z=0,thick=3.5, holed=3.5)
{
    difference() 
    {
        translate([x,y,z+thick/2]) 
        color("green", 50) 
        linear_extrude(height = thick, center = true, convexity = 10, twist = 0)
        {
            polygon(points=[[-9,0],[-8,3],[-2,5],[-1,13],[45,17],[58,24],[68,24],[72,20],[72,12],[52,12],[52,-12],[72,-12],[72,-20],[68,-24],[58,-24],[45,-17],[-1,-13],[-2,-5],[-8,-3]]);
        }
        translate([0,0,z+thick/2+0.2]) cylinder(thick, d=holed, $fn=24, center=true);
        translate([47.5,0,z+thick/2+0.2]) cylinder(thick, d=holed, $fn=24, center=true);
        translate([47.5+14.5,20,z+thick/2+0.2]) cylinder(thick, d=holed, $fn=24, center=true);
        translate([47.5+14.5,-20,z+thick/2+0.2]) cylinder(thick, d=holed, $fn=24, center=true);
    }
}
