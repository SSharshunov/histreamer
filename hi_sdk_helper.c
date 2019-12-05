#if PJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
    
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_isp.h"
    
#include <pj/log.h>

#include "hi_sdk/hisi_comm.h"  
    
#include <pjlib-util/json.h>
#include <pj/file_access.h>
    
#include <pj/string.h>
#include <pjlib-util/errno.h>
#include <pj/file_io.h>

//#define SAMPLE_AUDIO_PTNUMPERFRM   320
    
//VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
    
#define THIS_FILE           "hi_sdk_helper.c"
    
    
static HI_BOOL gs_bMicIn = HI_FALSE;
static HI_BOOL gs_bAiAnr = HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
//static HI_BOOL gs_bUserGetMode = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;

#define MY_DBG()\
do{\
    PJ_LOG(4, (THIS_FILE, "%s: After line #%d", __FUNCTION__, __LINE__));\
}while(0)

#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)
/******************************************************************************
* function : Ai -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32 s32Ret, s32AiChnCnt;
    AUDIO_DEV   AiDev = 0;
    AI_CHN      AiChn = 0;
    AUDIO_DEV   AoDev = 0;
    AO_CHN      AoChn = 0;

    /* config aio dev attr */
    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    /* config audio codec */
    s32Ret = HISI_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    
    /* enable AI channle */
    s32AiChnCnt = pstAioAttr->u32ChnCnt;
    s32Ret = HISI_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    
    /* enable AO channle */
    pstAioAttr->u32ChnCnt = 2;
    s32Ret = HISI_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    /* bind AI to AO channle */
//    if (HI_TRUE == gs_bUserGetMode)
//    {
//        s32Ret = HISI_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
//        if (s32Ret != HI_SUCCESS)
//        {
//            SAMPLE_DBG(s32Ret);
//            return HI_FAILURE;
//        }
//    }
//    else
//    {
//        s32Ret = HISI_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
//        if (s32Ret != HI_SUCCESS)
//        {
//            SAMPLE_DBG(s32Ret);
//            return HI_FAILURE;
//        }
//    }
    printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);

    printf("\nplease press twice ENTER to exit this sample\n");
//    getchar();
//    getchar();

//    if (HI_TRUE == gs_bUserGetMode)
//    {
//        HISI_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
//    }
//    else
//    {
//        HISI_COMM_AUDIO_AoUnbindAi(AiDev, AiChn, AoDev, AoChn);
//    }
    HISI_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);
    HISI_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
    return HI_SUCCESS;
}

HI_S32 AUDIO_OUT(HI_VOID) {
    HI_S32 s32Ret= HI_SUCCESS;
    VB_CONF_S stVbConf;
    AIO_ATTR_S stAioAttr;
    
    /* init stAio. all of cases will use it */
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 1;
    stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 1;
    
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = HISI_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }
    
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: system init failed with %#x!", __FUNCTION__, s32Ret));
        // printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: system init success with %#x!", __FUNCTION__, s32Ret));
    }
    
    s32Ret = SAMPLE_AUDIO_AiAo(&stAioAttr);

    return HI_SUCCESS;
}

//pj_json_elem* load_json_config(pj_pool_t *pool, const char *filename, pj_off_t *size) {
pj_json_elem* load_json_config(pj_pool_t *pool, const char *filename) {
    pj_json_elem* root;
    if (!pj_file_exists(filename)){
	PJ_LOG(1,(THIS_FILE, "File \"%s\" not found! PJ_ENOTFOUND", filename));
        return NULL;
    }

    pj_off_t size = (pj_off_t)pj_file_size(filename);
//    size = (pj_ssize_t)pj_file_size(filename);
//    pj_ssize_t size = (pj_ssize_t)pj_file_size(filename);
    pj_status_t status;

    char *buffer = (char*)pj_pool_alloc(pool, size+1);
    pj_oshandle_t fd = 0;
    unsigned parse_size;
    char err_msg[120];
    pj_json_err_info err_info;

    err_msg[0] = '\0';

    status = pj_file_open(pool, filename, PJ_O_RDONLY, &fd);
    if (status != PJ_SUCCESS)
	goto on_error;

    status = pj_file_read(fd, buffer, (pj_ssize_t*)&size);
    if (status != PJ_SUCCESS)
	goto on_error;

    pj_file_close(fd);
    fd = NULL;

    if (size <= 0) {
	status = PJ_EEOF;
	goto on_error;
    }

    parse_size = (unsigned)size;
    root = pj_json_parse(pool, buffer, &parse_size, &err_info);
    if (root == NULL) {
	pj_ansi_snprintf(err_msg, sizeof(err_msg),
	                 "JSON parsing failed: syntax error in file '%s' at "
	                 "line %d column %d",
	                 filename, err_info.line, err_info.col);
	PJ_LOG(1,(THIS_FILE, err_msg));
	status = PJLIB_UTIL_EINJSON;
	goto on_error;
    }
    return root;
on_error:
    if (fd)
	pj_file_close(fd);
//    if (err_msg[0])
//	PJSUA2_RAISE_ERROR3(status, "loadFile()", err_msg);
//    else
//	PJSUA2_RAISE_ERROR(status);
}

pj_json_elem* find_value_by_name(pj_json_elem* root_value, char *value_name) {
    if (root_value == NULL) {
        return NULL;
    }
    if (root_value->type != PJ_JSON_VAL_OBJ) {
        printf("this is object\n");
    }
    pj_str_t t_name = pj_str(value_name);
    
    struct pj_json_elem *tmp = (struct pj_json_elem *)root_value->value.children.next;
    
    while (tmp->next != NULL) {
        if (pj_strcmp(&tmp->name, &t_name) == 0) {
            return tmp;
        } else {
            tmp = (struct pj_json_elem *)tmp->next;
        };
    }
    return NULL;
}

