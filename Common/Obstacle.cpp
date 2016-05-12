// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2004, Sony Computer Entertainment America
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
// OpenSteer Obstacle classes
// 
// 10-28-04 cwr: split off from Obstacle.h 
//
//
// ----------------------------------------------------------------------------


#include "OpenSteer/Obstacle.h"

Obstacle::Obstacle()
{
	_forward	= Vec3(0.0f, 0.0f, 1.0f);
	_side		= Vec3(1.0f, 0.0f, 0.0f);
	Position	= Vec3(0.0f, 0.0f, 0.0f);
}

Obstacle::~Obstacle()
{
}

OpenSteer::Vec3 Obstacle::steerToAvoid(const AbstractVehicle& vehicle, const float minTimeToCollision)
{
    // find nearest intersection with this obstacle along vehicle's path
    PathIntersection pi = findIntersectionWithVehiclePath(vehicle);

    // return steering for vehicle to avoid intersection, or zero if non found
	if (!pi.intersect)
		return VEC3_ZERO;
	else
		return pi.steerToAvoidIfNeeded(vehicle, minTimeToCollision);
}

// ----------------------------------------------------------------------------
// Obstacle
// static method to find first vehicle path intersection in an ObstacleGroup
//
// returns its results in the PathIntersection argument "nearest",
// "next" is used to store internal state.

// ----------------------------------------------------------------------------
// PathIntersection
// determine steering once path intersections have been found


OpenSteer::Vec3 PathIntersection::steerToAvoidIfNeeded (const AbstractVehicle& vehicle, const float minTimeToCollision) const
{
    // if nearby intersection found, steer away from it, otherwise no steering
    const float minDistanceToCollision = minTimeToCollision * vehicle._speed;
    if (intersect && (distance < minDistanceToCollision))
    {
        // compute avoidance steering force: take the component of
        // steerHint which is lateral (perpendicular to vehicle's
        // forward direction), set its length to vehicle's maxForce
        Vec3 lateral = steerHint.perpendicularComponent (vehicle._forward);
        return lateral.normalize() * vehicle._maxForce;
    }
    else
        return VEC3_ZERO;
}

RectangleObstacle::RectangleObstacle(const Vec3 &Start, const Vec3 &End)
: Obstacle()
{
	this->Start = Start;
	this->End = End;

	Vec3 Line = this->End - this->Start;
	Vec3 Forward;

	this->_side		= Line.normalize();
	Forward.cross(this->_side, Vec3(0.0f, 1.0f, 0.0f));
	this->_forward	= Forward;

	this->width		= Line.length();
	this->Position	= Start + (Line * 0.5f);
}

RectangleObstacle::~RectangleObstacle()
{
}

bool RectangleObstacle::xyPointInsideShape(const Vec3& point, float radius)
{
    const float w = radius + (width * 0.5f);
    return !((point.x >  w) || (point.x < -w));
}

// transform a direction in global space to its equivalent in local space
Vec3 RectangleObstacle::localizeDirection(const Vec3& globalDirection) const
{
    // dot offset with local basis vectors to obtain local coordinates
    return Vec3(globalDirection.dot(_side), 0.0f, globalDirection.dot(_forward));
};

