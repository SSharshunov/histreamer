#PATH=/home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/bin:$PATH
#rm -rf * && cmake -DCMAKE_PREFIX_PATH=/home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi -DCMAKE_TOOLCHAIN_FILE=arm-gcc-toolchain.cmake -DCMAKE_CONFIGURATION_TYPES=audio -DCMAKE_BUILD_TYPE=Release ../
#STAGING_DIR=/home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer/staging_dir make
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)
# ===================================================================
# if(MINGW OR CYGWIN OR WIN32)
#     set(UTIL_SEARCH_CMD where)
# elseif(UNIX OR APPLE)
#     set(UTIL_SEARCH_CMD which)
# endif()

# set(TOOLCHAIN_PREFIX arm-openwrt-linux-)

# execute_process(
#   COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
#   OUTPUT_VARIABLE BINUTILS_PATH
#   OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)
# # Without that flag CMake is not able to pass test compilation check
# if (${CMAKE_VERSION} VERSION_EQUAL "3.6.0" OR ${CMAKE_VERSION} VERSION_GREATER "3.6")
#     set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
# else()
#     set(CMAKE_EXE_LINKER_FLAGS_INIT "--specs=nosys.specs")
# endif()

# set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
# set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

# set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
# set(CMAKE_SIZE_UTIL ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

# set(CMAKE_SYSROOT ${ARM_TOOLCHAIN_DIR}/../arm-openwrt-linux)
# set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
# ==================================================================

set(home /home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer)

# # set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfs)
set(CMAKE_STAGING_PREFIX ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
set(STAGING_DIR ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
set(tools ${home}/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi)
set(CMAKE_C_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-g++)

SET(CMAKE_FIND_ROOT_PATH  ${STAGING_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfs)
# set(CMAKE_STAGING_PREFIX /home/devel/stage)

# set(tools /home/devel/gcc-4.7-linaro-rpi-gnueabihf)
# set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
# set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)


# set(home /home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer) 

# # set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfs)
# # set(CMAKE_STAGING_PREFIX /home/devel/stage)
# set(CMAKE_STAGING_PREFIX ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
# set(STAGING_DIR ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
# set(tools ${home}/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi) 
# # set(tools /home/devel/gcc-4.7-linaro-rpi-gnueabihf)
# set(CMAKE_C_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-gcc)
# set(CMAKE_CXX_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-g++)
