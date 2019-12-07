//#include <pjmedia-codec/h264_passthrough_vid_codecs.h>
#include <pjmedia-codec/h263_packetizer.h>
#include <pjmedia-codec/h264_packetizer.h>
#include <pjmedia/errno.h>
#include <pjmedia/vid_codec_util.h>
#include <pj/assert.h>
#include <pj/list.h>
#include <pj/log.h>
#include <pj/math.h>
#include <pj/pool.h>
#include <pj/string.h>
#include <pj/os.h>

#include <pjmedia-codec/types.h>


#define DEFAULT_WIDTH     640
#define DEFAULT_HEIGHT    480
#define DEFAULT_FPS     25
#define DEFAULT_AVG_BITRATE 2097152
#define DEFAULT_MAX_BITRATE 2097152
// #define DEFAULT_AVG_BITRATE 256000
// #define DEFAULT_MAX_BITRATE 256000



/* Prototypes for H264 codecs factory */
static pj_status_t h264_passthrough_test_alloc( pjmedia_vid_codec_factory *factory,
                      const pjmedia_vid_codec_info *info );
static pj_status_t h264_passthrough_default_attr( pjmedia_vid_codec_factory *factory,
                        const pjmedia_vid_codec_info *info,
                        pjmedia_vid_codec_param *attr );
static pj_status_t h264_passthrough_enum_info(pjmedia_vid_codec_factory *factory,
                                   unsigned *count,
                                   pjmedia_vid_codec_info info[]);
static pj_status_t h264_passthrough_alloc_codec( pjmedia_vid_codec_factory *factory,
                       const pjmedia_vid_codec_info *info,
                       pjmedia_vid_codec **p_codec);
static pj_status_t h264_passthrough_dealloc_codec( pjmedia_vid_codec_factory *factory,
                         pjmedia_vid_codec *codec );

/* Prototypes for H264 codecs implementation. */
static pj_status_t  h264_passthrough_codec_init( pjmedia_vid_codec *codec,
                       pj_pool_t *pool );
static pj_status_t  h264_passthrough_codec_open( pjmedia_vid_codec *codec,
                       pjmedia_vid_codec_param *codec_param );
static pj_status_t  h264_passthrough_codec_close( pjmedia_vid_codec *codec );
static pj_status_t  h264_passthrough_codec_modify(pjmedia_vid_codec *codec,
                        const pjmedia_vid_codec_param *param );
static pj_status_t  h264_passthrough_codec_get_param(pjmedia_vid_codec *codec,
                       pjmedia_vid_codec_param *param);
static pj_status_t h264_passthrough_codec_encode_begin(pjmedia_vid_codec *codec,
                         const pjmedia_vid_encode_opt *opt,
                                             const pjmedia_frame *input,
                         unsigned out_size,
                         pjmedia_frame *output,
                         pj_bool_t *has_more);
static pj_status_t h264_passthrough_codec_encode_more(pjmedia_vid_codec *codec,
                        unsigned out_size,
                        pjmedia_frame *output,
                        pj_bool_t *has_more);
static pj_status_t h264_passthrough_codec_decode( pjmedia_vid_codec *codec,
                    pj_size_t pkt_count,
                    pjmedia_frame packets[],
                    unsigned out_size,
                    pjmedia_frame *output);

/*
 * Only build this file if PJMEDIA_HAS_H264_PASSTHROUGH_VID_CODEC != 0 and
 * PJMEDIA_HAS_VIDEO != 0
 */

#if PJMEDIA_HAS_VIDEO && PJMEDIA_HAS_H264_HISILICON_PASSTHROUGH_VID_CODEC

/* Definition for H264 codecs operations. */
static pjmedia_vid_codec_op h264_passthrough_op =
{
    &h264_passthrough_codec_init,
    &h264_passthrough_codec_open,
    &h264_passthrough_codec_close,
    &h264_passthrough_codec_modify,
    &h264_passthrough_codec_get_param,
    // NULL,
    &h264_passthrough_codec_encode_begin,
    // NULL,
    &h264_passthrough_codec_encode_more,
    // NULL,
    &h264_passthrough_codec_decode,
    NULL
};

