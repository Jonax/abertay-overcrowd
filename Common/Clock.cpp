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
// Keeps track of real clock time and simulation time.  Encapsulates OS's
// time API.  Can be put in either "as fast as possible" variable time step
// mode (where simulation time steps are based on real time elapsed between
// updates), or in fixed "target FPS" mode where the simulation steps are
// constrained to start on 1/FPS boundaries (e.g. on a 60 hertz video game
// console).  Also handles the notion of "pausing" simulation time.
//
// Usage: allocate a clock, set its "paused" or "targetFPS" parameters,
// then call updateGlobalSimulationClock before each simulation step.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 09-24-02 cwr: major overhaul
// 06-26-02 cwr: created
//
//
// ----------------------------------------------------------------------------

#include "OpenSteer/Clock.h"

// ----------------------------------------------------------------------------
// Constructor


OpenSteer::Clock::Clock (void)
{
    // default is "real time, variable frame rate" and not paused
	this->paused = false;

    // real "wall clock" time since launch
    totalRealTime = 0;

    // time simulation has run
    totalSimulationTime = 0;

    // time spent paused
    totalPausedTime = 0;

    // sum of (non-realtime driven) advances to simulation time
    totalAdvanceTime = 0;

    // interval since last simulation time 
    elapsedSimulationTime = 0;

    // interval since last clock update time 
    elapsedRealTime = 0;

    // "manually" advance clock by this amount on next update
    newAdvanceTime = 0;

    // "Calendar time" when this clock was first updated
    basePerformanceCounter = 0;  // from QueryPerformanceCounter on Windows
}


// ----------------------------------------------------------------------------
// update this clock, called once per simulation step ("frame") to:
//
//     track passage of real time
//     manage passage of simulation time (modified by Paused state)
//     measure time elapsed between time updates ("frame rate")
//     optionally: "wait" for next realtime frame boundary


void 
OpenSteer::Clock::update (void)
{
    // wait for next frame time (when targetFPS>0)
    // XXX should this be at the end of the update function?
    frameRateSync ();

    // save previous real time to measure elapsed time
    const float previousRealTime = totalRealTime;

    // real "wall clock" time since this application was launched
    totalRealTime = realTimeSinceFirstClockUpdate ();

    // time since last clock update
    elapsedRealTime = totalRealTime - previousRealTime;

    // accumulate paused time
    if (paused) totalPausedTime += elapsedRealTime;

    // save previous simulation time to measure elapsed time
    const float previousSimulationTime = totalSimulationTime;

    // update total simulation time
    if (ANIMATION_MODE)
    {
        // for "animation mode" use fixed frame time, ignore real time
        const float frameDuration = 1.0f / FIXED_FR;
        totalSimulationTime += paused ? newAdvanceTime : frameDuration;
        if (!paused) newAdvanceTime += frameDuration - elapsedRealTime;
    }
    else
    {
        // new simulation time is total run time minus time spent paused
        totalSimulationTime = (totalRealTime + totalAdvanceTime - totalPausedTime);
    }


    // update total "manual advance" time
    totalAdvanceTime += newAdvanceTime;

    // how much time has elapsed since the last simulation step?
    elapsedSimulationTime = (paused ?
                             newAdvanceTime :
                             (totalSimulationTime - previousSimulationTime));

    // reset advance amount
    newAdvanceTime = 0;
}


// ----------------------------------------------------------------------------
// "wait" until next frame time (actually spin around this tight loop)
//
//
// (xxx there are probably a smarter ways to do this (using events or
// thread waits (eg usleep)) but they are likely to be unportable. xxx)


void 
OpenSteer::Clock::frameRateSync (void)
{
    // when in real time fixed frame rate mode
    // (not animation mode and not variable frame rate mode)
    if ((!ANIMATION_MODE) && (! VARIABLE_FR))
    {
        // find next (real time) frame start time
        const float targetStepSize = 1.0f / FIXED_FR;
        const float now = realTimeSinceFirstClockUpdate ();
        const int lastFrameCount = (int) (now / targetStepSize);
        const float nextFrameTime = (lastFrameCount + 1) * targetStepSize;

        // wait until next frame time
        do {} while (realTimeSinceFirstClockUpdate () < nextFrameTime); 
    }
}


namespace {

// ----------------------------------------------------------------------------
// Returns the number of seconds of real time (represented as a float) since
// the clock was first updated.
//
// XXX Need to revisit conditionalization on operating system.



    float 
    clockErrorExit (void)
    {
        /// @todo - throw? how to handle error conditions? Not by crashing an app!
        std::cerr << "Problem reading system clock - results will not be valid";
        return 0.0f;
    }

} // anonymous namespace

float 
OpenSteer::Clock::realTimeSinceFirstClockUpdate (void)
#ifdef _WIN32
{
    // get time from Windows
    LONGLONG counter, frequency;
    bool clockOK = (QueryPerformanceCounter ((LARGE_INTEGER *)&counter)  &&
                    QueryPerformanceFrequency ((LARGE_INTEGER *)&frequency));
    if (!clockOK) return clockErrorExit ();

    // ensure the base counter is recorded once after launch
    if (basePerformanceCounter == 0) basePerformanceCounter = counter;

    // real "wall clock" time since launch
    const LONGLONG counterDifference = counter - basePerformanceCounter;
    return ((float) counterDifference) / ((float)frequency);
}
#else
{
    // get time from Linux (Unix, Mac OS X, ...)
    timeval t;
    if (gettimeofday (&t, 0) != 0) return clockErrorExit ();

    // ensure the base time is recorded once after launch
    if (baseRealTimeSec == 0)
    {
        baseRealTimeSec = t.tv_sec;
        baseRealTimeUsec = t.tv_usec;
    }

    // real "wall clock" time since launch
    return (( t.tv_sec  - baseRealTimeSec) +
            ((t.tv_usec - baseRealTimeUsec) / 1000000.0f));
}
#endif


// ----------------------------------------------------------------------------