//HISI_SDK_APP_S fill_app_from_config(ini_t *config, HISI_SDK_APP_S *app) {
//void fill_app_from_config(pj_json_elem* value, HISI_SDK_APP_S *app) {
HISI_SDK_APP_S* fill_app_from_config(pj_pool_t *pool, pj_json_elem* value) {
//void fill_app_from_config(json_value* value, HISI_SDK_APP_S *app) {
//    HISI_SDK_APP_S* app = (HISI_SDK_APP_S*) pj_pool_calloc();
    HISI_SDK_APP_S* app = (HISI_SDK_APP_S*) pj_pool_calloc(pool, 1, sizeof(HISI_SDK_APP_S));
    int dbg = 0;
    
//    memset(app, 0, sizeof(HISI_SDK_APP_S));

    app->init_stage_0 = HI_FALSE;
    app->init_stage_1 = HI_FALSE;
    app->init_stage_2 = HI_FALSE;
    app->init_stage_3 = HI_FALSE;
    app->init_stage_4 = HI_FALSE;
    app->init_stage_5 = HI_FALSE;
//
//    /******************************************
//     step  1: init sys variable
//    ******************************************/
//
////    memset(&app->stVbConf, 0, sizeof(VB_CONF_S));
//
//    app->stVbConf.u32MaxPoolCnt = 128;
//
    pj_json_elem* sensor_json_object = find_value_by_name(value, "sensor");
    if (sensor_json_object == NULL) {
        HISI_PRT("sensor not found\n");
//        json_value_free(sensor_json_object);
    }
    pj_json_elem* sensor_lib_name = find_value_by_name(sensor_json_object, "lib_name");
    if (sensor_lib_name == NULL) {
        HISI_PRT("lib_name not found\n");
    } else {
//        HISI_PRT("lib_name: %s\n", sensor_lib_name->value.str);
//        memcpy
        app->stViConfig.sensor.lib_name = (char*) pj_pool_calloc(pool, sensor_lib_name->value.str.slen, sizeof(char));
//        app->stViConfig.sensor.lib_name = (char*)malloc(sensor_lib_name->value.str.slen);
        if (app->stViConfig.sensor.lib_name == NULL) {
            fprintf(stderr, "Memory error: unable to allocate %d bytes\n", sensor_lib_name->value.str.slen);
            return NULL;
        }
        pj_memcpy(app->stViConfig.sensor.lib_name, sensor_lib_name->value.str.ptr, sensor_lib_name->value.str.slen);
//        memcpy(app->stViConfig.sensor.lib_name, sensor_lib_name->value.str.ptr, sensor_lib_name->value.str.slen);
//        app->stViConfig.sensor.lib_name = sensor_lib_name->value.str.ptr;
//        json_value_free(sensor_lib_name);
    }
    if (dbg == 1) HISI_PRT("app->stViConfig.sensor.lib_name = \"%s\"\n", app->stViConfig.sensor.lib_name);

    pj_json_elem* sensor_name = find_value_by_name(sensor_json_object, "sensor_name");
    if (sensor_name == NULL) {
        HISI_PRT("sensor_name not found\n");
    } else {
//        HISI_PRT("sensor_name: %s\n", sensor_name->value.str.ptr);
//        HISI_PRT("sensor_name = \"%d\"\n", sensor_name->value.str.slen);
//        memcpy
        app->stViConfig.sensor.sensor_name = (char*) pj_pool_calloc(pool, sensor_name->value.str.slen, sizeof(char));
//        app->stViConfig.sensor.sensor_name = (char*)malloc(sensor_name->value.str.slen);
        if (app->stViConfig.sensor.sensor_name == NULL) {
            fprintf(stderr, "Memory error: unable to allocate %d bytes\n", sensor_name->value.str.slen);
            return NULL;
        }
        pj_memcpy(app->stViConfig.sensor.sensor_name, sensor_name->value.str.ptr, sensor_name->value.str.slen);
//        memcpy(app->stViConfig.sensor.sensor_name, sensor_name->value.str.ptr, sensor_name->value.str.slen);
//        app->stViConfig.sensor.lib_name = sensor_lib_name->u.string.ptr;
//        json_value_free(sensor_name);
    }
    if (dbg == 1) HISI_PRT("app->stViConfig.sensor.sensor_name = \"%s\"\n", app->stViConfig.sensor.sensor_name);
//    HISI_PRT("sensor_lib_name->u.string.ptr = %s/\n", sensor_lib_name->u.string.ptr);

    pj_json_elem* sensor_s_enNorm = find_value_by_name(sensor_json_object, "s_enNorm");
    if (sensor_s_enNorm == NULL) {
        HISI_PRT("s_enNorm not found\n");
    } else {
        app->gs_enNorm = (int) sensor_s_enNorm->value.num;
        if (dbg == 1) HISI_PRT("gs_enNorm: %u\n", app->gs_enNorm);
//        json_value_free(sensor_s_enNorm);
    }

    pj_json_elem* sensor_enSize = find_value_by_name(sensor_json_object, "enSize");
    if (sensor_enSize == NULL) {
        HISI_PRT("enSize not found\n");
    } else {
        app->enSize = (int) sensor_enSize->value.num;
//        json_value_free(sensor_enSize);
        if (dbg == 1) HISI_PRT("enSize: %u\n", app->enSize);
    }

    pj_json_elem* sensor_enPayLoad = find_value_by_name(sensor_json_object, "enPayLoad");
    if (sensor_enPayLoad == NULL) {
        HISI_PRT("enPayLoad not found\n");
    } else {
        app->enPayLoad = (int) sensor_enPayLoad->value.num;
//        json_value_free(sensor_enPayLoad);
        if (dbg == 1) HISI_PRT("enPayLoad: %u\n", app->enPayLoad);
    }

    pj_json_elem* sensor_enRcMode = find_value_by_name(sensor_json_object, "enRcMode");
    if (sensor_enRcMode == NULL) {
        HISI_PRT("enRcMode not found\n");
    } else {
        app->enRcMode = (int) sensor_enRcMode->value.num;
//        json_value_free(sensor_enRcMode);
        if (dbg == 1) HISI_PRT("enRcMode: %u\n", app->enRcMode);
    }

    /*video buffer*/
    app->u32BlkSize = HISI_COMM_SYS_CalcPicVbBlkSize(app->gs_enNorm,\
                app->enSize, HISI_PIXEL_FORMAT, HISI_SYS_ALIGN_WIDTH);
    if (dbg == 1) HISI_PRT("app->u32BlkSize: %d\n", app->u32BlkSize);
    app->stVbConf.astCommPool[0].u32BlkSize = app->u32BlkSize;
    app->stVbConf.astCommPool[0].u32BlkCnt = 10;
//    app->stVbConf.astCommPool[1].u32BlkSize = app->u32BlkSize;
//    app->stVbConf.astCommPool[1].u32BlkCnt = 10;
//    app->stVbConf.astCommPool[2].u32BlkSize = app->u32BlkSize;
//    app->stVbConf.astCommPool[2].u32BlkCnt = 10;
//    app->stVbConf.astCommPool[3].u32BlkSize = app->u32BlkSize;
//    app->stVbConf.astCommPool[3].u32BlkCnt = 10;
    if (dbg == 1) HISI_PRT("app->stVbConf.astCommPool[0].u32BlkSize: %d\n", app->stVbConf.astCommPool[0].u32BlkSize);
    if (dbg == 1) HISI_PRT("app->stVbConf.astCommPool[0].u32BlkCnt: %d\n", app->stVbConf.astCommPool[0].u32BlkCnt);

    HI_S32 s32Ret = HI_SUCCESS;


    s32Ret = HISI_COMM_SYS_GetPicSize(app->gs_enNorm, app->enSize, &app->stSize);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HISI_COMM_SYS_GetPicSize failed, using 1280*720!\n");
        app->stSize.u32Height = 720;
        app->stSize.u32Width  = 1280;
    }

    HISI_PRT("HISI_COMM_SYS_GetPicSize return u32Width = %d, u32Height = %d\n", app->stSize.u32Width, app->stSize.u32Height);


//    memset(&app->stViConfig, 0, sizeof(HISI_VI_CONFIG_S));
//    sprintf(app->stViConfig.sensor.lib_name, "%s", lib_name);
//    sprintf(app->stViConfig.sensor.sensor_name, "%s", sensor_name);

    pj_json_elem* sensor_image_attrs_object = find_value_by_name(sensor_json_object, "image_attrs");
    if (sensor_image_attrs_object == NULL) {
        HISI_PRT("sensor_image_attrs_object not found\n");
    } else {
// <-------------------------------------------------------------------------------------------------------------
        pj_json_elem* sensor_enBayer = find_value_by_name(sensor_image_attrs_object, "enBayer");
        if (sensor_enBayer == NULL) {
            HISI_PRT("enBayer not found\n");
        } else {
            app->stViConfig.sensor.image_attrs.enBayer = (int) sensor_enBayer->value.num;
//            json_value_free(sensor_enBayer);
            if (dbg == 1) HISI_PRT("enBayer: %u\n", app->stViConfig.sensor.image_attrs.enBayer);
        }

        pj_json_elem* sensor_u16FrameRate = find_value_by_name(sensor_image_attrs_object, "u16FrameRate");
        if (sensor_u16FrameRate == NULL) {
            HISI_PRT("u16FrameRate not found\n");
        } else {
            app->stViConfig.sensor.image_attrs.u16FrameRate = (int) sensor_u16FrameRate->value.num;
//            json_value_free(sensor_u16FrameRate);
            if (dbg == 1) HISI_PRT("u16FrameRate: %u\n", app->stViConfig.sensor.image_attrs.u16FrameRate);
        }
// <-------------------------------------------------------------------------------------------------------------
//        json_value_free(sensor_image_attrs_object);
    }

