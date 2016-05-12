#ifndef _BOID_H_
#define _BOID_H_

#include <algorithm>
#include "OpenSteer/Proximity.h"
#include "OpenSteer/Obstacle.h"
#include "OpenSteer/Boid.h"
using namespace OpenSteer;

#define LIMIT_WIDTH		13.0f
#define LIMIT_LENGTH	16.0f

#define RIGHT_HANDED	false

#define INTERPOLATE(a, i, j)  	(i + ((j - i) * a))
#define CLIP(f, lower, upper)	(f < lower) ? lower : ((f > upper) ? upper : f)

typedef AbstractProximityDatabase<AbstractVehicle*> ProximityDatabase;
typedef AbstractTokenForProximityDatabase<AbstractVehicle*> ProximityToken;

class Force
{
	public:
		Force(float Radius, float Angle, float Weight)
		{
			this->Radius	= Radius;
			this->Angle		= Angle;
			this->Weight	= Weight;
		}
		~Force()
		{
		}

		float Radius, Angle, Weight;
};

class Boid : public AbstractVehicle
{
	public:
		Boid(ProximityDatabase& pd, BoxObstacle* obstacles);
		~Boid();

		void reset();
		void update(const float currentTime, const float elapsedTime);	// per frame simulation update

	protected:
		Vec3 steerToFlock();											// basic flocking
		void regenerateLocalSpace(const Vec3& newVelocity);
		void applySteeringForce  (const Vec3& force, const float deltaTime);		// apply a given steering force to our momentum, adjusting our orientation to maintain velocity-alignment.
		Vec3 adjustRawSteeringForce (const Vec3& force, const float deltaTime);

		Vec3 steerForSeparation();	// Separation behavior -- determines the direction away from nearby boids
		Vec3 steerForAlignment();	// Alignment behavior
        Vec3 steerForCohesion();	// Cohesion behavior

		bool inBoidNeighborhood(const AbstractVehicle& otherVehicle, const float minDistance, const float maxDistance, const float cosMaxAngle);

		BoxObstacle*	obstacles;		// group of all obstacles to be avoided by each Boid
		ProximityToken*	proximityToken;		// a pointer to this boid's interface object for the proximity database

		std::vector<AbstractVehicle*> neighbors;

		float maxRadius;

		Force *Separation, *Alignment, *Cohesion;

        Vec3 _lastForward;
        Vec3 _lastPosition;
        Vec3 _smoothedAcceleration;

		float _maxSpeed;   // the maximum speed this vehicle is allowed to move (velocity is clipped to this magnitude)
};

#endif