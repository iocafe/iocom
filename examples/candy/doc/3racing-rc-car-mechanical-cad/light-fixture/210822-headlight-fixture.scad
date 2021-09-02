

pcb_w = 90;
pcb_h = 13;
pcb_fix_hole_dist = 84.2;
pcb_hole_dist_from_top = 8.4;
pcb_size_tolerance = 0.2;

shoulder = 1.0;
edge_width = 2.5;
fixture_depth = 19;
pcb_depth = 8;
soldering_space = 3;
corner_w = 7;
corner_h = 7.0;
hole_1_d = 2.7;
hole_2_d = 3.5;

difference() {

 translate([0,0,fixture_depth/2])
    color("orange") cube([pcb_w + 2*edge_width + pcb_size_tolerance, pcb_h + 2*edge_width + pcb_size_tolerance, fixture_depth], center=true);

    union() {
        translate ([0, 0, fixture_depth-pcb_depth/2]) color("blue") cube([pcb_w + pcb_size_tolerance, pcb_h + pcb_size_tolerance, pcb_depth+0.1], center=true);
        translate ([0, 0, fixture_depth-pcb_depth-soldering_space/2]) color("green") cube([pcb_w - 2*corner_w, pcb_h - 2*shoulder, soldering_space+0.1], center=true);
        translate ([0, shoulder/2 -corner_h/2, fixture_depth-pcb_depth-soldering_space/2]) color("green") cube([pcb_w - 2*shoulder, pcb_h - corner_h - shoulder, soldering_space+0.1], center=true);

        translate ([84.2/2, (8.4-13/2), 0]) color("red") cylinder(2*fixture_depth + 0.1, d = hole_1_d, $fn=20, center=true);
        translate ([-84.2/2, (8.4-13/2), 0]) color("red") cylinder(2*fixture_depth + 0.1, d = hole_1_d, $fn=20, center=true);
        translate ([84.2/2 - 37.4, (8.4-13/2), 0]) color("red") cylinder(fixture_depth + 0.1, d = 4, $fn=20, center=true);

        translate ([-35, 0, 5]) rotate([90, 90, 0]) color("red") cylinder(30, d = hole_2_d, $fn=20, center=true);
        translate ([35, 0, 5]) rotate([90, 90, 0]) color("red") cylinder(30, d = hole_2_d, $fn=20, center=true);
    }
}


