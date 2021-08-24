include <210820-leg-bottom.scad>;
include <210822-leg-profile.scad>;
include <210820-leg-support.scad>;

bottom_thick = 3.5;

bottom(0,0,0, bottom_thick);

xpos = -0.8;
ypos = 15;
zpos = -1;

legprofile(xpos,ypos,zpos,52.8);

legsupport(6.0,ypos,zpos,4.0);
legsupport(40,ypos,zpos,4.0);
