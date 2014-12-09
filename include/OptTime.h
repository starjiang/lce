#ifndef OPTTIME_H_INCLUDED
#define OPTTIME_H_INCLUDED

#include <stdint.h>
#include <sys/time.h>
#include <string.h>

#ifdef __x86_64__
#define RDTSC() ({ register uint32_t a,d; __asm__ __volatile__( "rdtsc" : "=a"(a), "=d"(d)); (((uint64_t)a)+(((uint64_t)d)<<32)); })
#else
#define RDTSC() ({ register uint64_t tim; __asm__ __volatile__( "rdtsc" : "=A"(tim)); tim; })
#endif

// atomic operations
#ifdef __x86_64__
    #define ASUFFIX "q"
#else
    #define ASUFFIX "l"
#endif
#define CAS(ptr, val_old, val_new)({ char ret; __asm__ __volatile__("lock; cmpxchg"ASUFFIX" %2,%0; setz %1": "+m"(*ptr), "=q"(ret): "r"(val_new),"a"(val_old): "memory"); ret;})

//
// optimized gettimeofday()/time()
//
// Limitations:
//  1, here we assume the CPU speed is 2GB, if your CPU is 4GB, it will run well, but if your CPU is 10GB, please adjust CPU_SPEED_GB
//  2, these functions have precision of 1ms, if you wish higher precision, please adjust REGET_TIME_US, but it will degrade performance
//

#define REGET_TIME_US   1000
#define CPU_SPEED_GB    2  // assume a 2GB CPU

static inline int opt_gettimeofday(struct timeval *tv, void *not_used)
{
     static volatile uint64_t walltick;
     static volatile struct timeval walltime;
     static volatile long lock = 0;
     const unsigned int max_ticks = CPU_SPEED_GB*1000*REGET_TIME_US;

     if(walltime.tv_sec==0 || (RDTSC()-walltick) > max_ticks)
     {
          if(CAS(&lock, 0UL, 1UL)) // try lock
          {
               gettimeofday((struct timeval*)&walltime, NULL);
               walltick = RDTSC();
               lock = 0; // unlock
          }
          else // if try lock failed, use system time
          {
               return gettimeofday(tv, (__timezone_ptr_t)not_used);
          }
     }
     memcpy(tv, (void*)&walltime, sizeof(struct timeval));
     return 0;
}

static inline time_t opt_time(time_t *t)
{
     struct timeval tv;
     opt_gettimeofday(&tv, NULL);
     if(t) *t = tv.tv_sec;
     return tv.tv_sec;
}

#define gettimeofday(a, b) opt_gettimeofday(a, b)
#define time(t) opt_time(t)


#endif // OPTTIME_H_INCLUDED
