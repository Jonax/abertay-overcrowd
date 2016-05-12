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
// discrete time simulation clock for OpenSteerDemo
//
// Keeps track of real clock time and simulation time.  Encapsulates the time
// API of the underlying operating system.  Can be put in either "as fast as
// possible" variable time step mode (where simulation time steps are based on
// real time elapsed between updates), or in fixed "target FPS" mode where the
// simulation steps are constrained to start on 1/FPS boundaries (e.g. on a 60
// hertz video game console).  Also handles the notion of "pausing" simulation
// time.
//
// Usage: allocate a clock, set its "paused" or "targetFPS" parameters, then
// call updateGlobalSimulationClock before each simulation step.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-11-03 cwr: another overhaul: support aniamtion mode, switch to
//               functional API, move smoothed stats inside this class
// 09-24-02 cwr: major overhaul
// 06-26-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_CLOCK_H
#define OPENSTEER_CLOCK_H

#include <iostream>  // for ostream, <<, etc.
#include <windows.h>

#if defined ANIMATION_60									// animation mode at 60 fps
	#define FIXED_FR		60
	#define ANIMATION_MODE	true
	#define VARIABLE_FR		false
#elif defined REALTIME_FIXED_60								// real-time fixed frame rate mode at 60 fps
	#define FIXED_FR		60
	#define ANIMATION_MODE	false
	#define VARIABLE_FR		false
#elif defined REALTIME_FIXED_24								// real-time fixed frame rate mode at 24 fps
	#define FIXED_FR		24
	#define ANIMATION_MODE	false
	#define VARIABLE_FR		false
#else														// real-time variable frame rate mode ("as fast as possible")
	#define FIXED_FR		60
	#define ANIMATION_MODE	false
	#define VARIABLE_FR		true
#endif

namespace OpenSteer {

    class Clock
    {
		public:
			Clock();		// constructor
			void update();	// update this clock, called exactly once per simulation step ("frame")
			float realTimeSinceFirstClockUpdate(void);		// returns the number of seconds of real time (represented as a float) since the clock was first updated.
			void frameRateSync (void);					// "wait" until next frame time

        // main clock modes: variable or fixed frame rate, real-time or animation
        // mode, running or paused.
	public:
        bool paused;					// is simulation running or paused?

        float totalRealTime;				// real "wall clock" time since launch
        float totalSimulationTime;			// total time simulation has run
        float totalPausedTime;				// total time spent paused
        float totalAdvanceTime;				// sum of (non-realtime driven) advances to simulation time

        float elapsedSimulationTime;		// interval since last simulation time
        float elapsedRealTime;				// interval since last clock update time 

    private:
        float newAdvanceTime;				// "manually" advance clock by this amount on next update
        LONGLONG basePerformanceCounter;	// "Calendar time" when this clock was first updated
    };

} // namespace OpenSteer


// ----------------------------------------------------------------------------
#endif // OPENSTEER_CLOCK_H