//    app->stViConfig.sensor.image_attrs.enBayer      = BAYER_GRBG;
//    app->stViConfig.sensor.image_attrs.u16FrameRate = 25;
    app->stViConfig.sensor.image_attrs.u16Width     = app->stSize.u32Width;
    app->stViConfig.sensor.image_attrs.u16Height    = app->stSize.u32Height;

    pj_json_elem* sensor_enWndMode = find_value_by_name(sensor_json_object, "enWndMode");
    if (sensor_enWndMode == NULL) {
        HISI_PRT("enWndMode not found\n");
    } else {
        app->stViConfig.sensor.enWndMode = (int) sensor_enWndMode->value.num;
//        json_value_free(sensor_enWndMode);
        if (dbg == 1) HISI_PRT("enWndMode: %u\n", app->stViConfig.sensor.enWndMode);
    }

    pj_json_elem* sensor_u16HorWndStart = find_value_by_name(sensor_json_object, "u16HorWndStart");
    if (sensor_u16HorWndStart == NULL) {
        HISI_PRT("u16HorWndStart not found\n");
    } else {
        app->stViConfig.sensor.u16HorWndStart = (int) sensor_u16HorWndStart->value.num;
//        json_value_free(sensor_u16HorWndStart);
        if (dbg == 1) HISI_PRT("u16HorWndStart: %u\n", app->stViConfig.sensor.u16HorWndStart);
    }

    pj_json_elem* sensor_u16VerWndStart = find_value_by_name(sensor_json_object, "u16VerWndStart");
    if (sensor_u16VerWndStart == NULL) {
        HISI_PRT("u16VerWndStart not found\n");
    } else {
        app->stViConfig.sensor.u16VerWndStart = (int) sensor_u16VerWndStart->value.num;
//        json_value_free(sensor_u16VerWndStart);
        if (dbg == 1) HISI_PRT("u16VerWndStart: %u\n", app->stViConfig.sensor.u16VerWndStart);
    }


//    app->stViConfig.sensor.enWndMode = ISP_WIND_ALL; //for sony or pana
    app->stViConfig.sensor.u16HorWndLength = app->stSize.u32Width;
    app->stViConfig.sensor.u16VerWndLength = app->stSize.u32Height;
