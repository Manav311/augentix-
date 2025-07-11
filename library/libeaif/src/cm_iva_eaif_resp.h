#ifndef CM_IVA_EAIF_RESP_H_
#define CM_IVA_EAIF_RESP_H_

typedef signed int AGTX_INT32;
typedef unsigned int AGTX_UINT32;
typedef signed char AGTX_UINT8;
typedef unsigned char AGTX_INT8;

#define MAX_EAIF_RESP_OBJ_ATTR_SIZE 32
#define MAX_EAIF_RESP_OBJ_CLS_NUM 5

#define MAX_AGTX_IVA_EAIF_RESP_S_PREDICTIONS_SIZE 30

#ifndef EAIF_SUPPORT_JSON
struct json_object {
	int dummy;
};
#else
struct json_object;
#endif

typedef struct {
	AGTX_INT32 idx;
	AGTX_INT32 label_num;
	AGTX_INT32 rect[4];
	AGTX_UINT8 label[MAX_EAIF_RESP_OBJ_CLS_NUM][MAX_EAIF_RESP_OBJ_ATTR_SIZE];
	AGTX_UINT8 prob[MAX_EAIF_RESP_OBJ_CLS_NUM][MAX_EAIF_RESP_OBJ_ATTR_SIZE];
} AGTX_IVA_EAIF_RESP_OBJ_ATTR_S;

typedef struct {
	AGTX_INT32 pred_num;
	AGTX_IVA_EAIF_RESP_OBJ_ATTR_S predictions[MAX_AGTX_IVA_EAIF_RESP_S_PREDICTIONS_SIZE];
	AGTX_INT32 success; /* Post response */
	AGTX_INT32 time; /* timestamp */
} AGTX_IVA_EAIF_RESP_S;

void parse_iva_eaif_resp(AGTX_IVA_EAIF_RESP_S *data, struct json_object *cmd_obj);
void comp_iva_eaif_resp(struct json_object *ret_obj, AGTX_IVA_EAIF_RESP_S *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_IVA_EAIF_RESP_H_ */
