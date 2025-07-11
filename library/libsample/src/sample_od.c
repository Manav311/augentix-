#include "sample_od.h"

#ifdef CB_BASED_OD

INT32 SAMPLE_OD_registerCallback(MPI_WIN idx, MPI_IVA_OD_CALLBACK_S *cb)
{
	(void)idx;

	cb->init = NULL;
	cb->detect = NULL;
	cb->exit = NULL;

#ifdef USE_PEOPLE_VEHICLE_PET_MODEL
	cb->init = ML_OD_Init_PeopleVehiclePet;
	cb->detect = ML_OD_Detect_PeopleVehiclePet;
	cb->exit = ML_OD_Exit_PeopleVehiclePet;
#endif /*< USE_PEOPLE_VEHICLE_PET_MODEL */

#ifdef USE_PEOPLE_VEHICLE_PET_LITE_MODEL
	cb->init = ML_OD_Init_PeopleVehiclePet_Lite;
	cb->detect = ML_OD_Detect_PeopleVehiclePet_Lite;
	cb->exit = ML_OD_Exit_PeopleVehiclePet_Lite;
#endif /*< USE_PEOPLE_VEHICLE_PET_MODEL */

#ifdef USE_PEOPLE_LITE_MODEL
	cb->init = ML_OD_Init_People_Lite;
	cb->detect = ML_OD_Detect_People_Lite;
	cb->exit = ML_OD_Exit_People_Lite;
#endif /*< USE_PEOPLE_VEHICLE_PET_MODEL */

	return 0;
}
#endif //CB_BASED_OD