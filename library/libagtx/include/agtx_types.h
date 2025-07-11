/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_TYPES_H_
#define AGTX_TYPES_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define AGTX_SUCCESS               (0)       /**< Definition of success. */
#define AGTX_FAILURE               (-1)      /**< Definition of failure. */
#define AGTX_UNUSED(x)             (void)(x) /**< Definition of unused. */
#define AGTX_VOID                  (void)    /**< Definition of void. */


/**
 * @brief a typedef for AGTX_UINT8.
 */
typedef unsigned char              AGTX_UINT8;

/**
 * @brief a typedef for AGTX_UINT16.
 */
typedef unsigned short             AGTX_UINT16;

/**
 * @brief a typedef for AGTX_UINT32.
 */
typedef unsigned int               AGTX_UINT32;

/**
 * @brief a typedef for AGTX_UINT64.
 */
typedef unsigned long long         AGTX_UINT64;

/**
 * @brief a typedef for AGTX_INT8.
 */
typedef signed char                AGTX_INT8;

/**
 * @brief a typedef for AGTX_INT16.
 */
typedef signed short               AGTX_INT16;

/**
 * @brief a typedef for AGTX_INT32.
 */
typedef signed int                 AGTX_INT32;

/**
 * @brief a typedef for AGTX_INT64.
 */
typedef signed long long           AGTX_INT64;

/**
 * @brief a typedef for AGTX_FLOAT.
 */
typedef float                      AGTX_FLOAT;


/**
 * @brief Enumeration of AGTX_BOOL.
 */
typedef enum {
	AGTX_FALSE =  0, /**< False. */
	AGTX_TRUE  = -1, /**< True. */
} AGTX_BOOL;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_TYPES_H_ */

