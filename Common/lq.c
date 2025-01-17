/*
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
// ----------------------------------------------------------------------------
*/
/* ------------------------------------------------------------------ */
/*                                                                    */
/* Locality Query facility                                            */
/*                                                                    */
/* (by Craig Reynolds, see lq.h file for documentation)               */
/*                                                                    */
/*  5-17-99: created                                                  */
/*  5-20-99: found elusive "allocate 0 bins" bug                      */
/*  5-28-99: lqMapOverAllObjectsInLocality: clipped, incremental      */
/*  6- 7-99: clean up, split off annotation stuff into debuglq.c      */
/*  6- 8-99: tried screening by sum of coords ("first mean"?) but     */
/*           it was slightly slower, moved unused code to debuglq     */
/* 10-19-99: Change lqClientObject, lqObject from: "struct x {};" to  */
/*           "typedef struct x {} x;" for EE compiler.                */
/* 12- 2-00: Make lqObject "private" using lqInternalDB.              */
/* 12- 5-00: Rename lqObject to lqDB, lqClientObject to lqClientProxy */
/* 12- 6-00: Change lqCallBackFunction from arglist of (void*) to:    */
/*           (void* clientObject, float distanceSquared, void*        */
/*           clientQueryState).  Add void* clientQueryState arg to    */
/*           lqMapOverAllObjectsInLocality and its helper functions   */
/*           lqMapOverAllObjectsInLocalityClipped and                 */
/*           lqMapOverAllOutsideObjects. Change macro                 */
/*           lqTraverseBinClientObjectList to invoke callback         */
/*           function with three arguments, add "state" to its        */
/*           arglist.  Remove extern lqDistanceSquared.               */
/* 12- 7-00: Rename lqInitClientObject to lqInitClientProxy, make     */
/*           "func" be an argument to lqTraverseBinClientObjectList,  */
/*           add comments.                                            */
/* 12- 8-00: Add lqFindNearestNeighborWithinRadius and related        */
/*           definitions: lqFindNearestHelper lqFindNearestState      */
/*           Add lqMapOverAllObjects and lqRemoveAllObjects (plus:    */
/*           lqMapOverAllObjectsInBin and lqRemoveAllObjectsInBin)    */
/*                                                                    */
/* ------------------------------------------------------------------ */

#include "OpenSteer/lq.h"

#define lqBinCoordsToBinIndex(lq, ix, iy, iz)	((ix * (lq)->divy * (lq)->divz) + (iy * (lq)->divz) + iz)		/* Determine index into linear bin array given 3D bin indices */
#define lqRemoveAllObjectsInBin(bin)			while ((bin) != NULL) lqRemoveFromBin ((bin));

// This structure represents the spatial database.  Typically one of these would be created, by a call to lqCreateDatabase, for a given application.  */
typedef struct lqInternalDB
{
    float originx, originy, originz;				// the origin is the super-brick corner minimum coordinates
    float sizex, sizey, sizez;						// length of the edges of the super-brick
    int divx, divy, divz;							// number of sub-brick divisions in each direction
    lqClientProxy** bins;							// pointer to an array of pointers, one for each bin
    lqClientProxy* other;							// extra bin for "everything else" (points outside super-brick)
} lqInternalDB;

//	Allocate and initialize an LQ database, returns a pointer to it. The application needs to call this before using the LQ facility.
//	The nine parameters define the properties of the "super-brick":
//		(1) origin: coordinates of one corner of the super-brick, its minimum x, y and z extent.
//		(2) size: the width, height and depth of the super-brick.
//		(3) the number of subdivisions (sub-bricks) along each axis.
//	This routine also allocates the bin array, and initialize its contents.
lqInternalDB* lqCreateDatabase(float originx,	float originy,	float originz, 
							   float sizex,		float sizey,	float sizez, 
							   int divx,		int divy,		int divz)
{
    lqInternalDB* lq = ((lqInternalDB*) malloc (sizeof (lqInternalDB)));

    lqInitDatabase(lq,	originx,	originy,	originz,
						sizex,		sizey,		sizez,
						divx,		divy,		divz);
    return lq;
}

// Deallocate the memory used by the LQ database
void lqDeleteDatabase(lqDB* lq)
{
    free(lq->bins);
    free(lq);
}