/* Definition for H264 codecs factory operations. */
static pjmedia_vid_codec_factory_op h264_passthrough_factory_op =
{
    &h264_passthrough_test_alloc,
    &h264_passthrough_default_attr,
    &h264_passthrough_enum_info,
    &h264_passthrough_alloc_codec,
    &h264_passthrough_dealloc_codec
};

/* h264 Passthrough codecs factory */
static struct h264_passthrough_factory {
    pjmedia_vid_codec_factory    base;
    pjmedia_vid_codec_mgr       *mgr;
    pj_pool_factory             *pf;
    pj_pool_t                   *pool;
} h264_passthrough_factory;

typedef struct h264_passthrough_codec_data
{
    pjmedia_vid_codec       *codec;
    pj_pool_t           *pool;
} h264_passthrough_codec_data;

#define THIS_FILE   "h264_hisilicon_passthrough_vid_codecs.c"

/*
 * Initialize and register h264 Passthrough codec factory to pjmedia endpoint.
 */
PJ_DEF(pj_status_t) pjmedia_codec_h264_hisilicon_passthrough_vid_init(pjmedia_vid_codec_mgr *mgr, pj_pool_factory *pf)
{

    PJ_LOG(3, (THIS_FILE, "pjmedia_codec_h264_passthrough_vid_init"));

    const pj_str_t h264_name = { (char*)"H264", 4};
    pj_status_t status;

    if (h264_passthrough_factory.pool != NULL) {
        /* Already initialized. */
        PJ_LOG(3, (THIS_FILE, "Already initialized."));
        return PJ_SUCCESS;
    }

    if (!mgr) mgr = pjmedia_vid_codec_mgr_instance();
    PJ_ASSERT_RETURN(mgr, PJ_EINVAL);

    /* Create OpenH264 codec factory. */
    h264_passthrough_factory.base.op = &h264_passthrough_factory_op;
    h264_passthrough_factory.base.factory_data = NULL;
    h264_passthrough_factory.mgr = mgr;
    h264_passthrough_factory.pf = pf;
    h264_passthrough_factory.pool = pj_pool_create(pf, "h264_passthrough_factory", 256, 256, NULL);
    if (!h264_passthrough_factory.pool)
        return PJ_ENOMEM;
    /* Registering format match for SDP negotiation */
    status = pjmedia_sdp_neg_register_fmt_match_cb(
                    &h264_name,
                    &pjmedia_vid_codec_h264_match_sdp);
    if (status != PJ_SUCCESS)
        goto on_error;
    /* Register codec factory to codec manager. */
    status = pjmedia_vid_codec_mgr_register_factory(mgr, &h264_passthrough_factory.base);
    if (status != PJ_SUCCESS)
        goto on_error;

    PJ_LOG(4,(THIS_FILE, "H264 Passthrough codec initialized"));

    /* Done. */
    return PJ_SUCCESS;

on_error:
    pj_pool_release(h264_passthrough_factory.pool);
    h264_passthrough_factory.pool = NULL;
    return status;
}

/*
 * Unregister h264 Passthrough codecs factory from pjmedia endpoint.
 */
PJ_DEF(pj_status_t) pjmedia_codec_h264_hisilicon_passthrough_vid_deinit(void)
{
    pj_status_t status = PJ_SUCCESS;

    if (h264_passthrough_factory.pool == NULL) {
    /* Already deinitialized */
        return PJ_SUCCESS;
    }

    /* Unregister OpenH264 codecs factory. */
    status = pjmedia_vid_codec_mgr_unregister_factory(h264_passthrough_factory.mgr,
                              &h264_passthrough_factory.base);
    
    /* Destroy pool. */
    pj_pool_release(h264_passthrough_factory.pool);
    if (h264_passthrough_factory.pool != NULL) {
        h264_passthrough_factory.pool = NULL;
    }
    return status;
}

/*
 * Check if factory can allocate the specified codec.
 */
