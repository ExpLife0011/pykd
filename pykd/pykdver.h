
#define __VER_STR2__(x) #x
#define __VER_STR1__(x) __VER_STR2__(x)

#define PYKD_VERSION_MAJOR  1
#define PYKD_VERSION_MINOR  5

#define PYKD_VERSION_BUILD  \
    "0." __VER_STR1__(PYKD_VERSION_MAJOR) ".0." __VER_STR1__(PYKD_VERSION_MINOR)