// Given an LQ database object and the nine basic parameters: fill in the object's slots, allocate the bin array, and initialize its contents.
void lqInitDatabase(lqInternalDB* lq,	float originx, float originy, float originz,
										float sizex, float sizey, float sizez,
										int divx, int divy, int divz					)
{
    lq->originx = originx;
    lq->originy = originy;
    lq->originz = originz;

    lq->sizex = sizex;
    lq->sizey = sizey;
    lq->sizez = sizez;

    lq->divx = divx;
    lq->divy = divy;
    lq->divz = divz;

    {
		int i;
		int bincount	= divx * divy * divz;
		int arraysize	= sizeof(lqClientProxy*) * bincount;
		lq->bins		= (lqClientProxy**)malloc(arraysize);

		for (i = 0 ; i < bincount ; i++)
			lq->bins[i] = NULL;
    }

    lq->other = NULL;
}

// Find the bin ID for a location in space.  The location is given in terms of its XYZ coordinates.  The bin ID is a pointer to a pointer to the 
// bin contents list.
lqClientProxy** lqBinForLocation (lqInternalDB* lq, float x, float z)
{
    int i, ix, iz;

    /* if point outside super-brick, return the "other" bin */
    if (x < lq->originx)				return &(lq->other);
    if (z < lq->originz)				return &(lq->other);
    if (x >= lq->originx + lq->sizex)	return &(lq->other);
    if (z >= lq->originz + lq->sizez)	return &(lq->other);

    /* if point inside super-brick, compute the bin coordinates */
    ix = (int) (((x - lq->originx) / lq->sizex) * lq->divx);
    iz = (int) (((z - lq->originz) / lq->sizez) * lq->divz);

    /* convert to linear bin number */
    i = lqBinCoordsToBinIndex (lq, ix, 0, iz);

    /* return pointer to that bin */
    return &(lq->bins[i]);
}

// Adds a given client object to a given bin, linking it into the bin contents list.
void lqAddToBin (lqClientProxy* object, lqClientProxy** bin)
{
    if (*bin == NULL)			// if bin is currently empty...
    {
		object->prev = NULL;
		object->next = NULL;
		*bin = object;
    }
    else
    {
		object->prev = NULL;
		object->next = *bin;
		(*bin)->prev = object;
		*bin = object;
    }

    object->bin = bin;			// record bin ID in proxy object
}

// Removes a given client object from its current bin, unlinking it from the bin contents list.
void lqRemoveFromBin (lqClientProxy* object)
{
    if (object->bin != NULL)			// adjust pointers if object is currently in a bin
    {
		if (*(object->bin) == object)			// If this object is at the head of the list, move the bin pointer to the next item in the list (might be NULL).
			*(object->bin) = object->next;

		if (object->prev != NULL)				// If there is a prev object, link its "next" pointer to the object after this one.
			object->prev->next = object->next;

		if (object->next != NULL)				// If there is a next object, link its "prev" pointer to the object before this one.
			object->next->prev = object->prev;
    }

    object->prev	= NULL;		// Null out prev, next and bin pointers of this object.
    object->next	= NULL;
    object->bin		= NULL;
}

// Call for each client object every time its location changes.  For example, in an animation application, this would be called each frame for every moving object.
void lqUpdateForNewLocation(lqInternalDB* lq, lqClientProxy* object, float x, float y, float z)
{
	lqClientProxy** newBin = lqBinForLocation(lq, x, z);		// find bin for new location

    // store location in client object, for future reference
    object->x = x;
    object->y = y;
    object->z = z;

    if (newBin != object->bin)			// has object moved into a new bin?
    {
		lqRemoveFromBin (object);
 		lqAddToBin (object, newBin);
    }
}

// Given a bin's list of client proxies, traverse the list and invoke the given lqCallBackFunction on each object that falls within the search radius.
#define lqTraverseBinClientObjectList(co, radiusSquared, func, state) \
    while (co != NULL)                                                \
    {                                                                 \
	/* compute distance (squared) from this client   */           \
	/* object to given locality sphere's centerpoint */           \
	float dx = x - co->x;                                         \
	float dy = y - co->y;                                         \
	float dz = z - co->z;                                         \
	float distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);    \
                                                                      \
	/* apply function if client object within sphere */           \
	if (distanceSquared < radiusSquared)                          \
	    (*func) (co->object, distanceSquared, state);             \
                                                                      \
	/* consider next client object in bin list */                 \
	co = co->next;                                                \
    }

