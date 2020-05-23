/*
 * profiling/timing
 * 
 * timeThis(FN) 
 *  will profile any function FN given as argument
 *  
 * when PROFILING is not defined, it just executes the FN
 * when PROFILING is defined, it times the duration
 * when duration in above PROFILING_THRESHOLD it prints duration
 * 
 * PROFILING_THRESHOLD default to 3ms
 * 
 * defing PROFILING and PROFILING_THRESHOLD before including timing.h
 *  
 */
 
#ifndef PROFILING_INCLUDED
#define PROFILING_INCLUDED

#ifdef PROFILING

        #ifndef PROFILING_THRESHOLD
        #define PROFILING_THRESHOLD 3
        #endif

        #define timeThis(FN)    ({ unsigned long start_time=millis(); \
                                FN; \
                                unsigned long duration=millis() - start_time; \
                                yield(); \
                                if (duration >= PROFILING_THRESHOLD) \
                                DebugTf("Function %s [called from %s:%d] took %lu ms\n",\
                                        #FN, __FUNCTION__, __LINE__, duration); \
                                })
#else // PROFILING

        #define timeThis(FN)    FN ;

#endif // PROFILING

/*
 * DECLARE_TIMER(timername, interval)
 *  Declares two unsigned longs: 
 *    <timername>_last for last execution
 *    <timername>_interval for interval in seconds
 *    
 *    
 * DECLARE_TIMERms is same as DECLARE_TIMER **but** uses milliseconds!
 *    
 * DUE(timername) 
 *  returns false (0) if interval hasn't elapsed since last DUE-time
 *          true (current millis) if it has
 *  updates <timername>_last
 *  
 *  Usage example:
 *  
 *  DECLARE_TIMER(screenUpdate, 200) // update screen every 200 ms
 *  ...
 *  loop()
 *  {
 *  ..
 *    if ( DUE(screenUpdate) ) {
 *      // update screen
 *    }
 *  }
 */
#define DECLARE_TIMERm(timerName, timerTime)    static unsigned long timerName##_interval = timerTime * 60 * 1000,      timerName##_last = millis()+random(timerName##_interval);
#define DECLARE_TIMER(timerName, timerTime)     static unsigned long timerName##_interval = timerTime * 1000,           timerName##_last = millis()+random(timerName##_interval);
#define DECLARE_TIMERms(timerName, timerTime)   static unsigned long timerName##_interval = timerTime,                  timerName##_last = millis()+random(timerName##_interval);

#define DECLARE_TIMERs DECLARE_TIMER

#define SINCE(timerName)  (millis() - timerName##_last)
#define DUE(timerName) (( SINCE(timerName) < timerName##_interval) ? 0 : (timerName##_last=millis()))

#endif // PROFILING_INCLUDED
