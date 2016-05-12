#ifndef _BOIDS_H_
#define _BOIDS_H_

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
// OpenSteer Boids
// 
// 09-26-02 cwr: created 
//
//
// ----------------------------------------------------------------------------

#include <vector>
#include "OpenSteer/Proximity.h"
#include "../Presence.h"

#define MAX_INSTANCES		4000
#define DEFAULT_INSTANCES	100
#define LQ_BIN_LATTICE

using namespace OpenSteer;

typedef std::vector<Presence*>			groupType;	// type for a flock: an STL vector of Boid pointers

class BoidsPlugIn
{
	public:
		BoidsPlugIn();
		~BoidsPlugIn();	// be more "nice" to avoid a compiler warning

		void open();
		void update(const float elapsedTime);
		void close();
		void reset();

	protected:
		void addBoidToFlock();
		void removeBoidFromFlock();

		void initObstacles();

		// flock: a group (STL vector) of pointers to all boids
		groupType flock;

		ProximityDatabase* pd;	// pointer to database used to accelerate proximity queries

		BoxObstacle* insideBigBox;
};

#endif