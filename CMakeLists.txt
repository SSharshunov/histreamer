cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
# cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# set(CMAKE_SYSTEM_NAME Linux)
# set(CMAKE_SYSTEM_PROCESSOR arm)

# set(home /home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer) 

# # set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfs)
# # set(CMAKE_STAGING_PREFIX /home/devel/stage)
# set(CMAKE_STAGING_PREFIX ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
# set(STAGING_DIR ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi)
# set(tools ${home}/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi) 
# # set(tools /home/devel/gcc-4.7-linaro-rpi-gnueabihf)
# set(CMAKE_C_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-gcc)
# set(CMAKE_CXX_COMPILER ${tools}/bin/arm-openwrt-linux-uclibcgnueabi-g++)

# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


set (PROJECT histreamer) 
project (${PROJECT})

# macro(use_c99)
#   if (CMAKE_VERSION VERSION_LESS "3.1")
#     if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
#       set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
#     endif ()
#   else ()
#     set (CMAKE_C_STANDARD 99)
#   endif ()
# endmacro(use_c99)

# project(histreamer)

# find_package(pjsip-hisi 2.9 REQUIRED)
# -DPJ_AUTOCONF=1 -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_IS_BIG_ENDIAN=0 
# add_compile_definitions(PJ_AUTOCONF=1)
# add_compile_definitions(PJ_IS_LITTLE_ENDIAN=1)
# add_compile_definitions(PJ_IS_BIG_ENDIAN=0)

MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE("-- CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
MESSAGE("-- CMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}")

MESSAGE("-- CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

set(CMAKE_INCLUDE_PATH ${CMAKE_PREFIX_PATH}/usr/include/)
MESSAGE("-- CMAKE_INCLUDE_PATH = ${CMAKE_INCLUDE_PATH}")
MESSAGE("-- ARM_TOOLCHAIN_DIR = ${ARM_TOOLCHAIN_DIR}")

# file(GLOB_RECURSE MPP_HEADERS ${CMAKE_PREFIX_PATH}/ hi_common.h)


# find_path(MPP_HEADERS hi_common.h PATHS ${CMAKE_PREFIX_PATH}/usr/include/himpp-dev/)
find_path(MPP_HEADERS hi_common.h PATHS /usr/include/himpp-dev/)

# find_library(ISP_LIB isp)
# find_library(MPI_LIB mpi)
# find_library(HIAE_LIB _hiae)
# find_library(HIAF_LIB _hiaf)
# find_library(HIAWB_LIB _hiawb)
# find_library(AEC_LIB aec)
# find_library(RESAMPLER_LIB resampler)
# find_library(ANR_LIB anr)
# find_library(VQEV2_LIB vqev2)
# find_library(VO_EN_LIB VoiceEngine)
# find_library(DNVQE_LIB dnvqe)
# find_library(UPVQE_LIB upvqe)
# find_library(MPP_LIB hisi_comm_audio)
# find_library(MPP_LIB hisi_comm_isp)
# find_library(MPP_LIB hisi_comm_venc)
# find_library(MPP_LIB hisi_comm_sys)
# find_library(MPP_LIB hisi_comm_vi)

 


# find_path(MPP_LIBS ${MPP_LIB})

# set (MPP_HEADERS ${STAGING_DIR}/usr/include/hisi-osdrv) 
MESSAGE("-- CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
MESSAGE("-- CMAKE_INCLUDE_PATH = ${CMAKE_INCLUDE_PATH}")
MESSAGE("-- MPP_HEADERS = ${MPP_HEADERS}")
MESSAGE("-- MPP_LIBS = ${MPP_LIBS}")
# set (EXTDRV_HEADERS ./extdrv/tw2865) 
# set (GLOBAL_HEADERS ${home}/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi/usr/include) 
set (GLOBAL_LIBS ${CMAKE_PREFIX_PATH}/usr/lib) 
set (GLOBAL_HEADERS ${CMAKE_PREFIX_PATH}/usr/include) 
# set (MPP_LIBS ./lib/) 
# set (MPP_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/lib) 

add_library(hisi_comm_audio STATIC hi_sdk/hisi_comm_audio.c)
add_library(hisi_comm_isp STATIC hi_sdk/hisi_comm_isp.c)
add_library(hisi_comm_venc STATIC hi_sdk/hisi_comm_venc.c)
add_library(hisi_comm_vpss STATIC hi_sdk/hisi_comm_vpss.c)
add_library(hisi_comm_sys STATIC hi_sdk/hisi_comm_sys.c)
add_library(hisi_comm_vi STATIC hi_sdk/hisi_comm_vi.c)
add_library(sensor STATIC hi_sdk/sensor.c)


add_library(audiodev STATIC audiodev.c)
add_library(hisi_audio_dev STATIC hisi_audio_dev.c)

MESSAGE("-- GLOBAL_LIBS = ${GLOBAL_LIBS}")

IF(CMAKE_CONFIGURATION_TYPES MATCHES audio)
	MESSAGE("-- Audio streamer build.")
	# link_directories(/home/ussh/work/tools/netbeans/work/camopenwrt_docker/chaos_calmer/staging_dir/target-arm_arm926ej-s_uClibc-0.9.33.2_eabi/usr/lib/)
	link_directories(${GLOBAL_LIBS})
	add_executable(${PROJECT}-audio ${PROJECT}-audio.c)
	add_library(audio_stream_util STATIC audio_stream_util.c)
	set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")

	include_directories(${MPP_HEADERS})
	# include_directories(${EXTDRV_HEADERS})
	include_directories(${GLOBAL_HEADERS})
	add_definitions(-DPJ_AUTOCONF=1 -DHICHIP=0x35180100 -DHI_DEBUG -DHI_XXXX -DISP_V1 -DPJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO -DPJMEDIA_HAS_H264_HISILICON_PASSTHROUGH_VID_CODEC=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -DSENSOR_TYPE=SONY_IMX138_DC_720P_30FPS -Dhi3518)

	target_link_libraries(${PROJECT}-audio audio_stream_util audiodev hisi_audio_dev pj pjlib-util 
	pjmedia-audiodev pjmedia-codec pjmedia opus uuid pjnath srtp 
	isp mpi _hiae _hiaf _hiawb aec resampler anr vqev2 VoiceEngine dnvqe upvqe
	hisi_comm_audio hisi_comm_isp hisi_comm_venc hisi_comm_sys hisi_comm_vi)

	install(TARGETS ${PROJECT}-audio DESTINATION bin)
ELSEIF(CMAKE_CONFIGURATION_TYPES MATCHES video)
	MESSAGE("-- Video streamer build.")
	set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
	
	include_directories(./extdrv/hi_i2c/)

	link_directories(${GLOBAL_LIBS})
	add_executable(${PROJECT}-video ${PROJECT}-video.c)

	add_library(imx138_sensor_ctl STATIC hi_sdk/imx138_sensor_ctl.c)
	add_library(imx138_cmos STATIC hi_sdk/imx138_cmos.c)
	add_library(hi_sdk_helper STATIC hi_sdk_helper.c)
	add_library(video_stream_util STATIC video_stream_util.c)
	add_library(vid_stream STATIC vid_stream.c)
	add_library(vid_port STATIC vid_port.c)
	add_library(h264_hisilicon_passthrough_vid_codecs STATIC h264_hisilicon_passthrough_vid_codecs.c)
	include_directories(${MPP_HEADERS})
	include_directories(${GLOBAL_HEADERS})

	add_definitions(-DPJ_AUTOCONF=1 -DHICHIP=0x35180100 -DHI_DEBUG -DHI_XXXX -DISP_V1 -DPJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO -DPJMEDIA_HAS_H264_HISILICON_PASSTHROUGH_VID_CODEC=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -DSENSOR_TYPE=SONY_IMX138_DC_720P_30FPS -Dhi3518)
	target_link_libraries(${PROJECT}-video video_stream_util audiodev hisi_audio_dev 
		h264_hisilicon_passthrough_vid_codecs yuv
		vid_port vid_stream hi_sdk_helper pj pjlib-util pjmedia-audiodev pjmedia-codec pjmedia-videodev pjmedia pjnath srtp opus uuid
		 isp mpi _hiae _hiaf _hiawb aec resampler anr vqev2 VoiceEngine dnvqe upvqe 
		 hisi_comm_audio
		hisi_comm_sys hisi_comm_venc hisi_comm_vi hisi_comm_isp hisi_comm_vpss imx138_cmos imx138_sensor_ctl sensor pthread )

	install(TARGETS ${PROJECT}-video DESTINATION bin)
ELSE()
	MESSAGE(FATAL_ERROR "build type not selected")
ENDIF()