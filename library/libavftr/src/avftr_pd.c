#include "avftr_pd.h"

/**< Deprecated API */
int AVFTR_PD_addInstance(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_deleteInstance(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_getStat(MPI_WIN idx __attribute__((unused)), const AVFTR_PD_CTX_S *vftr_pd_ctx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_getRes(MPI_WIN idx __attribute__((unused)), VIDEO_FTR_OBJ_LIST_S *obj_list __attribute__((unused)),
                    int buf_idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_enable(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_disable(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_getParam(MPI_WIN idx __attribute__((unused)), AVFTR_PD_PARAM_S *param __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_setParam(MPI_WIN idx __attribute__((unused)), const AVFTR_PD_PARAM_S *param __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_writeParam(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

/**< Deprecated API */
int AVFTR_PD_regCallback(MPI_WIN idx __attribute__((unused)),
                         const AVFTR_PD_ALARM_CB alarm_cb_fptr __attribute__((unused)))
{
	return 0;
}
