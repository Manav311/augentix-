#include "sample_ctrl.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "mpi_enc.h"

#include "sample_stream.h"
#include "sample_utils.h"
#include "sample_osd.h"

#define SAMPLE_VIDEO_CHN_IDX_EQUAL(a, b) (((a).dev == (b).dev) && (a).chn == (b).chn)

static int disableOsd(const SAMPLE_CONF_S *conf)
{
	int err = 0;
	/** Stop all OSD */
	SAMPLE_destroyOsdUpdateThread();

	for (int i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
		if (!conf->dev[0].chn[i].enable) {
			continue;
		}

		err = SAMPLE_stopOsd(MPI_VIDEO_CHN(0, i));
		assert(err == 0);
	}

	/**
	 * native: free font, logo ptr
	 * libosd: OSD_destroy
	 */
	SAMPLE_freeOsdResources();

	return err;
}

static int enableOsd(const SAMPLE_CONF_S *conf)
{
	int err = 0;
	/** Add OSD back to video channels */
	UINT32 output_num = 0;
	UINT16 width = 0;
	UINT16 height = 0;
	MPI_ENC_CHN_ATTR_S enc_attr;

	/**
	 * native: load font, logo resource
	 * libosd: OSD_init
	 */
	SAMPLE_initOsd();

	for (int i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
		if (conf->dev[0].chn[i].enable) {
			output_num++;
		}
	}

	for (int i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
		if (!conf->dev[0].chn[i].enable) {
			continue;
		}
		/** chn resolution maybe changed, not same to case_config */
		err = MPI_ENC_getChnAttr(MPI_ENC_CHN(i), &enc_attr);
		assert(err == 0);

		width = enc_attr.res.width;
		width = enc_attr.res.height;

		err = SAMPLE_createOsd(conf->osd_visible, MPI_VIDEO_CHN(0, i), output_num, width, height);
		assert(err == 0);
	}

	SAMPLE_createOsdUpdateThread();

	return err;
}

/**
 * @brief Reconfig video resolution when video pipeline is running.
 * @details
 * The sample function is expected to work under the following context:
 *  - Video device, video channels and encoders are running.
 *    Otherwise, stop flow and restart flow must be adjusted.
 *  - Object detection is disabled before reconfiguring resolution.
 * @param[in,out] conf    Pointer to video pipeline configuration.
 * @param[in]     idx     Index of the target video channel
 * @param[in]     res     Pointer to target output resolution
 * @return The execution result
 * @retval 0      success
 * @exception SIGABRT if any MPI returns not success.
 * @exception SIGFAULT if NULL pointer is received.
 */
INT32 SAMPLE_reconfigResolution(const SAMPLE_CONF_S *conf, MPI_CHN idx, const MPI_SIZE_S *resolution)
{
	MPI_ENC_CHN_ATTR_S enc_attr[MPI_MAX_ENC_CHN_NUM];
	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout;
	int err;
	int i;

	/**
	 * To reconfigure resolution, update fields in the following attributes are needed:
	 *  - MPI_CHN_ATTR_S
	 *  - MPI_CHN_LAYOUT_S
	 *  - MPI_ENC_CHN_ATTR_S.
	 *
	 * Making sample code as simple as possible, we configure layout without PIP or POP
	 * features, just keep only 1 window. Please modify layout attribute as you want.
	 */

	err = MPI_DEV_getChnAttr(idx, &chn_attr);
	assert(err == 0);

	err = MPI_DEV_getChnLayout(idx, &layout);
	assert(err == 0);

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_getChnAttr(MPI_ENC_CHN(i), &enc_attr[i]);
		assert(err == 0);
	}

	chn_attr.res = *resolution;

	layout.window_num = 1;
	layout.window[0] = (MPI_RECT_S){ .x = 0, .y = 0, .width = resolution->width, .height = resolution->height };

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		// Configure encoders that are bound to target video channel.
		if (SAMPLE_VIDEO_CHN_IDX_EQUAL(conf->enc_chn[i].bind.idx, idx)) {
			enc_attr[i].res = *resolution;
			enc_attr[i].max_res = *resolution;
		}
	}

	/** First, stop encoders and video channels before configuring resolution and layout. */

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_stopChn(MPI_ENC_CHN(i));
		assert(err == 0);

		err = MPI_ENC_unbindFromVideoChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	err = MPI_DEV_stopAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	/** Stop all OSD */
	if (conf->osd_visible) {
		disableOsd(conf);
	}

	/** Then, call related setting MPIs to configure resolution */

	err = MPI_DEV_setChnAttr(idx, &chn_attr);
	assert(err == 0);

	err = MPI_DEV_setChnLayout(idx, &layout);
	assert(err == 0);

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		// Configure encoders that are bound to target video channel.
		if (SAMPLE_VIDEO_CHN_IDX_EQUAL(conf->enc_chn[i].bind.idx, idx)) {
			err = MPI_ENC_setChnAttr(MPI_ENC_CHN(i), &enc_attr[i]);
			assert(err == 0);
		}
	}

	/** Add OSD back to video channels */
	if (conf->osd_visible) {
		enableOsd(conf);
	}

	/** Finally, restart video channels and encoders to generate frames again. */

	err = MPI_DEV_startAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_bindToVideoChn(MPI_ENC_CHN(i), &conf->enc_chn[i].bind);
		assert(err == 0);

		err = MPI_ENC_startChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	return 0;
}

