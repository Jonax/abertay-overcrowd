#include "OpenSteer/Boids.h"

BoidsPlugIn::BoidsPlugIn()
{
}

BoidsPlugIn::~BoidsPlugIn()
{
}

void BoidsPlugIn::open()
{
	#ifdef LQ_BIN_LATTICE
		const Vec3 center;
		const float div = 10.0f;
		const Vec3 divisions(div, 1, div);
		const Vec3 dimensions(	LIMIT_LENGTH * 1.1f * 2, 
								2.2f,
								LIMIT_WIDTH * 1.1f * 2);
		pd = new LQProximityDatabase<AbstractVehicle*>(center, dimensions, divisions);
	#else
		pd = new BruteForceProximityDatabase<AbstractVehicle*>();
	#endif

	// set up obstacles
	initObstacles();

//	for (int i = 0 ; i < NUM_BOIDS ; i++)
//		addBoidToFlock();
}

void BoidsPlugIn::update(const float elapsedTime)
{
	for (groupType::const_iterator i = flock.begin() ; i != flock.end() ; i++)
		(**i).update(NULL, elapsedTime);
}

void BoidsPlugIn::close()
{
	// delete each member of the flock
	for (groupType::const_iterator i = flock.begin() ; i != flock.end() ; i++)
		removeBoidFromFlock();

	// delete the proximity database
	delete pd;
	pd = NULL;
}

void BoidsPlugIn::reset()
{
	// reset each boid in flock
	for (groupType::const_iterator i = flock.begin() ; i != flock.end() ; i++)
		(**i).reset();
}

void BoidsPlugIn::addBoidToFlock()
{
	Presence* boid = new Presence(*pd, insideBigBox);
	flock.push_back(boid);
}

void BoidsPlugIn::removeBoidFromFlock()
{
	if (flock.size() > 0)
	{
		// save a pointer to the last boid, then remove it from the flock
		const Boid* boid = flock.back();
		flock.pop_back();

		delete boid;	// delete the Boid
	}
}

void BoidsPlugIn::initObstacles()
{
	insideBigBox = new BoxObstacle(LIMIT_LENGTH * 2, LIMIT_WIDTH * 2);
}