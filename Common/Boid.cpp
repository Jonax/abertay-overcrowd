#include "OpenSteer/Boid.h"

Boid::Boid(ProximityDatabase& pd, BoxObstacle* obstacles)
{
	proximityToken	= pd.allocateToken(this);	// allocate a token for this boid in the proximity database
	this->obstacles = obstacles;

	reset();							// reset all boid state

	this->Separation	= new Force(1.0f, -0.707f, 12.0f);
	this->Alignment		= new Force(1.0f, 0.7f, 8.0f);
	this->Cohesion		= new Force(1.0f, -0.15f, 8.0f);

	this->maxRadius = max(Separation->Radius, max(Alignment->Radius, Cohesion->Radius));
}

Boid::~Boid()
{
	delete proximityToken;					// delete this boid's token in the proximity database
}

void Boid::reset()
{
	this->_lastForward			= VEC3_ZERO;
	this->_lastPosition			= VEC3_ZERO;
	this->_smoothedAcceleration = VEC3_ZERO;

	this->_maxForce = 27.0f;				// steering force is clipped to this magnitude
	this->_maxSpeed = 9.0f;
	this->_speed	= this->_maxSpeed * 0.3f;	// initial slow speed (30% of max speed)
	this->_radius	= 0.5f;			// size of bounding sphere

    this->_forward	= RandomVectorInUnitRadiusSphere().normalize();
	this->Position	= RandomVectorInUnitRadiusSphere() * 20;	// randomize initial position
    if (RIGHT_HANDED)
        _side.cross(_forward, Vec3(0.0f, 1.0f, 0.0f));
    else
        _side.cross(Vec3(0.0f, 1.0f, 0.0f), _forward);
    this->_side = _side.normalize();

	proximityToken->updateForNewPosition(Position);		// notify proximity database that our position has changed
}

void Boid::update(const float currentTime, const float elapsedTime)		// per frame simulation update
{
	this->applySteeringForce(steerToFlock(), elapsedTime);				// steer to flock and avoid obstacles if any

	if (Position.x < -(LIMIT_LENGTH + 2.0f))
		Position.x = LIMIT_LENGTH;
	if (Position.x > LIMIT_LENGTH + 2.0f)
		Position.x = -LIMIT_LENGTH;

	if (Position.z < -(LIMIT_WIDTH + 2.0f))
		Position.z = LIMIT_WIDTH;
	if (Position.z > LIMIT_WIDTH + 2.0f)
		Position.z = -LIMIT_WIDTH;

	proximityToken->updateForNewPosition(Position);				// notify proximity database that our position has changed
}

Vec3 Boid::steerToFlock()											// basic flocking
{
	// avoid obstacles if needed
	const Vec3 avoidance = obstacles->steerToAvoid(*this, 1.0f);
	if (avoidance != VEC3_ZERO)
		return avoidance;

	// find all flockmates within maxRadius using proximity database
	neighbors.clear();
	proximityToken->findNeighbors(this->Position, maxRadius, neighbors);

	// determine each of the three component behaviors of flocking
	return this->steerForSeparation() + this->steerForAlignment() + this->steerForCohesion();
}

void Boid::regenerateLocalSpace(const Vec3& newVelocity)	// control orientation for this boid
{
	const Vec3 newVel		= newVelocity.perpendicularComponent(VEC3_UP);

	this->_speed			= newVel.length();
	this->Position			= Vec3(this->Position.x, 0.0f, this->Position.z);
	this->_forward			= newVel / this->_speed;

    if (RIGHT_HANDED)
        _side.cross (_forward, Vec3(0.0f, 1.0f, 0.0f));
    else
        _side.cross (Vec3(0.0f, 1.0f, 0.0f), _forward);
    _side = _side.normalize ();
}

// Separation behavior: steer away from neighbors
Vec3 Boid::steerForSeparation()
{
	// Radius = Maximum Distance
	// Angle = Cos of Maximum Angle

    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;
    int neighbors = 0;

    // for each of the other vehicles...
	for (std::vector<AbstractVehicle*>::const_iterator otherVehicle = this->neighbors.begin(); otherVehicle != this->neighbors.end(); ++otherVehicle )
    {
        if (this->inBoidNeighborhood(**otherVehicle, this->_radius * 3, Separation->Radius,	Separation->Angle))
        {
            // add in steering contribution
            // (opposite of the offset direction, divided once by distance
            // to normalize, divided another time to get 1/d falloff)
            const Vec3 offset = (**otherVehicle).Position - Position;
            const float distanceSquared = offset.dot(offset);
            steering += (offset / -distanceSquared);

            // count neighbors
            ++neighbors;
        }
    }
    
    return steering.normalize() * Separation->Weight;
}

