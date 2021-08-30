difference(){
    translate([0,0,5]){color("orange") cube([10, 17, 10],center=true);
    translate([0,4.13,6.5])color("orange") cube([10, 8.75, 5],center=true);
        
    translate([0,4.15,9])rotate([0,45,90])color("orange") cube([7, 10, 6],center=true);
   translate([0,4.15,9])rotate([0,-45,90])color("orange") cube([7, 10, 7],center=true);
   }
   rotate([0,0,90]){translate([-5,2,.5])cube([3,4,6]);
   translate([1.75,2,8])cube([5,5,5]); 
       translate([1,2.5,2])cube([2.5,5,2.5]);
      translate([4,2,2])cube([2.5,5,2.5]); }}
   translate([-3,4.25,10])color("orange") cube([3, .5, 6],center=true);
   translate([-1.75,4,10.5])rotate([0,90,0])color("orange") cube([.5, 6, 6],center=true);
    
    
