$fn=50;

rotate([30,0,0]) minkowski()
{
    cube([10,10,5]);
    sphere(2);
}    


translate([30,10,0]) 
{
    minkowski()
    {
        polyhedron(
          points=[ [10,10,0],[10,-10,0],[-10,-10,0],[-10,10,0], // the four points at base
                   [0,0,10]  ],                                 // the apex point 
          faces=[ [0,1,4],[1,2,4],[2,3,4],[3,0,4],              // each triangle side
                      [1,0,3],[2,1,3] ]                         // two triangles for square base
         );
        sphere(2);
    }
}
