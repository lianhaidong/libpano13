/* PaniniGeneral.h		15Jan2010 TKS

This is the reference implementation of the General Pannini
Projection, an elaboration of the basic Pannini projection 
discovered by Bruno Postle and Thomas Sharpless in December 
2008 in paintings by Gian Paolo Pannini (1691-1765).

    (C) copyright 2010 Thomas K Sharpless
Free license is hereby granted for noncommercial use
under the terms of the GNU Lesser General Public License 
version 2.1 as published by the Free Software Foundation 
and appearing in the file LICENSE.LGPL included in the
packaging of this file.  Please review the following 
information to ensure the requirementsGeneral of the GNU Lesser 
General Public License version 2.1 will be met: 
http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.

It is a parameterized mapping between sphere and plane, 
that gives synthetic perspective views on the plane when
the sphere holds a linear projection of the scene.  Sphere 
coordinates (phi, theta) are equirectangular: longitude and 
latitude angles, in radians, relative to a point on the equator.
Plane coordinates (h, v) are relative to the image of the same
point, typically but not necessarily the center point of
the view.  The plane y coordinate is negative upward, as is
typical in image processing software.

There are 3 parameters:
  d [0:infinity) controls horizontal compression
  t [-1:1] controls vertical compression at top
  b [-1:1] controls vertical compression at bottom

There are functions to map cooridnates in either direction
and one to compute the maximum feasible field of view of the
plane image, given a d value and the projection angle limit
of your display system.  

Angles passed to and returned by panini_general_maxVAs() 
are max view angles (half-FOVs) in radians.

All 3 functions return an integer: 0: failure, 1: OK.
Computed coordinates and FOVs are returned in arguments
passed by address.

*/
int panini_general_toPlane	( double phi, double theta, 
							  double* h,  double* v, 
							  double d, double t, double b
							 );
int panini_general_toSphere	( double* phi, double* theta, 
							  double  h,  double  v, 
							  double d, double t, double b
							 );
int panini_general_maxVAs	( double d, 
							  double maxProj,
							  double * maxView
							 );