// Alignment behavior: steer to head in same direction as neighbors
Vec3 Boid::steerForAlignment()
{
	// Radius = Maximum Distance
	// Angle = Cos of Maximum Angle

    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;
    int neighbors = 0;

    // for each of the other vehicles...
    for (std::vector<AbstractVehicle*>::const_iterator otherVehicle = this->neighbors.begin(); otherVehicle != this->neighbors.end(); otherVehicle++)
    {
        if (this->inBoidNeighborhood(**otherVehicle, this->_radius * 3, Alignment->Radius, Alignment->Angle))
        {
            // accumulate sum of neighbor's heading
            steering += (**otherVehicle)._forward;

            // count neighbors
            neighbors++;
        }
    }

    // divide by neighbors, subtract off current heading to get error-
    // correcting direction, then normalize to pure direction
    if (neighbors > 0) steering = ((steering / (float)neighbors) - _forward).normalize();

    return steering * Alignment->Weight;
}

// Cohesion behavior: to to move toward center of neighbors
Vec3 Boid::steerForCohesion()
{
	// Radius = Maximum Distance
	// Angle = Cos of Maximum Angle

    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;
    int neighbors = 0;

    // for each of the other vehicles...
    for (std::vector<AbstractVehicle*>::const_iterator otherVehicle = this->neighbors.begin(); otherVehicle != this->neighbors.end(); otherVehicle++)
    {
        if (this->inBoidNeighborhood(**otherVehicle, this->_radius * 3, Cohesion->Radius, Cohesion->Angle))
        {
            // accumulate sum of neighbor's positions
            steering += (**otherVehicle).Position;

            // count neighbors
            neighbors++;
        }
    }

    // divide by neighbors, subtract off current position to get error-
    // correcting direction, then normalize to pure direction
    if (neighbors > 0) steering = ((steering / (float)neighbors) - Position).normalize();

    return steering * Cohesion->Weight;
}

// used by boid behaviors: is a given vehicle within this boid's neighborhood?
bool Boid::inBoidNeighborhood(const AbstractVehicle& otherVehicle, const float minDistance, const float maxDistance, const float cosMaxAngle)
{
    if (&otherVehicle == this)
        return false;
    else
    {
        const Vec3 offset = otherVehicle.Position - Position;
        const float distanceSquared = offset.lengthSquared();

        // definitely in neighborhood if inside minDistance sphere
        if (distanceSquared < (minDistance * minDistance))
            return true;

        // definitely not in neighborhood if outside maxDistance sphere
        if (distanceSquared > (maxDistance * maxDistance))
            return false;

        // otherwise, test angular offset from forward axis
        const Vec3 unitOffset = offset / sqrt (distanceSquared);
        const float forwardness = _forward.dot (unitOffset);
        return forwardness > cosMaxAngle;
    }
}

// ----------------------------------------------------------------------------
// apply a given steering force to our momentum,
// adjusting our orientation to maintain velocity-alignment.

void Boid::applySteeringForce(const Vec3& force, const float elapsedTime)
{
    const Vec3 adjustedForce = adjustRawSteeringForce (force, elapsedTime);

    // enforce limit on magnitude of steering force
    const Vec3 clippedForce = adjustedForce.truncateLength(this->_maxForce);

    // compute acceleration and velocity
    Vec3 newAcceleration = clippedForce;
    Vec3 newVelocity = _forward * _speed;

    // damp out abrupt changes and oscillations in steering acceleration
    // (rate is proportional to time step, then clipped into useful range)
    if (elapsedTime > 0)
    {
		float smoothTime = CLIP(9 * elapsedTime, 0.15f, 0.4f);
		_smoothedAcceleration = INTERPOLATE(smoothTime, _smoothedAcceleration, newAcceleration);
    }

    // Euler integrate (per frame) acceleration into velocity
    newVelocity += _smoothedAcceleration * elapsedTime;

    // enforce speed limit
    newVelocity = newVelocity.truncateLength(this->_maxSpeed);

    // update Speed
	this->_speed = newVelocity.length();

    // Euler integrate (per frame) velocity into position
	this->Position += newVelocity * elapsedTime;

    // regenerate local space (by default: align vehicle's forward axis with
    // new velocity, but this behavior may be overridden by derived classes.)
    regenerateLocalSpace (newVelocity);

    // maintain path curvature information
    if (elapsedTime > 0)
    {
        const Vec3 dP = _lastPosition - Position;
        const Vec3 dF = (_lastForward - _forward) / dP.length ();
        const Vec3 lateral = dF.perpendicularComponent(_forward);
        const float sign = (lateral.dot (_side) < 0) ? 1.0f : -1.0f;
        _lastForward	= _forward;
        _lastPosition	= Position;
    }
}

OpenSteer::Vec3 Boid::adjustRawSteeringForce (const Vec3& force, const float /* deltaTime */)
{
    const float maxAdjustedSpeed = 0.2f * this->_maxSpeed;

    if ((this->_speed > maxAdjustedSpeed) || (force == VEC3_ZERO))
        return force;
    else
    {
        const float range	= this->_speed / maxAdjustedSpeed;
        const float cosine	= INTERPOLATE(pow(range, 20), 1.0f, -1.0f);
        return vecLimitDeviationAngleUtility (true, // force source INSIDE cone
                                              force,
                                              cosine,
                                              _forward);
    }
}