// shared constants between main.cpp and buildCache.cpp

#define DEFAULT_ANISOTROPY 0.001

#define RESOLUTION_ALPHA 256 * 2
#define RESOLUTION_BETA 256 * 2
#define RESOLUTION_GAMMA 32

typedef enum { ARC, SPHERICAL_HARMONIC } CacheType;

const CacheType cacheType = ARC;