/**
 * @brief Reconfig video layout when video pipeline is running.
 * @details
 * The sample function is expected to work under the following context:
 *  - Video device, video channels and encoders are running
 *    Otherwise, stop flow and restart flow must be adjusted
 *  - Object detection is disabled before reconfiguring layout.
 * @param[in,out] conf      Pointer to video pipeline configuration.
 * @param[in]     idx       Index of the target video channel
 * @param[in]     layout    Pointer to target video layout
 * @return The execution result
 * @retval 0      success
 * @exception SIGABRT if any MPI returns not success.
 */
INT32 SAMPLE_reconfigLayout(const SAMPLE_CONF_S *conf, MPI_CHN idx, const MPI_CHN_LAYOUT_S *layout)
{
	int err;
	int i;

	/** First, stop encoders and video channels before configuring layout. */

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_stopChn(MPI_ENC_CHN(i));
		assert(err == 0);

		err = MPI_ENC_unbindFromVideoChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	err = MPI_DEV_stopAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	/** Stop all OSD */
	if (conf->osd_visible) {
		disableOsd(conf);
	}

	/** Then, call related setting MPIs to configure video layout */

	err = MPI_DEV_setChnLayout(idx, layout);
	assert(err == 0);

	/** Add OSD back to video channels */
	if (conf->osd_visible) {
		enableOsd(conf);
	}

	/** Finally, restart video channels and encoders to generate frames again. */

	err = MPI_DEV_startAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_bindToVideoChn(MPI_ENC_CHN(i), &conf->enc_chn[i].bind);
		assert(err == 0);

		err = MPI_ENC_startChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	return 0;
}

/**
 * @brief Update channel FPS.
 * @param[in] idx    Index of the target video channel
 * @param[in] res    Pointer to target output fps
 * @return The execution result
 * @retval 0      success
 * @retval others unexpected failure
 */
INT32 SAMPLE_updateFps(MPI_CHN idx, FLOAT fps)
{
	MPI_CHN_ATTR_S attr;
	int err;

	/** User only needs to update .fps in MPI_CHN_ATTR_S to update FPS */

	err = MPI_DEV_getChnAttr(idx, &attr);
	if (err != 0) {
		return err;
	}

	attr.fps = fps;

	err = MPI_DEV_setChnAttr(idx, &attr);
	if (err != 0) {
		return err;
	}

	/** Also update venc sps fps info */
	MPI_ECHN enc_idx;
	enc_idx.chn = idx.chn;
	err = SAMPLE_updateSpsVuiFpsInfo(enc_idx, (UINT32)fps);
	if (err != 0) {
		return err;
	}

	return 0;
}

/**
 * @brief Update region of interest (RoI) for target window
 * @details RoI is dynamic configuratable attributes.
 * @param[in] idx    Index of the target video window
 * @param[in] roi    Pointer to target output RoI.
 * @return The execution result
 * @retval 0      success
 * @retval others unexpeced failure
 */
