#ifndef NODES_H_
#define NODES_H_

#include "mpi_base_types.h"

#define CUSTOM_NIGHT_MODE_IQ "/system/mpp/script/sensor_ir_0.ini"
#define DEFAULT_DAY_MODE_IQ "/system/mpp/script/sensor_0.ini"
#define DEFAULT_NIGHT_MODE_IQ "/system/mpp/script/sensor_night.ini"

typedef enum { NONE = -1, VB, DEV, IMAGE_PREFERENCE, CHN, WIN_IMAGE_PREFERENCE, IVA, ENC } NodeId;
typedef enum { UNDO = -1, RESTART, SET } NodeAct;

#define MAX_VB_POOL_NUM (64)
#define NODE_NUM (ENC + 1)

/*Node image pref and win pref used*/
#define PREF_TH (50)

/*NODE set funtion to check agtx struct data what to do*/
typedef enum { ANTI_FLIKER, IMG_PREF, AWB_PREF, ADV_IMG_PREF, COLOR_CONF } NodeImagePreferenceSetType;
typedef enum { FPS, STITCH_ATTR, LDC_ATTR, PANNING_ATTR, PANORAMA_ATTR, SURROUND_ATTR } NodeChnSetType;
typedef enum { NR_WIN, SHP_WIN, IMG_PREF_WIN } NodeWinImagePreferenceSetType;
typedef enum {
	AROI_ATTR,
	BM_ATTR,
	DK_ATTR,
	EAIF_ATTR,
	EF_ATTR,
	FLD_ATTR,
	LSD_ATTR,
	LD_ATTR,
	MD_ATTR,
	OD_ATTR,
	PD_ATTR,
	PFM_ATTR,
	RMS_ATTR,
	SD_ATTR,
	SHD_ATTR,
	TD_ATTR,
	PTZ_ATTR,
	VDBG_ATTR
} NodeIvaSetType;
typedef enum {
	STRM_CONF,
	OSD_SRC,
	OSD_SHOW_WEEK_DAY,
	OSD_PM,
	TD_ALARM,
	MD_ALARM,
	TD_ALARM_END,
	MD_ALARM_END
} NodeEncSetType;

typedef enum { EIS_STRENGTH } NodeDevSetType;

typedef struct {
	NodeId id;
	int (*set)(int /*agtx cmd*/, void * /*data*/);
	int (*init)(void);
	int (*start)(void);
	int (*stop)(void);
	int (*exit)(void);
	void *parent;
	void *child[3];
} Node;

int NODES_initNodes(void);

int NODES_enterNodespreOrderTraversal(Node *current);
int NODES_startNodespreOrderTraversal(Node *current);
int NODES_leaveNodespreOrderTraversal(Node *current);
int NODES_execRestart(Node *current);
int NODES_execSet(Node *node, int agtx_cmd_id, void *data);

int NODE_initVb(void);
int NODE_exitVb(void);
int NODE_startVb(void);
int NODE_stopVb(void);
int NODE_setVb(int cmd_id, void *data);

int NODE_initDev(void);
int NODE_exitDev(void);
int NODE_startDev(void);
int NODE_stopDev(void);
int NODE_setDev(int cmd_id, void *data);

int NODE_initChn(void);
int NODE_exitChn(void);
int NODE_startChn(void);
int NODE_stopChn(void);
int NODE_setChn(int cmd_id, void *data);

int NODE_initEnc(void);
int NODE_exitEnc(void);
int NODE_startEnc(void);
int NODE_stopEnc(void);
int NODE_setEnc(int cmd_id, void *data);

int NODE_initImagePreference(void); /*use errno.h*/
int NODE_exitImagePreference(void);
int NODE_startImagePreference(void);
int NODE_stopImagePreference(void);
int NODE_setImagePreference(int cmd_id, void *data);

int NODE_initIva(void);
int NODE_exitIva(void);
int NODE_startIva(void);
int NODE_stopIva(void);
int NODE_setIva(int cmd_id, void *data);

int NODE_initWinImagePreference(void);
int NODE_exitWinImagePreference(void);
int NODE_startWinImagePreference(void);
int NODE_stopWinImagePreference(void);
int NODE_setWinImagePreference(int cmd_id, void *data);

UINT16 roundingUpAlign16(UINT16 num);
UINT16 roundingDownAlign16(UINT16 num);

#endif