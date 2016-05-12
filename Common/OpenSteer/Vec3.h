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
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_VEC3_H
#define OPENSTEER_VEC3_H

#include <cmath>     // for sqrt, etc.
#include <cstdlib>   // for rand, etc.

#define VEC3_ZERO		Vec3(0.0f, 0.0f, 0.0f)
#define VEC3_SIDE		Vec3(1.0f, 0.0f, 0.0f)
#define VEC3_UP			Vec3(0.0f, 1.0f, 0.0f)
#define VEC3_FORWARD	Vec3(0.0f, 0.0f, 1.0f)

#define F_RANDOM_01		(((float) rand ()) / ((float) RAND_MAX))

namespace OpenSteer {

    // ----------------------------------------------------------------------------


    class Vec3
    {
    public:

        // ----------------------------------------- generic 3d vector operations

        // three-dimensional Cartesian coordinates
        float x, y, z;

        // constructors
		Vec3()
		{
			this->x = 0.0f;
			this->y = 0.0f;
			this->z = 0.0f;
		}

        Vec3 (float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

        Vec3 operator + (const Vec3& v) const 
		{
			return Vec3(x + v.x, y + v.y, z + v.z);
		}

        // vector subtraction
        Vec3 operator- (const Vec3& v) const {return Vec3 (x-v.x, y-v.y, z-v.z);}

        // unary minus
        Vec3 operator- (void) const {return Vec3 (-x, -y, -z);}

        // vector times scalar product (scale length of vector times argument)
        Vec3 operator* (const float s) const {return Vec3 (x * s, y * s, z * s);}

        // vector divided by a scalar (divide length of vector by argument)
        Vec3 operator/ (const float s) const {return Vec3 (x / s, y / s, z / s);}

        // dot product
        float dot (const Vec3& v) const {return (x * v.x) + (y * v.y) + (z * v.z);}

        // length
        float length (void) const {return sqrt(lengthSquared ());}

        // length squared
        float lengthSquared (void) const {return (x * x) + (y * y) + (z * z);}

        // normalize: returns normalized version (parallel to this, length = 1)
        Vec3 normalize (void) const
        {
            // skip divide if length is zero
            const float len = length ();
            return (len>0) ? (*this)/len : (*this);
        }

        // cross product (modify "*this" to be A x B)
        // [XXX  side effecting -- deprecate this function?  XXX]
        void cross(const Vec3& a, const Vec3& b)
        {
            *this = Vec3 ((a.y * b.z) - (a.z * b.y),
                          (a.z * b.x) - (a.x * b.z),
                          (a.x * b.y) - (a.y * b.x));
        }

        Vec3 Cross(const Vec3& a, const Vec3& b)
        {
            return Vec3 ((a.y * b.z) - (a.z * b.y),
                         (a.z * b.x) - (a.x * b.z),
                         (a.x * b.y) - (a.y * b.x));
        }

        // assignment
        Vec3 operator= (const Vec3& v) {x=v.x; y=v.y; z=v.z; return *this;}

        // +=
        Vec3 operator+= (const Vec3& v) {return *this = (*this + v);}

        // -=
        Vec3 operator-= (const Vec3& v) {return *this = (*this - v);}

        // *=
        Vec3 operator*= (const float& s) {return *this = (*this * s);}

        
        Vec3 operator/=( float d ) { return *this = (*this / d);  }
        
        // equality/inequality
        bool operator== (const Vec3& v) const {return x==v.x && y==v.y && z==v.z;}
        bool operator!= (const Vec3& v) const {return !(*this == v);}

        // --------------------------- utility member functions used in OpenSteer

        // return component of vector perpendicular to a unit basis vector
        // (IMPORTANT NOTE: assumes "basis" has unit magnitude (length==1))

        inline Vec3 perpendicularComponent (const Vec3& unitBasis) const
        {
            return (*this) - (unitBasis * this->dot(unitBasis));
        }

        // clamps the length of a given vector to maxLength.  If the vector is
        // shorter its value is returned unaltered, if the vector is longer
        // the value returned has length of maxLength and is paralle to the
        // original input.

        Vec3 truncateLength (const float maxLength) const
        {
            const float maxLengthSquared = maxLength * maxLength;
            const float vecLengthSquared = this->lengthSquared ();
            if (vecLengthSquared <= maxLengthSquared)
                return *this;
            else
                return (*this) * (maxLength / sqrt(vecLengthSquared));
        }

        // forces a 3d position onto the XZ (aka y=0) plane

        Vec3 setYtoZero (void) const {return Vec3 (this->x, 0, this->z);}
    };


    // ----------------------------------------------------------------------------
    // scalar times vector product ("float * Vec3")


    inline Vec3 operator* (float s, const Vec3& v) {return v*s;}

    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed inside a sphere of unit radius
    // centered at the origin.  Orientation will be random and length will range
    // between 0 and 1


    Vec3 RandomVectorInUnitRadiusSphere (void);

    // ----------------------------------------------------------------------------
    // Returns a position randomly distributed on a circle of unit radius
    // on the XZ (Y=0) plane, centered at the origin.  Orientation will be
    // random and length will be 1

    inline Vec3 RandomUnitVectorOnXZPlane (void)
    {
		Vec3 Vector	= RandomVectorInUnitRadiusSphere();
		Vector.y	= 0.0f;

        return Vector.normalize();
    }

    // ----------------------------------------------------------------------------
    // used by limitMaxDeviationAngle / limitMinDeviationAngle below
    Vec3 vecLimitDeviationAngleUtility(const bool insideOrOutside, const Vec3& source, const float cosineOfConeAngle, const Vec3& basis);
}
    

// ----------------------------------------------------------------------------
#endif // OPENSTEER_VEC3_H
