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
// Obstacles for use with obstacle avoidance
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 09-05-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_OBSTACLE_H
#define OPENSTEER_OBSTACLE_H

#include <windows.h>
#include <GL/gl.h>     // for Linux and Windows
#include <float.h>
#include <vector>
#include "OpenSteer/Vec3.h"
#include "OpenSteer/AbstractVehicle.h"
using namespace OpenSteer;

class PathIntersection						// PathIntersection object: used internally to analyze and store information about intersections of vehicle paths and obstacles.
{
	public:
		PathIntersection()
		{
			this->intersect	= false;
			this->distance	= FLT_MAX;
		}

		bool intersect;						// was an intersection found?
		float distance;						// how far was intersection point from vehicle?
		Vec3 surfacePoint;					// position of intersection
		Vec3 surfaceNormal;					// unit normal at point of intersection
		Vec3 steerHint;						// where to steer away from intersection
		bool vehicleOutside;				// is the vehicle outside the obstacle?

		Vec3 steerToAvoidIfNeeded(const AbstractVehicle& vehicle, const float minTimeToCollision) const;	// determine steering based on path intersection tests
};

class RectangleObstacle;
typedef std::vector<RectangleObstacle*> ObstacleGroup;
typedef ObstacleGroup::const_iterator ObstacleIterator;

class Obstacle
{
	public:
		Obstacle();
        ~Obstacle();
        
        Vec3 steerToAvoid(const AbstractVehicle& v, const float minTimeToCollision);		// compute steering for a vehicle to avoid this obstacle, if needed 
		virtual PathIntersection findIntersectionWithVehiclePath(const AbstractVehicle& vehicle) = NULL;	// find first intersection of a vehicle's path with this obstacle (this must be specialized for each new obstacle shape class)

		Vec3 _forward, _side, Position;
};

class RectangleObstacle : public Obstacle
{
	public:
		RectangleObstacle(const Vec3 &Start, const Vec3 &End);
		~RectangleObstacle();

		void draw();
		Vec3 localizeDirection(const Vec3& globalDirection) const;

		bool xyPointInsideShape(const Vec3& point, float radius);
		PathIntersection findIntersectionWithVehiclePath (const AbstractVehicle& vehicle);	// find first intersection of a vehicle's path with this obstacle

		float width;
		Vec3 Start, End;
};

class BoxObstacle : public Obstacle
{
    public:
		BoxObstacle(float w, float d);
		~BoxObstacle();

		void draw();
		PathIntersection findIntersectionWithVehiclePath (const AbstractVehicle& vehicle);
		PathIntersection firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle, const ObstacleGroup& obstacles);

		float width, depth;
		Vec3 Position;

	private:
		RectangleObstacle* r[4];
		ObstacleGroup faces;
};

#endif