INT32 SAMPLE_updateWindowRoi(MPI_WIN idx, const MPI_RECT_S *roi)
{
	MPI_WIN_ATTR_S attr;
	int err;

	/** User only needs to update .roi in MPI_WIN_ATTR_S to update RoI */

	err = MPI_DEV_getWindowAttr(idx, &attr);
	if (err != 0) {
		return err;
	}

	attr.roi = *roi;

	err = MPI_DEV_setWindowAttr(idx, &attr);
	if (err != 0) {
		return err;
	}

	return 0;
}

/**
 * @brief Update mirroring flag.
 * @details Mirror is dynamic configuratable attributes.
 * For now, mirroring can be configured on WIN(0, 0, 0) only.
 * @param[in] enable boolean value. 0 for disable, otherwise for enable.
 * @return The execution result
 * @retval 0      success
 * @retval others failure
 */
INT32 SAMPLE_updateMirrorAttr(UINT8 enable)
{
	MPI_WIN_ATTR_S attr;
	int err;

	/** User only needs to update .mirr_en in MPI_WIN_ATTR_S to update mirroring attribute */

	err = MPI_DEV_getWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &attr);
	if (err != 0) {
		return err;
	}

	attr.mirr_en = enable;

	err = MPI_DEV_setWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &attr);
	if (err != 0) {
		return err;
	}

	return 0;
}

/**
 * @brief Update flipping flag.
 * @details Flipping is dynamic configuratable attributes.
 * For now, flipping can be configured on WIN(0, 0, 0) only.
 * @param[in] enable boolean value. 0 for disable, otherwise for enable.
 * @return The execution result
 * @retval 0      success
 * @retval others failure
 */
INT32 SAMPLE_updateFlipAttr(UINT8 enable)
{
	MPI_WIN_ATTR_S attr;
	int err;

	/** User only needs to update .flip_en in MPI_WIN_ATTR_S to update flipping attribute */

	err = MPI_DEV_getWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &attr);
	if (err != 0) {
		return err;
	}

	attr.flip_en = enable;

	err = MPI_DEV_setWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &attr);
	if (err != 0) {
		return err;
	}

	return 0;
}

/**
 * @brief Reconfig window view type when video pipeline is running.
 * @details
 * The sample function is expected to work under the following context:
 *  - Video device, video channels and encoders are running.
 *    Otherwise, stop flow and restart flow must be adjusted
 *  - Object detection is disabled before reconfiguring resolution.
 * @param[in,out] conf    Pointer to video pipeline configuration.
 * @param[in]     idx     Index of the target video window
 * @param[in]     res     Target window view type
 * @return The execution result
 * @retval 0      success
 * @exception SIGABRT if any MPI returns not success.
 * @exception SIGFAULT if NULL pointer is received.
 */
