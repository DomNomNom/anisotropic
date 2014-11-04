// shared constants between main.cpp and buildCache.cpp

// #define DEFAULT_ANISOTROPY 0.001
#define DEFAULT_ANISOTROPY 0.01
// #define DEFAULT_ANISOTROPY 0.1
#define RESOLUTION_ALPHA 256 * 2
#define RESOLUTION_BETA 256 * 1
#define RESOLUTION_GAMMA 128
// #define RESOLUTION_GAMMA 64
// #define RESOLUTION_GAMMA 32

#define NUMSAMPLES_CACHE 512
// #define NUMSAMPLES_CACHE 100
#define NUMSAMPLES_LIVE   50

typedef enum { ARC, SPHERICAL_HARMONIC } CacheType;

const CacheType cacheType = ARC;
