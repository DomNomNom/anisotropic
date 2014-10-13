// shared constants between main.cpp and buildCache.cpp

#define GAMMA_SLICES 64
#define DEFAULT_ANISOTROPY 0.001

typedef enum { ARC, SPHERICAL_HARMONIC } CacheType;

const CacheType cacheType = ARC;