// This subroutine of lqMapOverAllObjectsInLocality efficiently traverses of subset of bins specified by max and min bin coordinates.
void lqMapOverAllObjectsInLocalityClipped(	lqInternalDB* lq, float x, float y, float z, float radius, lqCallBackFunction func, void* clientQueryState,
											int minBinX, int minBinY,  int minBinZ, int maxBinX, int maxBinY, int maxBinZ)
{
    int i,		j,		k,
		iindex,	jindex,	kindex;

    int slab	= lq->divy * lq->divz;
    int row		= lq->divz;
    int istart	= minBinX * slab;
    int jstart	= minBinY * row;
    int kstart	= minBinZ;
    lqClientProxy* co;
    lqClientProxy** bin;
    float radiusSquared = radius * radius;

    iindex = istart;								// loop for x bins across diameter of sphere
    for (i = minBinX ; i <= maxBinX ; i++)
    {
		jindex = jstart;							// loop for y bins across diameter of sphere
		for (j = minBinY ; j <= maxBinY ; j++)
		{
			kindex = kstart;						// loop for z bins across diameter of sphere
			for (k = minBinZ ; k <= maxBinZ ; k++)
			{
				bin = &lq->bins[iindex + jindex + kindex];	// get current bin's client object list
				co = *bin;

				lqTraverseBinClientObjectList(co, radiusSquared, func, clientQueryState);	// traverse current bin's client object list
				kindex += 1;
			}
			jindex += row;
		}
		iindex += slab;
    }
}

// If the query region (sphere) extends outside of the "super-brick" we need to check for objects in the catch-all "other" bin which holds any 
// object which are not inside the regular sub-bricks
void lqMapOverAllOutsideObjects(lqInternalDB* lq, float x, float y, float z, float radius, lqCallBackFunction func, void* clientQueryState)
{
    lqClientProxy* co	= lq->other;
    float radiusSquared	= radius * radius;

    lqTraverseBinClientObjectList(co, radiusSquared, func, clientQueryState);	// traverse the "other" bin's client object list
}

// Apply an application-specific function to all objects in a certain locality.  The locality is specified as a sphere with a given center and 
// radius.  All objects whose location (key-point) is within this sphere are identified and the function is applied to them.  The 
// application-supplied function takes three arguments:
//		(1) a void* pointer to an lqClientProxy's "object".
//		(2) the square of the distance from the center of the search locality sphere (x,y,z) to object's key-point.
//		(3) a void* pointer to the caller-supplied "client query state" object -- typically NULL, but can be used to store state between calls to the lqCallBackFunction.
// This routine uses the LQ database to quickly reject any objects in bins which do not overlap with the sphere of interest.  Incremental 
// calculation of index values is used to efficiently traverse the bins of interest.
void lqMapOverAllObjectsInLocality (lqInternalDB* lq, float x, float y, float z, float radius, lqCallBackFunction func, void* clientQueryState)
{
    int partlyOut = 0;
    int completelyOutside = (
								((x + radius) < lq->originx) ||
								((y + radius) < lq->originy) ||
								((z + radius) < lq->originz) ||
								((x - radius) >= lq->originx + lq->sizex) ||
								((y - radius) >= lq->originy + lq->sizey) ||
								((z - radius) >= lq->originz + lq->sizez));
    int minBinX, minBinY, minBinZ, maxBinX, maxBinY, maxBinZ;

    if (completelyOutside)									// is the sphere completely outside the "super brick"?
    {
		lqMapOverAllOutsideObjects (lq, x, y, z, radius, func, clientQueryState);
		return;
	}

    // compute min and max bin coordinates for each dimension
    minBinX = (int) ((((x - radius) - lq->originx) / lq->sizex) * lq->divx);
    minBinY = (int) ((((y - radius) - lq->originy) / lq->sizey) * lq->divy);
    minBinZ = (int) ((((z - radius) - lq->originz) / lq->sizez) * lq->divz);
    maxBinX = (int) ((((x + radius) - lq->originx) / lq->sizex) * lq->divx);
    maxBinY = (int) ((((y + radius) - lq->originy) / lq->sizey) * lq->divy);
    maxBinZ = (int) ((((z + radius) - lq->originz) / lq->sizez) * lq->divz);

    // clip bin coordinates
    if (minBinX < 0)         {partlyOut = 1; minBinX = 0;}
    if (minBinY < 0)         {partlyOut = 1; minBinY = 0;}
    if (minBinZ < 0)         {partlyOut = 1; minBinZ = 0;}
    if (maxBinX >= lq->divx) {partlyOut = 1; maxBinX = lq->divx - 1;}
    if (maxBinY >= lq->divy) {partlyOut = 1; maxBinY = lq->divy - 1;}
    if (maxBinZ >= lq->divz) {partlyOut = 1; maxBinZ = lq->divz - 1;}

	if (partlyOut)									// map function over outside objects if necessary (if clipped)
		lqMapOverAllOutsideObjects(lq, x, y, z, radius, func, clientQueryState);
    
    lqMapOverAllObjectsInLocalityClipped(	lq, x, y, z, radius, func, clientQueryState,	// map function over objects in bins
											minBinX, minBinY, minBinZ,
											maxBinX, maxBinY, maxBinZ);
}