INT32 SAMPLE_reconfigWindowViewType(const SAMPLE_CONF_S *conf, MPI_WIN idx, MPI_WIN_VIEW_TYPE_E type)
{
	MPI_WIN_ATTR_S win_attr;
	int err;
	int i;

	/** To reconfigure window view type, update field in MPI_WIN_ATTR_S is needed. */

	err = MPI_DEV_getWindowAttr(idx, &win_attr);
	assert(err == 0);

	win_attr.view_type = type;

	/** First, stop encoders and video channels before configuring window view type. */

	for (int i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_stopChn(MPI_ENC_CHN(i));
		assert(err == 0);

		err = MPI_ENC_unbindFromVideoChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	err = MPI_DEV_stopAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	/** Stop all OSD in video channels */
	if (conf->osd_visible) {
		disableOsd(conf);
	}

	/** Then, call related setting MPIs to configure window view type */

	err = MPI_DEV_setWindowAttr(idx, &win_attr);
	assert(err == 0);

	/** Add OSD back to video channels */
	if (conf->osd_visible) {
		enableOsd(conf);
	}

	/** Finally, restart video channels and encoders to generate frames again. */

	err = MPI_DEV_startAllChn(MPI_VIDEO_DEV(0));
	assert(err == 0);

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_bindToVideoChn(MPI_ENC_CHN(i), &conf->enc_chn[i].bind);
		assert(err == 0);

		err = MPI_ENC_startChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	return 0;
}

/**
 * @brief Update stitch attribute.
 * @details Parameters of stitch transform is dynamic configuratable attributes.
 * For now, stitch can be configured on WIN(0, 0, 0) only.
 * @param[in] attr    pointer to stitch attributes
 * @return The execution result
 * @retval 0       success
 * @retval others  failure
 */
INT32 SAMPLE_updateStitchAttr(const MPI_STITCH_ATTR_S *attr)
{
	/** User only needs to update MPI_STITCH_ATTR_S */
	return MPI_DEV_setStitchAttr(MPI_VIDEO_WIN(0, 0, 0), attr);
}

/**
 * @brief Update panorama attribute.
 * @details Parameters of panorama transform is dynamic configuratable attributes.
 * For now, panorama can be configured on WIN(0, 0, 0) only.
 * @param[in] attr    pointer to panorama attributes
 * @return The execution result
 * @retval 0       success
 * @retval others  failure
 */
INT32 SAMPLE_updatePanoramaAttr(const MPI_PANORAMA_ATTR_S *attr)
{
	/** User only needs to update MPI_PANORAMA_ATTR_S */
	return MPI_DEV_setPanoramaAttr(MPI_VIDEO_WIN(0, 0, 0), attr);
}

/**
 * @brief Update panning attribute.
 * @details Parameters of panning transform is dynamic configuratable attributes.
 * For now, panning can be configured on WIN(0, 0, 0) only.
 * @param[in] attr    pointer to panning attributes
 * @return The execution result
 * @retval 0       success
 * @retval others  failure
 */
INT32 SAMPLE_updatePanningAttr(const MPI_PANNING_ATTR_S *attr)
{
	/** User only needs to update MPI_PANNING_ATTR_S */
	return MPI_DEV_setPanningAttr(MPI_VIDEO_WIN(0, 0, 0), attr);
}

/**
 * @brief Update surround attribute.
 * @details Parameters of surround transform is dynamic configuratable attributes.
 * For now, surround can be configured on WIN(0, 0, 0) only.
 * @param[in] attr    pointer to surround attributes
 * @return The execution result
 * @retval 0      success
 * @retval others failure
 */
INT32 SAMPLE_updateSurroundAttr(const MPI_SURROUND_ATTR_S *attr)
{
	/** User only needs to update MPI_SURROUND_ATTR_S */
	return MPI_DEV_setSurroundAttr(MPI_VIDEO_WIN(0, 0, 0), attr);
}

/**
 * @brief Update lens distortion correction attribute.
 * @details Parameters of LDC transform is dynamic configuratable attributes.
 * For now, LDC can be configured on WIN(0, 0, 0) only.
 * @param[in] attr    pointer to LDC attributes
 * @return The execution result
 * @retval 0      success
 * @retval others failure
 */
INT32 SAMPLE_updateLdcAttr(const MPI_LDC_ATTR_S *attr)
{
	/** User only needs to update MPI_LDC_ATTR_S */
	return MPI_DEV_setLdcAttr(MPI_VIDEO_WIN(0, 0, 0), attr);
}

/**
 * @brief Reconfig codec type when video pipeline is running.
 * @details
 * The sample function is expected to work under the following context:
 *  - Video device, video channels and encoders are running
 *    Otherwise, stop flow and restart flow must be adjusted
 * @param[in,out] conf      Pointer to video pipeline configuration.
 * @param[in]     idx       Index of the target video channel
 * @param[in]     layout    Pointer to target encoder attributes.
 * @return The execution result
 * @retval 0      success
 * @exception SIGABRT if any MPI returns not success.
 */
INT32 SAMPLE_reconfigCodec(const SAMPLE_CONF_S *conf, MPI_ECHN idx, const MPI_VENC_ATTR_S *attr)
{
	int err;
	int i;

	/** First, stop all encoders before configuring codec type. */

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_stopChn(MPI_ENC_CHN(i));
		assert(err == 0);

		err = MPI_ENC_unbindFromVideoChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	/** Stop all OSD in video channels */
	if (conf->osd_visible) {
		disableOsd(conf);
	}

	/** Then, call MPI to configure codec type. */

	err = MPI_ENC_setVencAttr(idx, attr);
	assert(err == 0);

	/** Add OSD back to video channels */
	if (conf->osd_visible) {
		enableOsd(conf);
	}

	/** Finally, restart encoders to generate frames again. */

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (!conf->enc_chn[i].enable) {
			continue;
		}

		err = MPI_ENC_bindToVideoChn(MPI_ENC_CHN(i), &conf->enc_chn[i].bind);
		assert(err == 0);

		err = MPI_ENC_startChn(MPI_ENC_CHN(i));
		assert(err == 0);
	}

	return 0;
}

