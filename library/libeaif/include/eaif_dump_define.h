/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

/**
 * @file eaifdump_define.h
 * @brief The definitions of message content
 */
#ifndef EAIF_DUMP_DEFINE_H_
#define EAIF_DUMP_DEFINE_H_

#include "vftr_dump_define.h"

#define FLAG_CAT_EAIF 1

/* Array of structures for debugging */
#define EXPORT_EAIF_DUMP_ARRAY                                         \
	EXPORT_DUMP(MPI_IVA_OBJ_LIST_S, FLAG_TYPE_INPUT)               \
	EXPORT_DUMP(EAIF_INSTANCE_S, FLAG_TYPE_INPUT)                  \
	EXPORT_DUMP(EAIF_PARAM_S, FLAG_TYPE_OUTPUT)                    \
	EXPORT_DUMP(EAIF_STATUS_S, FLAG_TYPE_OUTPUT)                   \
	EXPORT_DUMP(InfObjList, FLAG_TYPE_INPUT)                       \
	EXPORT_DUMP(InfImage, FLAG_TYPE_IMG)                           \
	EXPORT_DUMP(InfU8Array, FLAG_TYPE_IMG)                         \
	EXPORT_DUMP(InfResultList, FLAG_TYPE_INPUT)                    \
	EXPORT_DUMP(InfResult, FLAG_TYPE_INPUT)                        \
	EXPORT_DUMP(InfIntArray, FLAG_TYPE_INPUT | FLAG_TYPE_OUTPUT)   \
	EXPORT_DUMP(InfFloatArray, FLAG_TYPE_INPUT | FLAG_TYPE_OUTPUT) \
	EXPORT_DUMP(InfDetList, FLAG_TYPE_INPUT)                       \
	EXPORT_DUMP(InfDetResult, FLAG_TYPE_INPUT)                     \
	EXPORT_DUMP(InfFaceImage, FLAG_TYPE_IMG)                       \
	EXPORT_DUMP(InfFaceU8Array, FLAG_TYPE_IMG)

/*
 * @brief An array macro to collect all data structures for debugging
 * @note Maintain the following array if new VFTR structure added
 * PLEASE ADD NEW ELEMENT AT LAST
 */

#define EXPORT_DUMP(name, type) EAIF_ID_##name,
#define DECLARE_EAIF_ENUM_ID(name, array) enum name { array MAX_EAIF_DUMP_ID_NUM };
/**<
 * @brief The following code generates structure ids
 * enum vftr_dump_id  {
 *      ID_MPI_IVA_OBJ_LIST_S,
 *      ID_VFTR_MD_INSTANCE_S,
 *      ID_VFTR_MD_PARAM_S,
 *      ID_VFTR_MD_STATUS_S,
 *      ...
 *      MAX_VFTR_DUMP_ID_NUM
 * };
 */

DECLARE_EAIF_ENUM_ID(eaif_dump_id, EXPORT_EAIF_DUMP_ARRAY);

#undef EXPORT_DUMP
#undef DECLARE_EAIF_ENUM_ID

#define EXPORT_DUMP(name, type) EAIF_TYPE_##name = (type),
#define DECLARE_EAIF_ENUM_TYPE(name, array) enum name { array MAX_EAIF_DUMP_TYPE_NUM };
/**<
 * @brief The following code generates structure types
 * @details Produce the code like:
 * enum vfre_dump_type  {
 *      TYPE_MPI_IVA_OBJ_LIST_S = type,
 *      TYPE_VFTR_MD_INSTANCE_S = type,
 *      TYPE_VFTR_MD_PARAM_S = type,
 *      TYPE_VFTR_MD_STATUS_S = type,
 *      ...
 *      MAX_VFTR_DUMP_TYPE_NUM
 * };
 */

DECLARE_EAIF_ENUM_TYPE(eaif_dump_type, EXPORT_EAIF_DUMP_ARRAY);

#undef EXPORT_DUMP
#undef DECLARE_EAIF_ENUM_TYPE

#define COMPOSE_EAIF_FLAG_VER_1(name, group)                                                                   \
	((EAIF_ID_##name) + ((EAIF_TYPE_##name) << 10) + ((FLAG_CAT_EAIF) << 16) + ((EAIF_ID_##group) << 19) + \
	 ((FLAG_VER_1) << 30))

#endif /* VFTR_DUMP_DEFINE_H_ */