static pj_status_t h264_passthrough_test_alloc( pjmedia_vid_codec_factory *factory, const pjmedia_vid_codec_info *info ) {

    PJ_ASSERT_RETURN(factory == &h264_passthrough_factory.base, PJ_EINVAL);

    if (info->fmt_id == PJMEDIA_FORMAT_H264 && info->pt != 0) {
        return PJ_SUCCESS;
    }
    
    PJ_LOG(4, (THIS_FILE, "CODEC NOT SUPPORTED"));
    return PJMEDIA_CODEC_EUNSUP;
}

/*
 * Enum codecs supported by this factory.
 */
static pj_status_t h264_passthrough_enum_info(pjmedia_vid_codec_factory *factory,
                                   unsigned *count,
                                   pjmedia_vid_codec_info info[])
{
    PJ_ASSERT_RETURN(info && *count > 0, PJ_EINVAL);
    PJ_ASSERT_RETURN(factory == &h264_passthrough_factory.base, PJ_EINVAL);

    *count = 1;
    info->fmt_id = PJMEDIA_FORMAT_H264;
    info->pt = PJMEDIA_RTP_PT_H264;
    info->encoding_name = pj_str((char*)"H264");
    info->encoding_desc = pj_str((char*)"Hisilicon H264 codec");
    info->clock_rate = 90000;
    info->dir = PJMEDIA_DIR_ENCODING;
//    info->dir = PJMEDIA_DIR_ENCODING_DECODING;
    info->dec_fmt_id_cnt = 1;
    info->dec_fmt_id[0] = PJMEDIA_FORMAT_I420;
    info->packings = PJMEDIA_VID_PACKING_PACKETS;
    // info->packings = PJMEDIA_VID_PACKING_WHOLE;
//    info->packings = PJMEDIA_VID_PACKING_WHOLE;
//    info->packings = PJMEDIA_VID_PACKING_PACKETS | PJMEDIA_VID_PACKING_WHOLE;

    info->fps_cnt = 1;
    info->fps[0].num = 25;
    info->fps[0].denum = 1;

    return PJ_SUCCESS;

}

/*
 * Generate default attribute.
 */
static pj_status_t h264_passthrough_default_attr( pjmedia_vid_codec_factory *factory,
    const pjmedia_vid_codec_info *info, pjmedia_vid_codec_param *attr ) {

    PJ_ASSERT_RETURN(factory == &h264_passthrough_factory.base, PJ_EINVAL);
    PJ_ASSERT_RETURN(info && attr, PJ_EINVAL);

    pj_bzero(attr, sizeof(pjmedia_vid_codec_param));

    attr->dir = PJMEDIA_DIR_CAPTURE;
    attr->packing = PJMEDIA_VID_PACKING_PACKETS;

    /* Encoded format */
    pjmedia_format_init_video(&attr->enc_fmt, PJMEDIA_FORMAT_H264,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_FPS, 1);

    /* Decoded format */
    pjmedia_format_init_video(&attr->dec_fmt, PJMEDIA_FORMAT_I420,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_FPS, 1);

    /* Encoding fmtp */
    attr->enc_fmtp.cnt = 2;
    attr->enc_fmtp.param[0].name = pj_str((char*)"profile-level-id");
    attr->enc_fmtp.param[0].val = pj_str((char*)"4d0028");
//    attr->enc_fmtp.param[0].val = pj_str((char*)"4d0014");
//    attr->enc_fmtp.param[0].val = pj_str((char*)"42001e");
    attr->enc_fmtp.param[1].name = pj_str((char*)" packetization-mode");
    attr->enc_fmtp.param[1].val = pj_str((char*)"1");

    /* Bitrate */
    attr->enc_fmt.det.vid.avg_bps = DEFAULT_AVG_BITRATE;
    attr->enc_fmt.det.vid.max_bps = DEFAULT_MAX_BITRATE;

    /* Encoding MTU */
    attr->enc_mtu = PJMEDIA_MAX_VID_PAYLOAD_SIZE;

    // attr->ignore_fmtp = PJ_FALSE;
    attr->ignore_fmtp = PJ_TRUE;

    return PJ_SUCCESS;
}