/**
 * @brief Update VBR parameters when encoder is running on VBR mode.
 * @param[in] idx      Index of the target encoder
 * @param[in] param    Pointer to target VBR parameters
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateVbrParams(MPI_ECHN idx, const MPI_MCVC_VBR_PARAM_S *param)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.vbr = *param;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.vbr = *param;
		break;

	default:
		return -1;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update CBR parameters when encoder is running on CBR mode.
 * @param[in] idx      Index of the target encoder
 * @param[in] param    Pointer to target CBR parameters
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateCbrParams(MPI_ECHN idx, const MPI_MCVC_CBR_PARAM_S *param)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.cbr = *param;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.cbr = *param;
		break;

	default:
		return -EFAULT;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update SBR parameters when encoder is running on SBR mode.
 * @param[in] idx      Index of the target encoder
 * @param[in] param    Pointer to target SBR parameters
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateSbrParams(MPI_ECHN idx, const MPI_MCVC_SBR_PARAM_S *param)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.sbr = *param;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.sbr = *param;
		break;

	default:
		return -EFAULT;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update CQP parameters when encoder is running on CQP mode.
 * @param[in] idx      Index of the target encoder
 * @param[in] param    Pointer to target CQP parameters
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateCqpParams(MPI_ECHN idx, const MPI_MCVC_CQP_PARAM_S *param)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.cqp = *param;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.cqp = *param;
		break;

	default:
		return -EFAULT;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update GOP parameters when encoder is running in H.264 or H.265.
 * @param[in] idx      Index of the target encoder
 * @param[in] param    Target GOP
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateGopAttr(MPI_ECHN idx, UINT32 gop)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.gop = gop;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.gop = gop;
		break;

	default:
		return -EFAULT;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

static inline void setBitRate(void *param, MPI_RC_MODE_E mode, UINT32 rate)
{
	switch (mode) {
	case MPI_RC_MODE_VBR:
		((MPI_MCVC_RC_ATTR_S *)param)->vbr.max_bit_rate = rate;
		break;

	case MPI_RC_MODE_CBR:
		((MPI_MCVC_RC_ATTR_S *)param)->cbr.bit_rate = rate;
		break;

	case MPI_RC_MODE_SBR:
		((MPI_MCVC_RC_ATTR_S *)param)->sbr.bit_rate = rate;
		break;

	default:
		break;
	}

	return;
}

/**
 * @brief Update encoder bitrate when encoder is running in H.264 or H.265.
 * @param[in] idx      Index of the target encoder
 * @param[in] rate     Target bit rate
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 */
INT32 SAMPLE_updateBitRate(MPI_ECHN idx, UINT32 rate)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		setBitRate(&attr.h264.rc, attr.h264.rc.mode, rate);
		break;

	case MPI_VENC_TYPE_H265:
		setBitRate(&attr.h265.rc, attr.h265.rc.mode, rate);
		break;

	default:
		return -EFAULT;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update encoder bitrate when encoder is running in H.264 or H.265.
 * @param[in] idx      Index of the target encoder
 * @param[in] fps      Target fps
 * @return The execution result
 * @retval 0          success
 * @retval others     unexpected failure
 * @note only support H264/H265
 */
INT32 SAMPLE_updateSpsVuiFpsInfo(MPI_ECHN idx, UINT32 fps)
{
	MPI_VENC_ATTR_S attr;
	int ret;

	ret = MPI_ENC_getVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		attr.h264.rc.frm_rate_o = fps;
		break;

	case MPI_VENC_TYPE_H265:
		attr.h265.rc.frm_rate_o = fps;
		break;

	default:
		/** only H264/H265 need to change VUI*/
		return -EINVAL;
	}

	ret = MPI_ENC_setVencAttr(idx, &attr);
	if (ret != 0) {
		return ret;
	}

	return 0;
}