typedef struct lqFindNearestState
{
    void* ignoreObject;
    void* nearestObject;
    float minDistanceSquared;
} lqFindNearestState;

void lqFindNearestHelper(void* clientObject, float distanceSquared, void* clientQueryState)
{
    lqFindNearestState* fns = (lqFindNearestState*)clientQueryState;

    if (fns->ignoreObject != clientObject)		// do nothing if this is the "ignoreObject"
    {
		if (fns->minDistanceSquared > distanceSquared)		// record this object if it is the nearest one so far
		{
			fns->nearestObject = clientObject;
			fns->minDistanceSquared = distanceSquared;
		}
    }
}

// Search the database to find the object whose key-point is nearest to a given location yet within a given radius.  That is, it finds the object 
// (if any) within a given search sphere which is nearest to the sphere's center.  The ignoreObject argument can be used to exclude an object from 
// consideration (or it can be NULL).  This is useful when looking for the nearest neighbor of an object in the database, since otherwise it would
// be its own nearest neighbor. The function returns a void* pointer to the nearest object, or NULL if none is found.
void* lqFindNearestNeighborWithinRadius (lqInternalDB* lq,  float x, float y, float z, float radius, void* ignoreObject)
{
    lqFindNearestState lqFNS;				// initialize search state

    lqFNS.nearestObject			= NULL;
    lqFNS.ignoreObject			= ignoreObject;
    lqFNS.minDistanceSquared	= FLT_MAX;

    lqMapOverAllObjectsInLocality(lq, x, y, z, radius, lqFindNearestHelper, &lqFNS);		// map search helper function over all objects within radius

    return lqFNS.nearestObject;			// return nearest object found, if any
}

void lqMapOverAllObjectsInBin(lqClientProxy* binProxyList, lqCallBackFunction func, void* clientQueryState)
{
    while (binProxyList)									// walk down proxy list, applying call-back function to each one
    {
		(*func)(binProxyList->object, 0, clientQueryState);
		binProxyList = binProxyList->next;
    }
}

// Apply a user-supplied function to all objects in the database, regardless of locality (cf lqMapOverAllObjectsInLocality) */
void lqMapOverAllObjects(lqInternalDB* lq, lqCallBackFunction func, void* clientQueryState)
{
    int i;
    int bincount = lq->divx * lq->divz;

    for (i = 0 ; i < bincount ; i++)
		lqMapOverAllObjectsInBin(lq->bins[i], func, clientQueryState);

    lqMapOverAllObjectsInBin(lq->other, func, clientQueryState);
}

// Removes (all proxies for) all objects from all bins
void lqRemoveAllObjects(lqInternalDB* lq)
{
    int i;
    int bincount = lq->divx * lq->divz;

    for (i = 0 ; i < bincount ; i++)
		lqRemoveAllObjectsInBin(lq->bins[i]);

    lqRemoveAllObjectsInBin (lq->other);
}