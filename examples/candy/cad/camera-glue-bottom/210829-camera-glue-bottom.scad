difference() 
{
    translate([0,0,0]) {
    color ("blue") translate([0,0,1.5]) cylinder(3,d=20,center=true,$fn=52); 
    color ("yellow") translate([0,0,4.5]) cylinder(4.5,d=10,center=true,$fn=52); 
    }
    color ("green") cylinder(30,d=2.6,center=true,$fn=52); 
}

