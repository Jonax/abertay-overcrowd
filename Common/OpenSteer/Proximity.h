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
// Proximity 
//
// Data structures for accelerating proximity/locality/neighborhood queries
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 06-20-01 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_PROXIMITY_H
#define OPENSTEER_PROXIMITY_H

#include <vector>
#include "OpenSteer/Vec3.h"
#include "OpenSteer/lq.h"   // XXX temp?
using namespace OpenSteer;

// "tokens" are the objects manipulated by the spatial database
template <class ContentType> class AbstractTokenForProximityDatabase
{
	public:
		virtual ~AbstractTokenForProximityDatabase() {}
		virtual void updateForNewPosition (const Vec3& position) = 0;	// the client object calls this each time its position changes
		virtual void findNeighbors (const Vec3& center, const float radius, std::vector<ContentType>& results) = 0;		// find all neighbors within the given sphere (as center and radius)
};

// abstract type for all kinds of proximity databases
template <class ContentType> class AbstractProximityDatabase
{
	public:
		typedef AbstractTokenForProximityDatabase<ContentType> tokenType;		// type for the "tokens" manipulated by this spatial database
		virtual ~AbstractProximityDatabase() {}
		virtual tokenType* allocateToken (ContentType parentObject) = 0;		// allocate a token to represent a given client object in this database
};

// This is the "brute force" O(n^2) approach implemented in terms of the AbstractProximityDatabase protocol so it can be compared directly to 
// other approaches.  (e.g. the Boids plugin allows switching at runtime.)
template <class ContentType> class BruteForceProximityDatabase : public AbstractProximityDatabase<ContentType>
{
	public:
		BruteForceProximityDatabase()
		{
		}

		virtual ~BruteForceProximityDatabase()
		{
		}

        class tokenType : public AbstractTokenForProximityDatabase<ContentType>		// "token" to represent objects stored in the database
	    {
			public:
				tokenType (ContentType parentObject, BruteForceProximityDatabase& pd)
				{
					// store pointer to our associated database and the object this token represents, and store this token on the database's 
					// vector.
					bfpd = &pd;
					object = parentObject;
					bfpd->group.push_back(this);
				}

				virtual ~tokenType ()
				{
					// remove this token from the database's vector
					bfpd->group.erase(std::find (bfpd->group.begin(), bfpd->group.end(), this));
				}

				// the client object calls this each time its position changes
				void updateForNewPosition (const Vec3& newPosition)
				{
					position = newPosition;
				}

				// find all neighbors within the given sphere (as center and radius)
				void findNeighbors (const Vec3& center, const float radius, std::vector<ContentType>& results)
				{
					// loop over all tokens
					const float r2 = radius * radius;

					for (std::vector<tokenType*>::const_iterator i = bfpd->group.begin() ; i != bfpd->group.end(); i++)
					{
						const Vec3 offset = center - (**i).position;
						const float d2 = offset.lengthSquared();

						// push onto result vector when within given radius
						if (d2 < r2) results.push_back ((**i).object);
					}
				}

			private:
				BruteForceProximityDatabase* bfpd;
				ContentType object;
				Vec3 position;
		};

		// allocate a token to represent a given client object in this database
		tokenType* allocateToken (ContentType parentObject)
		{
			return new tokenType (parentObject, *this);
		}
    
	private:
		std::vector<tokenType*> group;				// STL vector containing all tokens in database
};

// A AbstractProximityDatabase-style wrapper for the LQ bin lattice system
template <class ContentType> class LQProximityDatabase : public AbstractProximityDatabase<ContentType>
{
	public:
        LQProximityDatabase(const Vec3& center, const Vec3& dimensions, const Vec3& divisions)
        {
            const Vec3 halfsize (dimensions * 0.5f);
            const Vec3 origin (center - halfsize);

			lq = lqCreateDatabase (origin.x,			origin.y,		origin.z, 
                                   dimensions.x,		dimensions.y,	dimensions.z,  
                                   (int) divisions.x,	1,				(int) divisions.z	);
        }

        virtual ~LQProximityDatabase()
        {
            lqDeleteDatabase(lq);

            lq = NULL;
        }

        // "token" to represent objects stored in the database
        class tokenType : public AbstractTokenForProximityDatabase<ContentType>
        {
			public:
	            tokenType (ContentType parentObject, LQProximityDatabase& lqsd)
		        {
					proxy.prev   = NULL;
					proxy.next   = NULL;
					proxy.bin    = NULL;
					proxy.object = parentObject;

					lq = lqsd.lq;
				}

				virtual ~tokenType()
				{
					lqRemoveFromBin (&proxy);
				}

				// the client object calls this each time its position changes
				void updateForNewPosition (const Vec3& p)
				{
					lqUpdateForNewLocation (lq, &proxy, p.x, p.y, p.z);
				}

				// find all neighbors within the given sphere (as center and radius)
				void findNeighbors (const Vec3& center, const float radius, std::vector<ContentType>& results)
				{
					lqMapOverAllObjectsInLocality(	lq, 
													center.x, center.y, center.z,
													radius,
													perNeighborCallBackFunction,
													(void*)&results);
				}

            // called by LQ for each clientObject in the specified neighborhood:
            // push that clientObject onto the ContentType vector in void* clientQueryState
            // (parameter names commented out to prevent compiler warning from "-W")
			static void perNeighborCallBackFunction(void* clientObject, float /*distanceSquared*/, void* clientQueryState)
            {
				typedef std::vector<ContentType> ctv;
				ctv& results = *((ctv*) clientQueryState);
				results.push_back ((ContentType) clientObject);
            }

			private:
				lqClientProxy proxy;
				lqDB* lq;
		};

        // allocate a token to represent a given client object in this database
        tokenType* allocateToken(ContentType parentObject)
        {
            return new tokenType(parentObject, *this);
        }

		private:
			lqDB* lq;
};

#endif
