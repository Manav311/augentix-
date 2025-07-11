#include "sample_stream.h"

#include <errno.h>
#include <stdio.h>

#include "mpi_dip_alg.h"
#include "sensor.h"

/** Link symbols stored in sensor library */

extern CUSTOM_SNS_CTRL_S custom_sns(SNS0_ID);
#ifdef SNS1
extern CUSTOM_SNS_CTRL_S custom_sns(SNS1_ID);
#endif
#ifdef SNS2
extern CUSTOM_SNS_CTRL_S custom_sns(SNS2_ID);
#endif
#ifdef SNS3
extern CUSTOM_SNS_CTRL_S custom_sns(SNS3_ID);
#endif

static CUSTOM_SNS_CTRL_S *p_custom_sns[] = {
	&custom_sns(SNS0_ID),
#ifdef SNS1
	&custom_sns(SNS1_ID),
#ifdef SNS2
	&custom_sns(SNS2_ID),
#ifdef SNS3
	&custom_sns(SNS3_ID),
#endif
#endif
#endif
};

/**
 * @brief Create and configure video pipeline.
 * @details This function aims to demo how to create and configure video
 * pipeline. To do so, User should call the following MPIs in sequence.
 * @param[in] conf    pointer to the application configuration
 * @return The execution result.
 * @retval 0      success
 * @retval others unexpected failure
 * @see SAMPLE_destroyVideoPipeline()
 * @see SAMPLE_startStream()
 */
