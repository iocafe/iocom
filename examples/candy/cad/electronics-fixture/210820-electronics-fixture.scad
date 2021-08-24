include <210820-electronics-plate.scad>;
include <210820-camera-support.scad>;

intersection() {
  translate([0,0,1.0]) union() {
    plate();
    camerasupport(62, 0, -1.25);
  };
  translate([-75,-60,0]) cube ([150, 120, 100]);
}