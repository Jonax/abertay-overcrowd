// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// Vec3: OpenSteer's generic type for 3d vectors
//
// This file defines the class Vec3, which is used throughout OpenSteer to
// manipulate 3d geometric data.  It includes standard vector operations (like
// vector addition, subtraction, scale, dot, cross...) and more idiosyncratic
// utility functions.
//
// When integrating OpenSteer into a preexisting 3d application, it may be
// important to use the 3d vector type of that application.  In that case Vec3
// can be changed to inherit from the preexisting application' vector type and
// to match the interface used by OpenSteer to the interface provided by the
// preexisting 3d vector type.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 03-26-03 cwr: created to replace for Hiranabe-san's execellent but larger
//               vecmath package (http://objectclub.esm.co.jp/vecmath/)
//
//
// ----------------------------------------------------------------------------


#include "OpenSteer/Vec3.h"

// ----------------------------------------------------------------------------
// Returns a position randomly distributed inside a sphere of unit radius
// centered at the origin.  Orientation will be random and length will range
// between 0 and 1


OpenSteer::Vec3 
OpenSteer::RandomVectorInUnitRadiusSphere (void)
{
    Vec3 v;

    do
    {
		v = Vec3(	(F_RANDOM_01 * 2) - 1,
					(F_RANDOM_01 * 2) - 1,
					(F_RANDOM_01 * 2) - 1);
    }
    while (v.length() >= 1);

    return v;
}

// ----------------------------------------------------------------------------
// Does a "ceiling" or "floor" operation on the angle by which a given vector
// deviates from a given reference basis vector.  Consider a cone with "basis"
// as its axis and slope of "cosineOfConeAngle".  The first argument controls
// whether the "source" vector is forced to remain inside or outside of this
// cone.  Called by vecLimitMaxDeviationAngle and vecLimitMinDeviationAngle.


OpenSteer::Vec3 
OpenSteer::vecLimitDeviationAngleUtility (const bool insideOrOutside,
                                          const Vec3& source,
                                          const float cosineOfConeAngle,
                                          const Vec3& basis)
{
    // immediately return zero length input vectors
    float sourceLength = source.length();
    
	if (sourceLength == 0)
		return source;

    // measure the angular diviation of "source" from "basis"
    const Vec3 direction = source / sourceLength;
    float cosineOfSourceAngle = direction.dot (basis);

    // Simply return "source" if it already meets the angle criteria.
    // (note: we hope this top "if" gets compiled out since the flag
    // is a constant when the function is inlined into its caller)
    if (insideOrOutside)
    {
		// source vector is already inside the cone, just return it
		if (cosineOfSourceAngle >= cosineOfConeAngle)
			return source;
	}
	else
	{
		// source vector is already outside the cone, just return it
		if (cosineOfSourceAngle <= cosineOfConeAngle)
			return source;
	}

    // find the portion of "source" that is perpendicular to "basis"
    const Vec3 perp = source.perpendicularComponent (basis);

    // normalize that perpendicular
    const Vec3 unitPerp = perp.normalize ();

    // construct a new vector whose length equals the source vector,
    // and lies on the intersection of a plane (formed the source and
    // basis vectors) and a cone (whose axis is "basis" and whose
    // angle corresponds to cosineOfConeAngle)
    float perpDist = sqrt(1 - (cosineOfConeAngle * cosineOfConeAngle));
    const Vec3 c0 = basis * cosineOfConeAngle;
    const Vec3 c1 = unitPerp * perpDist;
    return (c0 + c1) * sourceLength;
}