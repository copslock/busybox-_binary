# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/mipseltools-gcc412-lnx26/bin/mipsel-linux-gcc)
SET(CMAKE_CXX_COMPILER /opt/mipseltools-gcc412-lnx26/bin/mipsel-linux-g++)

SET(CMAKE_C_FLAGS   "-pipe -march=mips32 -fomit-frame-pointer -ffast-math -I/mnt/rootfs/mipsel-poky-staging/include")
SET(CMAKE_CXX_FLAGS "-pipe -march=mips32 -fomit-frame-pointer -ffast-math -I/mnt/rootfs/mipsel-poky-staging/include")

SET(CMAKE_EXE_LINKER_FLAGS    "-L/mnt/rootfs/mipsel-poky-staging/lib -Wl,--rpath-link -Wl,/mnt/rootfs/mipsel-poky-staging/lib")
SET(CMAKE_SHARED_LINKER_FLAGS "-L/mnt/rootfs/mipsel-poky-staging/lib -Wl,--rpath-link -Wl,/mnt/rootfs/mipsel-poky-staging/lib")

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH /mnt/rootfs/mipsel-poky-staging /opt/mipseltools-gcc412-lnx26/mipsel-linux)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

