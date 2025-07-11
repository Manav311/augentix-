#ifndef ADO_CB_H_
#define ADO_CB_H_

enum acodec_id {
	ACODEC_ADC = 0,
	ACODEC_AK4637,
	ACODEC_CJC8990,
	ACODEC_RT5660,
	ACODEC_WM8731,
	ACODEC_DUMMY,
	ACODEC_MAX_ID,
};

struct ado_callback {
	enum acodec_id id;
	int (*set_volume)(unsigned int volume);
	int (*get_db_gain)(unsigned int volume_before, unsigned int volume_after, float *db_gain);
};

struct ado_callback_list {
	struct ado_callback *cb;
	struct ado_callback_list *next;
};

#endif /* ADO_CB_H_ */
