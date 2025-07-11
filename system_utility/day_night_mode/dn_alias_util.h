#define DN_DEBUG (0)

#if DN_DEBUG
#define DN_TRACE(x, ...) fprintf(stderr, "[DN][DEBUG] " x, ##__VA_ARGS__)
#else
#define DN_TRACE(x, ...)
#endif

#define DN_ERR(x, ...) fprintf(stderr, "[DN][ERROR] %s(): " x, __func__, ##__VA_ARGS__)
#define DN_WARN(x, ...) fprintf(stderr, "[DN][WARNING] %s(): " x, __func__, ##__VA_ARGS__)
#define DN_NOTICE(x, ...) fprintf(stderr, "\n[DN][NOTICE] %s(): " x, __func__, ##__VA_ARGS__)
#define DN_INFO(x, ...) fprintf(stderr, "[DN][INFO] " x, ##__VA_ARGS__)

#define DAY_COLOR_SETTING_VIA_MPI (111)
#define NIGHT_COLOR_SETTING_VIA_MPI (222)
#define ACTIVE_IR_CUT (333)
#define REMOVE_IR_CUT (444)
#define TRY (555)
#define DAY_COLOR_SETTING_VIA_CC (666)
#define NIGHT_COLOR_SETTING_VIA_CC (777)
#define READ_DAY_NIGHT_ATTR (888)
#define DIRECT_SWITCH_BY_READ_DAY_NIGHT_ATTR (999)

#define CTRL_IR_CUT_CMD "sh /system/mpp/script/ir_cut.sh"
#define CTRL_IR_LED_CMD "sh /system/mpp/script/ir_led.sh"

typedef struct {
	char *name;
	int val;
} ALIAS_S;

static ALIAS_S k_alias[] = {
	{ "day_color_setting_via_mpi", DAY_COLOR_SETTING_VIA_MPI },
	{ "night_color_setting_via_mpi", NIGHT_COLOR_SETTING_VIA_MPI },
	{ "active_ir_cut", ACTIVE_IR_CUT },
	{ "remove_ir_cut", REMOVE_IR_CUT },
	{ "try", TRY },
	{ "day_color_setting_via_cc", DAY_COLOR_SETTING_VIA_CC },
	{ "night_color_setting_via_cc", NIGHT_COLOR_SETTING_VIA_CC },
	{ "read_day_night_attr", READ_DAY_NIGHT_ATTR },
};

static inline int ALIAS_str2num(const char *str)
{
	int i;
	int ret;
	int num_of_element;

	num_of_element = sizeof(k_alias) / sizeof(ALIAS_S);

	if (num_of_element <= 0) {
		DN_ERR("Invaild element size(%d)!\n", num_of_element);
		ret = -1;
		return ret;
	}

	ret = -1;
	for (i = 0; i < num_of_element; i++) {
		if (strcmp(str, k_alias[i].name) == 0) {
			ret = k_alias[i].val;
			return ret;
		}
	}

	if (ret < 0) {
		DN_ERR("Name(%s) no match\n", str);
	}

	return ret;
}

static inline char *ALIAS_num2str(const int num)
{
	int i;
	char *str;
	int num_of_element;

	num_of_element = sizeof(k_alias) / sizeof(ALIAS_S);

	if (num_of_element <= 0) {
		DN_ERR("Invaild element size(%d)!\n", num_of_element);
		str = NULL;
		return str;
	}

	str = NULL;
	for (i = 0; i < num_of_element; i++) {
		if (k_alias[i].val == num) {
			str = k_alias[i].name;
			return str;
		}
	}

	if (str == NULL) {
		DN_ERR("Value(%d) no match!\n", num);
	}

	return str;
}