static pj_status_t h264_passthrough_alloc_codec(pjmedia_vid_codec_factory *factory,
                                     const pjmedia_vid_codec_info *info,
                                     pjmedia_vid_codec **p_codec)
{
    pj_pool_t *pool;
    pjmedia_vid_codec *codec;
    h264_passthrough_codec_data *h264_passthrough_data;
    int rc;

    PJ_ASSERT_RETURN(factory == &h264_passthrough_factory.base && info && p_codec, PJ_EINVAL);

    *p_codec = NULL;

    pool = pj_pool_create(h264_passthrough_factory.pf, "hisih264%p", 512, 512, NULL);
    if (!pool)
        return PJ_ENOMEM;

    /* codec instance */
    codec = PJ_POOL_ZALLOC_T(pool, pjmedia_vid_codec);
    codec->factory = factory;
    codec->op = &h264_passthrough_op;

    /* codec data */
    h264_passthrough_data = PJ_POOL_ZALLOC_T(pool, h264_passthrough_codec_data);
    h264_passthrough_data->pool = pool;
    h264_passthrough_data->codec = codec;
    codec->codec_data = h264_passthrough_data;

    *p_codec = codec;
    
    return PJ_SUCCESS;

on_error:
    h264_passthrough_dealloc_codec(factory, codec);
    return PJMEDIA_CODEC_EFAILED;
}

/*
 * Free codec.
 */
static pj_status_t h264_passthrough_dealloc_codec(pjmedia_vid_codec_factory *factory,
                                       pjmedia_vid_codec *codec )
{
    h264_passthrough_codec_data *h264_passthrough_data;

    PJ_ASSERT_RETURN(codec, PJ_EINVAL);

    PJ_UNUSED_ARG(factory);

    h264_passthrough_data = (h264_passthrough_codec_data*) codec->codec_data;
    pj_pool_release(h264_passthrough_data->pool);
    return PJ_SUCCESS;
}

/*
 * Init codec.
 */
static pj_status_t h264_passthrough_codec_init(pjmedia_vid_codec *codec,
                                    pj_pool_t *pool )
{
    PJ_ASSERT_RETURN(codec && pool, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    PJ_UNUSED_ARG(pool);
    return PJ_SUCCESS;
}

/*
 * Open codec.
 */
static pj_status_t h264_passthrough_codec_open(pjmedia_vid_codec *codec,
                                    pjmedia_vid_codec_param *codec_param )
{
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

static pj_status_t h264_passthrough_codec_encode_more(pjmedia_vid_codec *codec,
                                           unsigned out_size,
                                           pjmedia_frame *output,
                                           pj_bool_t *has_more)
{
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

/*
 * Modify codec settings.
 */
static pj_status_t h264_passthrough_codec_modify(pjmedia_vid_codec *codec,
                                      const pjmedia_vid_codec_param *param)
{
    PJ_ASSERT_RETURN(codec && param, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    PJ_UNUSED_ARG(param);
    return PJ_EINVALIDOP;
}

static pj_status_t h264_passthrough_codec_get_param(pjmedia_vid_codec *codec,
                                         pjmedia_vid_codec_param *param)
{
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

static pj_status_t h264_passthrough_codec_encode_begin(pjmedia_vid_codec *codec,
                                            const pjmedia_vid_encode_opt *opt,
                                            const pjmedia_frame *input,
                                            unsigned out_size,
                                            pjmedia_frame *output,
                                            pj_bool_t *has_more)
{
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}


/*
 * Decode frame.
 */
static pj_status_t h264_passthrough_codec_decode( pjmedia_vid_codec *codec,
                    pj_size_t pkt_count,
                    pjmedia_frame packets[],
                    unsigned out_size,
                    pjmedia_frame *output)
{
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

/*
 * Close codec.
 */
static pj_status_t h264_passthrough_codec_close( pjmedia_vid_codec *codec )
{
    PJ_ASSERT_RETURN(codec, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}


#endif  /* PJMEDIA_HAS_H264_PASSTHROUGH_VID_CODEC */