PathIntersection RectangleObstacle::findIntersectionWithVehiclePath(const AbstractVehicle& vehicle)
{
	// initialize pathIntersection object to "no intersection found"
	PathIntersection pi;

    const Vec3 lp = localizeDirection(vehicle.Position - this->Position);
    const Vec3 ld = localizeDirection(vehicle._forward);

    // no obstacle intersection if path is parallel to XY (side/up) plane
    if (ld.dot (VEC3_FORWARD) == 0.0f)
		return pi;

    // no obstacle intersection if vehicle is heading away from the XY plane
    if ((lp.z > 0.0f) && (ld.z > 0.0f))
		return pi;
    if ((lp.z < 0.0f) && (ld.z < 0.0f))
		return pi;

    // find intersection of path with rectangle's plane (XY plane)
    const float ix = lp.x - (ld.x * lp.z / ld.z);
    const Vec3 planeIntersection (ix, 0.0f, 0.0f);

    // no obstacle intersection if plane intersection is outside 2d shape
    if (!xyPointInsideShape (planeIntersection, vehicle._radius))
		return pi;

    // otherwise, the vehicle path DOES intersect this rectangle
    const float sideSign	= (lp.z > 0.0f) ? +1.0f : -1.0f;

    pi.intersect			= true;
    pi.distance				= (lp - planeIntersection).length();
    pi.surfaceNormal		= _forward * sideSign;
	pi.steerHint			= pi.surfaceNormal	+ (this->_side * planeIntersection.normalize().x);
	pi.surfacePoint			= this->Position	+ (this->_side * planeIntersection.x);
    pi.vehicleOutside		= lp.z > 0.0f;

	return pi;
}

void RectangleObstacle::draw()
{
	//const float w	= this->width / 2;

	//const Vec3 v1 = this->Start;
	//const Vec3 v2 = this->End;

	//glColor3f (0.1f, 0.1f, 0.2f);
	//glBegin(GL_LINES);
	//{
	//	glVertex3f(v1.x, v1.y, v1.z);	glVertex3f(v2.x, v2.y, v2.z);
	//}
	//glEnd();
}

// ----------------------------------------------------------------------------
// BoxObstacle
// find first intersection of a vehicle's path with this obstacle

BoxObstacle::BoxObstacle(float w, float d)
{
	this->width		= w;
	this->depth		= d;

	this->Position	= Vec3(0.0f, 0.0f, 0.0f);

	const float hw = 0.5f * width;	// offsets for face centers
    const float hd = 0.5f * depth;

	r[0] = new RectangleObstacle(Position + Vec3(-hw, 0.0f,  hd), Position + Vec3( hw, 0.0f,  hd));
	r[1] = new RectangleObstacle(Position + Vec3( hw, 0.0f, -hd), Position + Vec3(-hw, 0.0f, -hd));
	r[2] = new RectangleObstacle(Position + Vec3( hw, 0.0f,  hd), Position + Vec3( hw, 0.0f, -hd));
	r[3] = new RectangleObstacle(Position + Vec3(-hw, 0.0f, -hd), Position + Vec3(-hw, 0.0f,  hd)); // bottom

    // group the six RectangleObstacle faces together
	for (int i = 0 ; i < 4 ; i++)
		faces.push_back(r[i]);
}

BoxObstacle::~BoxObstacle()
{
}

PathIntersection BoxObstacle::findIntersectionWithVehiclePath(const AbstractVehicle& vehicle)
{
    PathIntersection pi = firstPathIntersectionWithObstacleGroup(vehicle, faces);

    // when intersection found, adjust PathIntersection for the box case
    if (pi.intersect)
        pi.steerHint = ((pi.surfacePoint - this->Position).normalize () * (pi.vehicleOutside ? 1.0f : -1.0f));

	return pi;
}

PathIntersection BoxObstacle::firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle, const ObstacleGroup& obstacles)
{
	PathIntersection Current, Nearest;

    // test all obstacles in group for an intersection with the vehicle's
    // future path, select the one whose point of intersection is nearest
    //Nearest->intersect = false;

    for (ObstacleIterator o = obstacles.begin(); o != obstacles.end(); o++)
    {
        // find nearest point (if any) where vehicle path intersects obstacle
        // o, storing the results in PathIntersection object "next"
        Current = (**o).findIntersectionWithVehiclePath(vehicle);

		if (Current.intersect)
		{
			// if this is the first intersection found, or it is the nearest found
			// so far, store it in PathIntersection object "nearest"
			if ((!Nearest.intersect) || (Current.distance < Nearest.distance))
				Nearest = Current;
		}
    }

	return Nearest;
}

void BoxObstacle::draw()
{
	for (int i = 0 ; i < 4 ; i++)
		r[i]->draw();
}