int SAMPLE_createVideoPipeline(const SAMPLE_CONF_S *conf)
{
	UINT32 p_idx = 0;
	UINT32 c_idx = 0;
	UINT32 w_idx = 0;
	UINT32 e_idx = 0;
	UINT32 sensor_idx;
	MPI_PATH path_idx;
	MPI_CHN chn_idx;
	MPI_WIN win_idx;
	MPI_ECHN echn_idx;
	const MPI_CHN_LAYOUT_S *chn_layout;
	int ret;

	/** Configure then initialize video buffer. */

	ret = MPI_VB_setConf(&conf->sys.vb_conf);
	if (ret != MPI_SUCCESS) {
		printf("Configure video buffer failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_VB_init();
	if (ret != MPI_SUCCESS) {
		printf("Initialize video buffer failed. err: %d\n", ret);
		return ret;
	}

	/** Create and configure video device. */

	ret = MPI_DEV_createDev(MPI_VIDEO_DEV(0), &conf->dev[0].attr);
	if (ret != MPI_SUCCESS) {
		printf("Create video device failed. err: %d\n", ret);
		return ret;
	}

	/** Add and configure input path. */

	for (p_idx = 0; p_idx < MPI_MAX_INPUT_PATH_NUM; p_idx++) {
		if (!conf->dev[0].path[p_idx].enable) {
			continue;
		}

		path_idx = MPI_INPUT_PATH(0, p_idx);
		sensor_idx = conf->dev[0].path[p_idx].attr.sensor_idx;
		ret = MPI_DEV_addPath(path_idx, &conf->dev[0].path[p_idx].attr);
		if (ret != MPI_SUCCESS) {
			printf("Set input path %d failed. err: %d\n", MPI_GET_INPUT_PATH(path_idx), ret);
			return ret;
		}

		p_custom_sns[sensor_idx]->reg_callback(path_idx);

		/* Register AE, AWB default libraries to input path */
		MPI_regAeDftLib(path_idx);
		MPI_regAwbDftLib(path_idx);

		/* Get parameter from sensor driver */
		MPI_updateSnsParam(path_idx);
	}
	/** Add then configure video channels and video windows. */

	for (c_idx = 0; c_idx < MPI_MAX_VIDEO_CHN_NUM; c_idx++) {
		if (!conf->dev[0].chn[c_idx].enable) {
			continue;
		}

		chn_idx = MPI_VIDEO_CHN(0, c_idx);
		ret = MPI_DEV_addChn(chn_idx, &conf->dev[0].chn[c_idx].attr);
		if (ret != MPI_SUCCESS) {
			printf("Add video channel %d failed. err: %d\n", MPI_GET_VIDEO_CHN(chn_idx), ret);
			return ret;
		}

		chn_layout = &conf->dev[0].chn[c_idx].layout;
		ret = MPI_DEV_setChnLayout(chn_idx, chn_layout);
		if (ret != MPI_SUCCESS) {
			printf("Set video channel layout %d failed. err: %d\n", MPI_GET_VIDEO_CHN(chn_idx), ret);
			return ret;
		}

		for (w_idx = 0; (INT32)w_idx < chn_layout->window_num; w_idx++) {
			printf("c_idx = %d\n", c_idx);
			win_idx = MPI_VIDEO_WIN(0, c_idx, w_idx);
			ret = MPI_DEV_setWindowAttr(win_idx, &conf->dev[0].chn[c_idx].win[w_idx]);
			if (ret != MPI_SUCCESS) {
				printf("Set video window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
				return ret;
			}

			MPI_WIN_ATTR_S win_attr;
			ret = MPI_DEV_getWindowAttr(win_idx, &win_attr);
			if (ret != MPI_SUCCESS) {
				printf("Get video window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
			}
		}
	}
	/**
	 * Determine parameters of various view types (Transformation)
	 *
	 * Notes that view type attributes can be configured to WIN(0, 0, 0) only.
	 */

	win_idx = MPI_VIDEO_WIN(0, 0, 0);

	ret = MPI_DEV_setStitchAttr(win_idx, &conf->dev[0].stitch);
	if (ret != MPI_SUCCESS) {
		printf("Set stitch attr for window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
		return ret;
	}

	ret = MPI_DEV_setLdcAttr(win_idx, &conf->dev[0].ldc);
	if (ret != MPI_SUCCESS) {
		printf("Set LDC attr for window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
		return ret;
	}

	ret = MPI_DEV_setPanoramaAttr(win_idx, &conf->dev[0].panorama);
	if (ret != MPI_SUCCESS) {
		printf("Set panorama attr for window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
		return ret;
	}

	ret = MPI_DEV_setPanningAttr(win_idx, &conf->dev[0].panning);
	if (ret != MPI_SUCCESS) {
		printf("Set panning attr for window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
		return ret;
	}

	ret = MPI_DEV_setSurroundAttr(win_idx, &conf->dev[0].surround);
	if (ret != MPI_SUCCESS) {
		printf("Set surround attr for window %d failed. err: %d\n", MPI_GET_VIDEO_WIN(win_idx), ret);
		return ret;
	}

	/** Create and configure encoder channels. */

	for (e_idx = 0; e_idx < MPI_MAX_ENC_CHN_NUM; e_idx++) {
		if (!conf->enc_chn[e_idx].enable) {
			continue;
		}

		echn_idx = MPI_ENC_CHN(e_idx);
		ret = MPI_ENC_createChn(echn_idx, &conf->enc_chn[e_idx].attr);
		if (ret != MPI_SUCCESS) {
			printf("Create encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}

		ret = MPI_ENC_setVencAttr(echn_idx, &conf->enc_chn[e_idx].venc_attr);
		if (ret != MPI_SUCCESS) {
			printf("Set VENC attr for encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}

		ret = MPI_ENC_setVencAttrEx(echn_idx, &conf->enc_chn[e_idx].venc_ex);
		if (ret != MPI_SUCCESS) {
			printf("Set VENC attr ex for encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Destroy video pipeline.
 * @details This function aims to demo how to destroy video pipeline. To do so,
 * User should call the following MPIs in sequence.
 * @param[in] conf    pointer to video pipeline configuration
 * @return The execution result.
 * @retval 0         success
 * @retval others    unexpected failure
 * @see SAMPLE_createVideoPipeline()
 * @see SAMPLE_stopStream()
 */
int SAMPLE_destroyVideoPipeline(const SAMPLE_CONF_S *conf)
{
	UINT32 c_idx = 0;
	UINT32 p_idx = 0;
	UINT32 e_idx = 0;
	MPI_ECHN echn_idx;
	MPI_CHN chn_idx;
	MPI_PATH path_idx;
	UINT32 sensor_idx;

	int ret;

	/** Destroy encoder channels. */

	for (e_idx = 0; e_idx < MPI_MAX_ENC_CHN_NUM; ++e_idx) {
		if (!conf->enc_chn[e_idx].enable) {
			continue;
		}

		echn_idx = MPI_ENC_CHN(e_idx);
		ret = MPI_ENC_destroyChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Destroy encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}
	}

	/** Delete video channels. */

	for (c_idx = 0; c_idx < MPI_MAX_VIDEO_CHN_NUM; ++c_idx) {
		if (!conf->dev[0].chn[c_idx].enable) {
			continue;
		}

		chn_idx = MPI_VIDEO_CHN(0, c_idx);
		ret = MPI_DEV_deleteChn(chn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Delete video channel %d failed. err: %d\n", MPI_GET_VIDEO_CHN(chn_idx), ret);
			return ret;
		}
	}

	/** Delete input paths */

	for (p_idx = 0; p_idx < MPI_MAX_INPUT_PATH_NUM; ++p_idx) {
		if (!conf->dev[0].path[p_idx].enable) {
			continue;
		}

		path_idx = MPI_INPUT_PATH(0, p_idx);
		sensor_idx = conf->dev[0].path[p_idx].attr.sensor_idx;

		MPI_deregAeDftLib(path_idx);
		MPI_deregAwbDftLib(path_idx);

		p_custom_sns[sensor_idx]->dereg_callback(path_idx);

		MPI_DEV_deletePath(path_idx);
	}

	/** Destroy video device */

	ret = MPI_DEV_destroyDev(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		printf("Destroy video device failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_VB_exit();
	if (ret != MPI_SUCCESS) {
		printf("Exit video buffer failed. err: %d\n", ret);
		return ret;
	}

	printf("Stop stream succeeded!\n");
	return 0;
}

/**
 * @brief Start video pipeline.
 * @details This function aims to demo how to start video pipeline. To do so,
 * User should call the following MPIs in sequence.
 * @param[in] conf    pointer to the application configuration
 * @return The execution result.
 * @retval 0      success
 * @retval others unexpected failure
 * @see SAMPLE_stopStream()
 * @see SAMPLE_createVideoPipeline()
 */
int SAMPLE_startStream(const SAMPLE_CONF_S *conf)
{
	MPI_ECHN echn_idx;
	int idx;
	int ret;

	ret = MPI_DEV_startDev(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		printf("Start video device failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_DEV_startAllChn(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		printf("Start all video channels on video device 0 failed. err: %d\n", ret);
		if (ret == -EAGAIN) {
			ret = MPI_DEV_stopDev(MPI_VIDEO_DEV(0));
			if (ret != MPI_SUCCESS) {
				printf("Stop video device 0 failed. err: %d\n", ret);
				return ret;
			} else {
				printf("Stop video device 0 success.\n");
				return -EAGAIN;
			}
		} else {
			return ret;
		}
	}

	for (idx = 0; idx < MPI_MAX_ENC_CHN_NUM; idx++) {
		if (!conf->enc_chn[idx].enable) {
			continue;
		}

		echn_idx = MPI_ENC_CHN(idx);
		ret = MPI_ENC_bindToVideoChn(echn_idx, &conf->enc_chn[idx].bind);
		if (ret != MPI_SUCCESS) {
			printf("Bind encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}

		ret = MPI_ENC_startChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Start encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Stop video pipeline.
 * @details This function aims to demo how to stop video pipeline. To do so,
 * User should call the following MPIs in sequence.
 * @param[in] conf    pointer to the application configuration
 * @return The execution result.
 * @retval 0      success
 * @retval others unexpected failure
 * @see SAMPLE_startStream()
 * @see SAMPLE_destroyVideoPipeline()
 */
int SAMPLE_stopStream(const SAMPLE_CONF_S *conf)
{
	MPI_ECHN echn_idx;
	int idx;
	int ret;

	for (idx = 0; idx < MPI_MAX_ENC_CHN_NUM; idx++) {
		if (!conf->enc_chn[idx].enable) {
			continue;
		}

		echn_idx = MPI_ENC_CHN(idx);
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Stop encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}

		ret = MPI_ENC_unbindFromVideoChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Unbind encoder channel %d failed.\n", MPI_GET_ENC_CHN(echn_idx));
			return ret;
		}
	}

	ret = MPI_DEV_stopAllChn(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		printf("Stop all video channels on video device 0 failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_DEV_stopDev(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		printf("Stop video device 0 failed. err: %d\n", ret);
		return ret;
	}

	return 0;
}
