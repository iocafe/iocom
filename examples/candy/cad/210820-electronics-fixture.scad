include <210820-electronics-plate.scad>;
include <210820-camera-support.scad>;
include <210820-switch-fixture.scad>;

intersection() {
  translate([0,0,1.0]) union() {
    plate();
    camerasupport(55.5, 0, -1.25);
    switchfixture(-51,11,0.75);
  };
  translate([-75,-60,0]) cube ([150, 120, 100]);
}