//    app->stViConfig.sensor.u16HorWndStart = 72;
//    app->stViConfig.sensor.u16VerWndStart = 20;


    pj_json_elem* sensor_vi_dev_attrs_object = find_value_by_name(sensor_json_object, "vi_dev_attrs");
    if (sensor_vi_dev_attrs_object == NULL) {
        HISI_PRT("sensor_vi_dev_attrs_object not found\n");
    } else {
        pj_json_elem* sensor_enIntfMode = find_value_by_name(sensor_vi_dev_attrs_object, "enIntfMode");
        if (sensor_enIntfMode == NULL) {
            HISI_PRT("enIntfMode not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enIntfMode = (int) sensor_enIntfMode->value.num;
//            json_value_free(sensor_enIntfMode);
            if (dbg == 1) HISI_PRT("enIntfMode: %u\n", app->stViConfig.sensor.vi_dev_attrs.enIntfMode);
        }
        pj_json_elem* sensor_enWorkMode = find_value_by_name(sensor_vi_dev_attrs_object, "enWorkMode");
        if (sensor_enWorkMode == NULL) {
            HISI_PRT("enWorkMode not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enWorkMode = (int) sensor_enWorkMode->value.num;
//            json_value_free(sensor_enWorkMode);
            if (dbg == 1) HISI_PRT("enWorkMode: %u\n", app->stViConfig.sensor.vi_dev_attrs.enWorkMode);
        }

        pj_json_elem* sensor_au32CompMask = find_value_by_name(sensor_vi_dev_attrs_object, "au32CompMask");
        if (sensor_au32CompMask == NULL) {
            HISI_PRT("au32CompMask not found\n");
        } else {
//            HISI_PRT("sensor_au32CompMask[0] 0x%X\n", (HI_U32)strtoul(sensor_au32CompMask->value.children.next->value.str.ptr, NULL, 16));
//            HISI_PRT("sensor_au32CompMask[1] 0x%X\n", (HI_U32)strtoul(sensor_au32CompMask->value.children.next->next->value.str.ptr, NULL, 16));
//            HISI_PRT("sensor_au32CompMask %s\n", sensor_au32CompMask->value.children.next->value.str);
//            HISI_PRT("sensor_au32CompMask %s\n", sensor_au32CompMask->value.children.next->next->value.str);
            app->stViConfig.sensor.vi_dev_attrs.au32CompMask[0] = \
                    (HI_U32)strtoul(sensor_au32CompMask->value.children.next->value.str.ptr, NULL, 16);
//            app->stViConfig.sensor.vi_dev_attrs.au32CompMask[0] = \
//                    (int)strtoul(sensor_au32CompMask->u.array.values[0]->u.string.ptr, NULL, 16);
            app->stViConfig.sensor.vi_dev_attrs.au32CompMask[1] = \
                    (HI_U32)strtoul(sensor_au32CompMask->value.children.next->next->value.str.ptr, NULL, 16);
//            app->stViConfig.sensor.vi_dev_attrs.au32CompMask[1] = \
//                    (int)strtoul(sensor_au32CompMask->u.array.values[1]->u.string.ptr, NULL, 16);
//            json_value_free(sensor_au32CompMask);
            if (dbg == 1) HISI_PRT("au32CompMask[0]: 0x%X\n", app->stViConfig.sensor.vi_dev_attrs.au32CompMask[0]);
            if (dbg == 1) HISI_PRT("au32CompMask[1]: 0x%X\n", app->stViConfig.sensor.vi_dev_attrs.au32CompMask[1]);
        }

        pj_json_elem* sensor_enScanMode = find_value_by_name(sensor_vi_dev_attrs_object, "enScanMode");
        if (sensor_enScanMode == NULL) {
            HISI_PRT("enScanMode not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enScanMode = (int) sensor_enScanMode->value.num;
//            json_value_free(sensor_enScanMode);
            if (dbg == 1) HISI_PRT("enScanMode: %u\n", app->stViConfig.sensor.vi_dev_attrs.enScanMode);
        }

        pj_json_elem* sensor_s32AdChnId = find_value_by_name(sensor_vi_dev_attrs_object, "s32AdChnId");
        if (sensor_s32AdChnId == NULL) {
            HISI_PRT("s32AdChnId not found\n");
        } else {
            
//            HISI_PRT("\n\n\n\nsensor_s32AdChnId->value.children.next->value.num: %d\n\n\n\n", (HI_S32) sensor_s32AdChnId->value.children.next->value.num);
//            HISI_PRT("\n\n\n\nsensor_s32AdChnId->value.children.next->name: %d\n\n\n\n",      (HI_S32) sensor_s32AdChnId->value.children.next->value.num);
//            HISI_PRT("\n\n\n\nsensor_s32AdChnId->value.children.next->name: %d\n\n\n\n",      (HI_S32) sensor_s32AdChnId->value.children.next->next->value.num);
//            HISI_PRT("\n\n\n\nsensor_s32AdChnId->value.children.next->name: %d\n\n\n\n",      (HI_S32) sensor_s32AdChnId->value.children.next->next->next->value.num);
//            HISI_PRT("\n\n\n\nsensor_s32AdChnId->value.children.next->name: %d\n\n\n\n",      (HI_S32) sensor_s32AdChnId->value.children.next->next->next->next->value.num);
            
            app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[0] = \
                    (HI_S32) sensor_s32AdChnId->value.children.next->value.num;
//                    (HI_S32)sensor_s32AdChnId->u.array.values[0]->u.integer;
            app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[1] = \
                    (HI_S32) sensor_s32AdChnId->value.children.next->next->value.num;
//                    (HI_S32)sensor_s32AdChnId->u.array.values[1]->u.integer;
            app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[2] = \
                    (HI_S32) sensor_s32AdChnId->value.children.next->next->next->value.num;
//                    (HI_S32)sensor_s32AdChnId->u.array.values[2]->u.integer;
            app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[3] = \
                    (HI_S32) sensor_s32AdChnId->value.children.next->next->next->next->value.num;
//                    (HI_S32)sensor_s32AdChnId->u.array.values[3]->u.integer;
//            json_value_free(sensor_au32CompMask);
            if (dbg == 1) {
                HISI_PRT("s32AdChnId[0]: %d\n", app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[0]);
                HISI_PRT("s32AdChnId[1]: %d\n", app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[1]);
                HISI_PRT("s32AdChnId[2]: %d\n", app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[2]);
                HISI_PRT("s32AdChnId[3]: %d\n", app->stViConfig.sensor.vi_dev_attrs.s32AdChnId[3]);
            }
        }

        pj_json_elem* sensor_enDataSeq = find_value_by_name(sensor_vi_dev_attrs_object, "enDataSeq");
        if (sensor_enDataSeq == NULL) {
            HISI_PRT("enDataSeq not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enDataSeq = (int) sensor_enDataSeq->value.num;
//            json_value_free(sensor_enDataSeq);
            if (dbg == 1) HISI_PRT("enDataSeq: %u\n", app->stViConfig.sensor.vi_dev_attrs.enDataSeq);
        }

//        json_value* sensor_enDataSeq = find_value_by_name(sensor_vi_dev_attrs_object, "enDataSeq");
//        if (sensor_enDataSeq == NULL) {
//            HISI_PRT("enDataSeq not found\n");
//        } else {
//            app->stViConfig.sensor.vi_dev_attrs.enDataSeq = sensor_enDataSeq->u.integer;
//            json_value_free(sensor_enDataSeq);
//            HISI_PRT("enDataSeq: %u\n", app->stViConfig.sensor.vi_dev_attrs.enDataSeq);
//        }

        pj_json_elem* sensor_vi_dev_attrs_stSynCfg_object = find_value_by_name(sensor_vi_dev_attrs_object, "stSynCfg");
        if (sensor_vi_dev_attrs_stSynCfg_object == NULL) {
            HISI_PRT("sensor_vi_dev_attrs_object not found\n");
        } else {
            pj_json_elem* sensor_enVsync = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enVsync");
            if (sensor_enVsync == NULL) {
                HISI_PRT("enVsync not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsync = (int) sensor_enVsync->value.num;
//                json_value_free(sensor_enVsync);
                if (dbg == 1) HISI_PRT("enVsync: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsync);
            }

            pj_json_elem* sensor_enVsyncNeg = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enVsyncNeg");
            if (sensor_enVsyncNeg == NULL) {
                HISI_PRT("enVsyncNeg not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncNeg = (int) sensor_enVsyncNeg->value.num;
//                json_value_free(sensor_enVsyncNeg);
                if (dbg == 1) HISI_PRT("enVsyncNeg: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncNeg);
            }
            pj_json_elem* sensor_enHsync = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enHsync");
            if (sensor_enHsync == NULL) {
                HISI_PRT("enHsync not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enHsync = (int) sensor_enHsync->value.num;
//                json_value_free(sensor_enHsync);
                if (dbg == 1) HISI_PRT("enHsync: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enHsync);
            }
            pj_json_elem* sensor_enHsyncNeg = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enHsyncNeg");
            if (sensor_enHsyncNeg == NULL) {
                HISI_PRT("enHsyncNeg not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enHsyncNeg = (int) sensor_enHsyncNeg->value.num;
//                json_value_free(sensor_enHsyncNeg);
                if (dbg == 1) HISI_PRT("enHsyncNeg: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enHsyncNeg);
            }
            pj_json_elem* sensor_enVsyncValid = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enVsyncValid");
            if (sensor_enVsyncValid == NULL) {
                HISI_PRT("enVsyncValid not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncValid = (int) sensor_enVsyncValid->value.num;
//                json_value_free(sensor_enVsyncValid);
                if (dbg == 1) HISI_PRT("enVsyncValid: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncValid);
            }
            pj_json_elem* sensor_enVsyncValidNeg = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "enVsyncValidNeg");
            if (sensor_enVsyncValidNeg == NULL) {
                HISI_PRT("enVsyncValidNeg not found\n");
            } else {
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncValidNeg = (int) sensor_enVsyncValidNeg->value.num;
//                json_value_free(sensor_enVsyncValidNeg);
                if (dbg == 1) HISI_PRT("enVsyncValidNeg: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.enVsyncValidNeg);
            }


            pj_json_elem* sensor_vi_dev_attrs_stTimingBlank_object = find_value_by_name(sensor_vi_dev_attrs_stSynCfg_object, "stTimingBlank");
            if (sensor_vi_dev_attrs_stTimingBlank_object == NULL) {
                HISI_PRT("sensor_vi_dev_attrs_stTimingBlank_object not found\n");
            } else {
                pj_json_elem* sensor_u32HsyncHfb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32HsyncHfb");
                if (sensor_u32HsyncHfb == NULL) {
                    HISI_PRT("u32HsyncHfb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncHfb = (int) sensor_u32HsyncHfb->value.num;
//                    json_value_free(sensor_u32HsyncHfb);
                    if (dbg == 1) HISI_PRT("u32HsyncHfb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncHfb);
                }
                pj_json_elem* sensor_u32HsyncHbb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32HsyncHbb");
                if (sensor_u32HsyncHbb == NULL) {
                    HISI_PRT("u32HsyncHbb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncHbb = (int) sensor_u32HsyncHbb->value.num;
//                    json_value_free(sensor_u32HsyncHbb);
                    if (dbg == 1) HISI_PRT("u32HsyncHbb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncHbb);
                }
                pj_json_elem* sensor_u32VsyncVfb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32VsyncVfb");
                if (sensor_u32VsyncVfb == NULL) {
                    HISI_PRT("u32VsyncVfb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVfb = (int) sensor_u32VsyncVfb->value.num;
//                    json_value_free(sensor_u32VsyncVfb);
                    if (dbg == 1) HISI_PRT("u32VsyncVfb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVfb);
                }
                pj_json_elem* sensor_u32VsyncVbb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32VsyncVbb");
                if (sensor_u32VsyncVbb == NULL) {
                    HISI_PRT("u32VsyncVbb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbb = (int) sensor_u32VsyncVbb->value.num;
//                    json_value_free(sensor_u32VsyncVbb);
                    if (dbg == 1) HISI_PRT("u32VsyncVbb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbb);
                }
                pj_json_elem* sensor_u32VsyncVbfb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32VsyncVbfb");
                if (sensor_u32VsyncVbfb == NULL) {
                    HISI_PRT("u32VsyncVbfb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbfb = (int) sensor_u32VsyncVbfb->value.num;
//                    json_value_free(sensor_u32VsyncVbfb);
                    if (dbg == 1) HISI_PRT("u32VsyncVbfb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbfb);
                }
                pj_json_elem* sensor_u32VsyncVbact = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32VsyncVbact");
                if (sensor_u32VsyncVbact == NULL) {
                    HISI_PRT("u32VsyncVbact not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbact = (int) sensor_u32VsyncVbact->value.num;
//                    json_value_free(sensor_u32VsyncVbact);
                    if (dbg == 1) HISI_PRT("u32VsyncVbact: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbact);
                }
                pj_json_elem* sensor_u32VsyncVbbb = find_value_by_name(sensor_vi_dev_attrs_stTimingBlank_object, "u32VsyncVbbb");
                if (sensor_u32VsyncVbbb == NULL) {
                    HISI_PRT("u32VsyncVbbb not found\n");
                } else {
                    app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbbb = (int) sensor_u32VsyncVbbb->value.num;
//                    json_value_free(sensor_u32VsyncVbbb);
                    if (dbg == 1) HISI_PRT("u32VsyncVbbb: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVbbb);
                }
//                json_value_free(sensor_vi_dev_attrs_stTimingBlank_object);

                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncAct        = app->stSize.u32Width;
                app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVact       = app->stSize.u32Height;
                if (dbg == 1) HISI_PRT("u32HsyncAct: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32HsyncAct);
                if (dbg == 1) HISI_PRT("u32VsyncVact: %u\n", app->stViConfig.sensor.vi_dev_attrs.stSynCfg.stTimingBlank.u32VsyncVact);
            }

#if 0
#endif
//            json_value_free(sensor_vi_dev_attrs_stSynCfg_object);
        }

        pj_json_elem* sensor_enDataPath = find_value_by_name(sensor_vi_dev_attrs_object, "enDataPath");
        if (sensor_enDataPath == NULL) {
            HISI_PRT("enDataPath not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enDataPath = (int) sensor_enDataPath->value.num;
//            json_value_free(sensor_enDataPath);
            if (dbg == 1) HISI_PRT("enDataPath: %u\n", app->stViConfig.sensor.vi_dev_attrs.enDataPath);
        }
        pj_json_elem* sensor_enInputDataType = find_value_by_name(sensor_vi_dev_attrs_object, "enInputDataType");
        if (sensor_enInputDataType == NULL) {
            HISI_PRT("enInputDataType not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.enInputDataType = (int) sensor_enInputDataType->value.num;
//            json_value_free(sensor_enInputDataType);
            if (dbg == 1) HISI_PRT("enInputDataType: %u\n", app->stViConfig.sensor.vi_dev_attrs.enInputDataType);
        }
        pj_json_elem* sensor_bDataRevq = find_value_by_name(sensor_vi_dev_attrs_object, "bDataRev");
        if (sensor_bDataRevq == NULL) {
            HISI_PRT("bDataRev not found\n");
        } else {
            app->stViConfig.sensor.vi_dev_attrs.bDataRev = (int) sensor_bDataRevq->value.is_true;
//            json_value_free(sensor_bDataRevq);
            if (dbg == 1) HISI_PRT("bDataRev: %u\n", app->stViConfig.sensor.vi_dev_attrs.bDataRev);
        }
#if 0
#endif
//        json_value_free(sensor_vi_dev_attrs_object);
    }

    pj_json_elem* sensor_stCapRect_object = find_value_by_name(sensor_json_object, "stCapRect");
    if (sensor_stCapRect_object == NULL) {
        HISI_PRT("sensor_stCapRect_object not found\n");
    } else {
        pj_json_elem* sensor_s32X = find_value_by_name(sensor_stCapRect_object, "s32X");
        if (sensor_s32X == NULL) {
            HISI_PRT("s32X not found\n");
        } else {
            app->stViConfig.sensor.stCapRect.s32X = (int) sensor_s32X->value.num;
//            json_value_free(sensor_s32X);
            if (dbg == 1) HISI_PRT("s32X: %u\n", app->stViConfig.sensor.stCapRect.s32X);
        }

        pj_json_elem* sensor_s32Y = find_value_by_name(sensor_stCapRect_object, "s32Y");
        if (sensor_s32Y == NULL) {
            HISI_PRT("s32Y not found\n");
        } else {
            app->stViConfig.sensor.stCapRect.s32Y = (int) sensor_s32Y->value.num;
//            json_value_free(sensor_s32Y);
            if (dbg == 1) HISI_PRT("s32Y: %u\n", app->stViConfig.sensor.stCapRect.s32Y);
        }
        app->stViConfig.sensor.stCapRect.u32Width = app->stSize.u32Width;
        app->stViConfig.sensor.stCapRect.u32Height = app->stSize.u32Height;
        app->stViConfig.sensor.stTargetSize.u32Width = app->stSize.u32Width;
        app->stViConfig.sensor.stTargetSize.u32Height = app->stSize.u32Height;
        if (dbg == 1) HISI_PRT("stCapRect.u32Width: %u\n", app->stViConfig.sensor.stCapRect.u32Width);
        if (dbg == 1) HISI_PRT("stCapRect.u32Height: %u\n", app->stViConfig.sensor.stCapRect.u32Height);
        if (dbg == 1) HISI_PRT("stTargetSize.u32Width: %u\n", app->stViConfig.sensor.stTargetSize.u32Width);
        if (dbg == 1) HISI_PRT("stTargetSize.u32Height: %u\n", app->stViConfig.sensor.stTargetSize.u32Height);
//        json_value_free(sensor_stCapRect_object);
    }

    pj_json_elem* sensor_enRotate = find_value_by_name(value, "enRotate");
    if (sensor_enRotate == NULL) {
        HISI_PRT("enRotate not found\n");
    } else {
        app->stViConfig.enRotate = (int) sensor_enRotate->value.num;
//        json_value_free(sensor_enRotate);
        if (dbg == 1) HISI_PRT("enRotate: %u\n", app->stViConfig.enRotate);
    }
    pj_json_elem* sensor_enNorm = find_value_by_name(value, "enNorm");
    if (sensor_enNorm == NULL) {
        HISI_PRT("enNorm not found\n");
    } else {
        app->stViConfig.enNorm = (int) sensor_enNorm->value.num;
//        json_value_free(sensor_enNorm);
        if (dbg == 1) HISI_PRT("enNorm: %u\n", app->stViConfig.enNorm);
    }
    pj_json_elem* sensor_enViChnSet = find_value_by_name(value, "enViChnSet");
    if (sensor_enViChnSet == NULL) {
        HISI_PRT("enViChnSet not found\n");
    } else {
        app->stViConfig.enViChnSet = (int) sensor_enViChnSet->value.num;
//        json_value_free(sensor_enViChnSet);
        if (dbg == 1) HISI_PRT("enViChnSet: %u\n", app->stViConfig.enViChnSet);
    }
    app->stViConfig.enPixFmt   = HISI_PIXEL_FORMAT;
    if (dbg == 1) HISI_PRT("enPixFmt: %u\n", app->stViConfig.enPixFmt);
//    json_value_free(sensor_vi_dev_attrs_object);

    pj_json_elem* sensor_chl_attrs_object = find_value_by_name(sensor_json_object, "chl_attrs");
    if (sensor_chl_attrs_object == NULL) {
        HISI_PRT("sensor_chl_attrs_object not found\n");
    } else {
        pj_json_elem* sensor_bChromaResample = find_value_by_name(sensor_chl_attrs_object, "bChromaResample");
        if (sensor_bChromaResample == NULL) {
            HISI_PRT("bChromaResample not found\n");
        } else {
            app->stViConfig.sensor.chl_attrs.bChromaResample = (int) sensor_bChromaResample->value.is_true;
//            json_value_free(sensor_bChromaResample);
            if (dbg == 1) HISI_PRT("bChromaResample: %u\n", app->stViConfig.sensor.chl_attrs.bChromaResample);
        }

        pj_json_elem* sensor_enCapSel = find_value_by_name(sensor_chl_attrs_object, "enCapSel");
        if (sensor_enCapSel == NULL) {
            HISI_PRT("enCapSel not found\n");
        } else {
            app->stViConfig.sensor.chl_attrs.enCapSel = (int) sensor_enCapSel->value.num;
//            json_value_free(sensor_enCapSel);
            if (dbg == 1) HISI_PRT("enCapSel: %u\n", app->stViConfig.sensor.chl_attrs.enCapSel);
        }
//        json_value_free(sensor_chl_attrs_object);
    }

    app->stViConfig.sensor.chl_attrs.s32SrcFrameRate = app->stViConfig.sensor.image_attrs.u16FrameRate;
    app->stViConfig.sensor.chl_attrs.s32FrameRate = app->stViConfig.sensor.image_attrs.u16FrameRate;


    switch(app->stViConfig.enViChnSet) {
        case HISI_VI_CHN_SET_MIRROR:
            app->stViConfig.sensor.chl_attrs.bMirror = HI_TRUE;
            break;

        case HISI_VI_CHN_SET_FLIP:
            app->stViConfig.sensor.chl_attrs.bFlip = HI_TRUE;
            break;

        case HISI_VI_CHN_SET_FLIP_MIRROR:
            app->stViConfig.sensor.chl_attrs.bMirror = HI_TRUE;
            app->stViConfig.sensor.chl_attrs.bFlip = HI_TRUE;
            break;

        default:
            break;
    }


    pj_json_elem* sensor_vi_params_object = find_value_by_name(sensor_json_object, "vi_params");
    if (sensor_vi_params_object == NULL) {
        HISI_PRT("sensor_vi_params_object not found\n");
    } else {
        pj_json_elem* sensor_s32ViChnCnt = find_value_by_name(sensor_vi_params_object, "s32ViChnCnt");
        if (sensor_s32ViChnCnt == NULL) {
            HISI_PRT("s32ViChnCnt not found\n");
        } else {
            app->stViConfig.sensor.vi_params.s32ViChnCnt = (int) sensor_s32ViChnCnt->value.num;
//            json_value_free(sensor_s32ViChnCnt);
            if (dbg == 1) HISI_PRT("s32ViChnCnt: %u\n", app->stViConfig.sensor.vi_params.s32ViChnCnt);
        }

        pj_json_elem* sensor_s32ViChnInterval = find_value_by_name(sensor_vi_params_object, "s32ViChnInterval");
        if (sensor_s32ViChnInterval == NULL) {
            HISI_PRT("s32ViChnInterval not found\n");
        } else {
            app->stViConfig.sensor.vi_params.s32ViChnInterval = (int) sensor_s32ViChnInterval->value.num;
//            json_value_free(sensor_s32ViChnInterval);
            if (dbg == 1) HISI_PRT("s32ViChnInterval: %u\n", app->stViConfig.sensor.vi_params.s32ViChnInterval);
        }

        pj_json_elem* sensor_s32ViDevCnt = find_value_by_name(sensor_vi_params_object, "s32ViDevCnt");
        if (sensor_s32ViDevCnt == NULL) {
            HISI_PRT("s32ViDevCnt not found\n");
        } else {
            app->stViConfig.sensor.vi_params.s32ViDevCnt = (int) sensor_s32ViDevCnt->value.num;
//            json_value_free(sensor_s32ViDevCnt);
            if (dbg == 1) HISI_PRT("s32ViDevCnt: %u\n", app->stViConfig.sensor.vi_params.s32ViDevCnt);
        }

        pj_json_elem* sensor_s32ViDevInterval = find_value_by_name(sensor_vi_params_object, "s32ViDevInterval");
        if (sensor_s32ViDevInterval == NULL) {
            HISI_PRT("s32ViDevInterval not found\n");
        } else {
            app->stViConfig.sensor.vi_params.s32ViDevInterval = (int) sensor_s32ViDevInterval->value.num;
//            json_value_free(sensor_s32ViDevInterval);
            if (dbg == 1) HISI_PRT("s32ViDevInterval: %u\n", app->stViConfig.sensor.vi_params.s32ViDevInterval);
        }

//        json_value_free(sensor_vi_params_object);
    }

    app->stVpssGrpAttr.u32MaxW = app->stSize.u32Width;
    app->stVpssGrpAttr.u32MaxH = app->stSize.u32Height;
    app->stVpssGrpAttr.enPixFmt = app->stViConfig.enPixFmt;


    pj_json_elem* stVpssGrpAttr_object = find_value_by_name(value, "stVpssGrpAttr");
    if (stVpssGrpAttr_object == NULL) {
        HISI_PRT("stVpssGrpAttr_object not found\n");
    } else {
        pj_json_elem* bDrEn = find_value_by_name(stVpssGrpAttr_object, "bDrEn");
        if (bDrEn == NULL) {
            HISI_PRT("bDrEn not found\n");
        } else {
            app->stVpssGrpAttr.bDrEn = bDrEn->value.is_true;
//            json_value_free(bDrEn);
            if (dbg == 1) HISI_PRT("bDrEn: %u\n", app->stVpssGrpAttr.bDrEn);
        }

        pj_json_elem* bDbEn = find_value_by_name(stVpssGrpAttr_object, "bDbEn");
        if (bDbEn == NULL) {
            HISI_PRT("bDbEn not found\n");
        } else {
            app->stVpssGrpAttr.bDbEn = bDbEn->value.is_true;
//            json_value_free(bDbEn);
            if (dbg == 1) HISI_PRT("bDbEn: %u\n", app->stVpssGrpAttr.bDbEn);
        }

        pj_json_elem* bIeEn = find_value_by_name(stVpssGrpAttr_object, "bIeEn");
        if (bIeEn == NULL) {
            HISI_PRT("bIeEn not found\n");
        } else {
            app->stVpssGrpAttr.bIeEn = (HI_BOOL) bIeEn->value.is_true;
//            json_value_free(bIeEn);
            if (dbg == 1) HISI_PRT("bIeEn: %d\n", app->stVpssGrpAttr.bIeEn);
        }

        pj_json_elem* bNrEn = find_value_by_name(stVpssGrpAttr_object, "bNrEn");
        if (bNrEn == NULL) {
            HISI_PRT("bNrEn not found\n");
        } else {
            app->stVpssGrpAttr.bNrEn = (HI_BOOL) bNrEn->value.is_true;
//            json_value_free(bNrEn);
            if (dbg == 1) HISI_PRT("bNrEn: %d\n", app->stVpssGrpAttr.bNrEn);
        }

        pj_json_elem* bHistEn = find_value_by_name(stVpssGrpAttr_object, "bHistEn");
        if (bHistEn == NULL) {
            HISI_PRT("bHistEn not found\n");
        } else {
            app->stVpssGrpAttr.bHistEn = (HI_BOOL) bHistEn->value.is_true;
//            json_value_free(bHistEn);
            if (dbg == 1) HISI_PRT("bHistEn: %d\n", app->stVpssGrpAttr.bHistEn);
        }

        pj_json_elem* enDieMode = find_value_by_name(stVpssGrpAttr_object, "enDieMode");
        if (enDieMode == NULL) {
            HISI_PRT("enDieMode not found\n");
        } else {
            app->stVpssGrpAttr.enDieMode = (int) enDieMode->value.num;
//            json_value_free(enDieMode);
            if (dbg == 1) HISI_PRT("enDieMode: %u\n", app->stVpssGrpAttr.enDieMode);
        }
//        json_value_free(stVpssGrpAttr_object);
    }

    pj_json_elem* stVpssChnMode_object = find_value_by_name(value, "stVpssChnMode");
    if (stVpssChnMode_object == NULL) {
        HISI_PRT("stVpssChnMode_object not found\n");
    } else {
        pj_json_elem* enChnMode = find_value_by_name(stVpssChnMode_object, "enChnMode");
        if (enChnMode == NULL) {
            HISI_PRT("enChnMode not found\n");
        } else {
            app->stVpssChnMode.enChnMode = (int) enChnMode->value.num;
//            json_value_free(enChnMode);
            if (dbg == 1) HISI_PRT("enChnMode: %u\n", app->stVpssChnMode.enChnMode);
        }

        pj_json_elem* bDouble = find_value_by_name(stVpssChnMode_object, "bDouble");
        if (bDouble == NULL) {
            HISI_PRT("bDouble not found\n");
        } else {
            app->stVpssChnMode.bDouble = bDouble->value.is_true;
//            json_value_free(bDouble);
            if (dbg == 1) HISI_PRT("bDouble: %u\n", app->stVpssChnMode.bDouble);
        }

//        json_value_free(stVpssGrpAttr_object);
    }

    app->stVpssChnMode.enPixelFormat = HISI_PIXEL_FORMAT;
    app->stVpssChnMode.u32Width      = app->stSize.u32Width;
    app->stVpssChnMode.u32Height     = app->stSize.u32Height;

    pj_json_elem* stVpssChnAttr_object = find_value_by_name(value, "stVpssChnAttr");
    if (stVpssChnAttr_object == NULL) {
        HISI_PRT("stVpssChnAttr_object not found\n");
    } else {
        pj_json_elem* bFrameEn = find_value_by_name(stVpssChnAttr_object, "bFrameEn");
        if (bFrameEn == NULL) {
            HISI_PRT("bFrameEn not found\n");
        } else {
            app->stVpssChnAttr.bFrameEn = (int) bFrameEn->value.num;
//            json_value_free(bFrameEn);
            if (dbg == 1) HISI_PRT("bFrameEn: %u\n", app->stVpssChnAttr.bFrameEn);
        }

        pj_json_elem* bSpEn = find_value_by_name(stVpssChnAttr_object, "bSpEn");
        if (bSpEn == NULL) {
            HISI_PRT("bSpEn not found\n");
        } else {
            app->stVpssChnAttr.bSpEn = bSpEn->value.is_true;
//            json_value_free(bSpEn);
            if (dbg == 1) HISI_PRT("bSpEn: %u\n", app->stVpssChnAttr.bSpEn);
        }

//        json_value_free(stVpssChnAttr_object);
    }

    pj_json_elem* stVpssExtChnAttr_object = find_value_by_name(value, "stVpssExtChnAttr");
    if (stVpssExtChnAttr_object == NULL) {
        HISI_PRT("stVpssExtChnAttr_object not found\n");
    } else {
        pj_json_elem* s32BindChn = find_value_by_name(stVpssExtChnAttr_object, "s32BindChn");
        if (s32BindChn == NULL) {
            HISI_PRT("s32BindChn not found\n");
        } else {
            app->stVpssExtChnAttr.s32BindChn = (int) s32BindChn->value.num;
//            json_value_free(s32BindChn);
            if (dbg == 1) HISI_PRT("s32BindChn: %u\n", app->stVpssExtChnAttr.s32BindChn);
        }

//        json_value_free(stVpssExtChnAttr_object);
    }

//    app->stVpssExtChnAttr.s32BindChn = 1;
    app->stVpssExtChnAttr.s32SrcFrameRate = app->stViConfig.sensor.image_attrs.u16FrameRate;
    app->stVpssExtChnAttr.s32DstFrameRate = app->stViConfig.sensor.image_attrs.u16FrameRate;
    app->stVpssExtChnAttr.enPixelFormat   = HISI_PIXEL_FORMAT;
    app->stVpssExtChnAttr.u32Width        = app->stSize.u32Width;
    app->stVpssExtChnAttr.u32Height       = app->stSize.u32Height;

    return app;

#if 0

#endif
}

//int start_sdk(HISI_SDK_APP_S *app) {
//    pj_status_t status;
//    HI_S32 s32Ret = HI_SUCCESS;
//    VPSS_GRP VpssGrp;
//    VPSS_CHN VpssChn;
//
//    VENC_GRP VencGrp;
//    VENC_CHN VencChn;
////
//    printf("\033[0;33m\napp->u32BlkSize:           \033[0m     \033[0;31m%u\033[0m\n", app->u32BlkSize);
////    HI_S32 s32ChnNum = 1;
////    
////    /******************************************
////     step 2: mpp system init.
////    ******************************************/
//    s32Ret = HISI_COMM_SYS_Init(&app->stVbConf);
////    status = HISI_COMM_SYS_Init(&app->stVbConf);
////    if (status != PJ_SUCCESS)
////	return status;
//    
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("system init failed with %d!\n", status);
//        app->init_stage_0 = HI_TRUE;
////        stop_sdk(app);
////        return EXIT_FAILURE;
//        return status;
////        goto END_VENC_720P_CLASSIC_0;
//    }
//    
//    /******************************************
//     step 3: start vi dev & chn to capture
//    ******************************************/
//    status = HISI_COMM_VI_StartVi(&app->stViConfig);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("start vi failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_1;
//    }
//    
//    /******************************************
//     step 4: start vpss and vi bind vpss
//    ******************************************/
//
//
////    VpssGrp = 0;
////
//    status = HISI_COMM_VPSS_StartGroup(0, &app->stVpssGrpAttr);
////    status = HISI_COMM_VPSS_StartGroup(VpssGrp, &app->stVpssGrpAttr);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Start Vpss failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_2;
//    }
//    
//    status = HISI_COMM_VI_BindVpss(app->stViConfig.sensor.vi_params);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Vi bind Vpss failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_3;
//    }
//    
////    VpssChn = 0;
//
//    status = HISI_COMM_VPSS_EnableChn(0, 0, 
////    status = HISI_COMM_VPSS_EnableChn(VpssGrp, VpssChn, 
//            &app->stVpssChnAttr, 
//            &app->stVpssChnMode, 
//            &app->stVpssExtChnAttr);
////    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, HI_NULL, HI_NULL);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Enable vpss chn failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
//        app->init_stage_4 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_4;
//    }
//    
//        /******************************************
//     step 5: start stream venc
//    ******************************************/
//    /*** HD720P **/ // <===================================================
//    VpssGrp = 0;
//    VpssChn = 0;
//    VencGrp = 0;
//    VencChn = 0;
//    status = HISI_COMM_VENC_Start(VencGrp, VencChn, 
//            app->enPayLoad,
//            app->gs_enNorm, 
//            app->enSize, 
//            app->enRcMode);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Start Venc failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
//        app->init_stage_4 = HI_TRUE;
//        app->init_stage_5 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_5;
//    }
//
//    status = HISI_COMM_VENC_BindVpss(VencGrp, VpssGrp, VpssChn);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Start Venc failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
//        app->init_stage_4 = HI_TRUE;
//        app->init_stage_5 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_5;
//    }
//
//    /******************************************
//     step 6: stream venc process -- get stream, then save it to file.
//    ******************************************/
//    HI_S32 s32ChnNum = 1;
//    status = HISI_COMM_VENC_StartGetStream(s32ChnNum);
////    status = HISI_COMM_VENC_StartGetStream(s32ChnNum);
//    if (status != PJ_SUCCESS) {
//        HISI_PRT("Start Venc failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
//        app->init_stage_4 = HI_TRUE;
//        app->init_stage_5 = HI_TRUE;
////        stop_sdk(app);
//        return status;
////        goto END_VENC_720P_CLASSIC_5;
//    }
//}
    
int stop_sdk(HISI_SDK_APP_S *app) {
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;

    if (app->init_stage_5 == HI_TRUE) {
        VpssGrp = 0;
        VpssChn = 0;
        VencGrp = 0;
        VencChn = 0;

        HISI_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        HISI_COMM_VENC_Stop(VencGrp,VencChn);

        HISI_COMM_VI_UnBindVpss(app->stViConfig.sensor.vi_params);
    }
    if (app->init_stage_4 == HI_TRUE) {
        VpssGrp = 0;
//        VpssChn = 3;
//        SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
        VpssChn = 0;
        HISI_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
//        VpssChn = 1;
//        SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    }
    if (app->init_stage_3 == HI_TRUE) {
        HISI_COMM_VI_UnBindVpss(app->stViConfig.sensor.vi_params);
    }
    if (app->init_stage_2 == HI_TRUE) {
        HISI_COMM_VPSS_StopGroup(VpssGrp);
    }
    if (app->init_stage_1 == HI_TRUE) {
        HISI_COMM_VI_StopVi(&app->stViConfig);
    }
    if (app->init_stage_0 == HI_TRUE) {
        HISI_COMM_SYS_Exit();
    }
}

int start_sdk(HISI_SDK_APP_S *app) {
//    printf("\033[0;33mApp build with headers MPP version:\033[0m \033[0;31m%s\033[0m\n", MPP_VERSION);
//    MPP_VERSION_S version;
//    HI_MPI_SYS_GetVersion(&version);
//    printf("\033[0;33mCurrent MPP version:           \033[0m     \033[0;31m%s\033[0m\n", version.aVersion);

    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;

    VENC_GRP VencGrp;
    VENC_CHN VencChn;

    printf("\033[0;33m\napp->u32BlkSize:           \033[0m     \033[0;31m%u\033[0m\n", app->u32BlkSize);
    HI_S32 s32ChnNum = 1;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = HISI_COMM_SYS_Init(&app->stVbConf);

//    VB_CONF_S stVbConf;
//    memset(&stVbConf,0,sizeof(VB_CONF_S));
//    stVbConf.u32MaxPoolCnt = 128;
//    stVbConf.astCommPool[0].u32BlkSize = 1474560;
//    stVbConf.astCommPool[0].u32BlkCnt = 10;

//    s32Ret = HISI_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("system init failed with %d!\n", s32Ret);
        app->init_stage_0 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = HISI_COMM_VI_StartVi(&app->stViConfig);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("start vi failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_1;
    }
//#if 0
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/


    VpssGrp = 0;

    s32Ret = HISI_COMM_VPSS_StartGroup(VpssGrp, &app->stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Start Vpss failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        app->init_stage_2 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_2;
    }

    s32Ret = HISI_COMM_VI_BindVpss(app->stViConfig.sensor.vi_params);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Vi bind Vpss failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        app->init_stage_2 = HI_TRUE;
        app->init_stage_3 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_3;
    }

    VpssChn = 0;

    s32Ret = HISI_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &app->stVpssChnAttr, &app->stVpssChnMode, &app->stVpssExtChnAttr);
//    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, HI_NULL, HI_NULL);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Enable vpss chn failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        app->init_stage_2 = HI_TRUE;
        app->init_stage_3 = HI_TRUE;
        app->init_stage_4 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD720P **/ // <===================================================
    VpssGrp = 0;
    VpssChn = 0;
    VencGrp = 0;
    VencChn = 0;
    s32Ret = HISI_COMM_VENC_Start(VencGrp, VencChn, app->enPayLoad,\
                                   app->gs_enNorm, app->enSize, app->enRcMode);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Start Venc failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        app->init_stage_2 = HI_TRUE;
        app->init_stage_3 = HI_TRUE;
        app->init_stage_4 = HI_TRUE;
        app->init_stage_5 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_5;
    }

    s32Ret = HISI_COMM_VENC_BindVpss(VencGrp, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Start Venc failed!\n");
        app->init_stage_0 = HI_TRUE;
        app->init_stage_1 = HI_TRUE;
        app->init_stage_2 = HI_TRUE;
        app->init_stage_3 = HI_TRUE;
        app->init_stage_4 = HI_TRUE;
        app->init_stage_5 = HI_TRUE;
        stop_sdk(app);
        return EXIT_FAILURE;
//        goto END_VENC_720P_CLASSIC_5;
    }

//    /******************************************
//     step 6: stream venc process -- get stream, then save it to file.
//    ******************************************/
//    s32Ret = HISI_COMM_VENC_StartGetStream(s32ChnNum);
//    if (HI_SUCCESS != s32Ret) {
//        HISI_PRT("Start Venc failed!\n");
//        app->init_stage_0 = HI_TRUE;
//        app->init_stage_1 = HI_TRUE;
//        app->init_stage_2 = HI_TRUE;
//        app->init_stage_3 = HI_TRUE;
//        app->init_stage_4 = HI_TRUE;
//        app->init_stage_5 = HI_TRUE;
//        stop_sdk(app);
//        return EXIT_FAILURE;
////        goto END_VENC_720P_CLASSIC_5;
//    }

//    printf("please press twice ENTER to exit this sample\n");
//    getchar();
//    getchar();
//
//    /******************************************
//     step 7: exit process
//    ******************************************/
//    HISI_COMM_VENC_StopGetStream();
////#endif
//    app->init_stage_0 = HI_TRUE;
//    app->init_stage_1 = HI_TRUE;
//    app->init_stage_2 = HI_TRUE;
//    app->init_stage_3 = HI_TRUE;
//    app->init_stage_4 = HI_TRUE;
//    app->init_stage_5 = HI_TRUE;
//    stop_sdk(app);
    return 0;
}
    
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif