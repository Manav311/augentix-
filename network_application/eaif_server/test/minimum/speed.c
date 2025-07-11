#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "eaif_trc.h"
#include "lite_classifier.h"

void help(char *help_message)
{
	fprintf(stderr,
	        "\n%s\n"
	        "\n\t usage:"
	        "\t<img w:int> <h:int> <c:int> <sleep:float> <looptime:int> <model config:str>\n\n",
	        help_message);
}

int test_speed(int w, int h, int c, float sleep, int num_time, char *config)
{
	struct timespec start;
	int ret = 0;
	LiteImage img = {};
	LiteResultList result = {};
	LiteClassifierCtx ctx = {};
	MPI_IVA_OBJ_LIST_S obj_list = {};

	img.w = w;
	img.h = h;
	img.c = c;
	img.dtype = (c == 1) ? Lite8UC1 : Lite8UC3;
	img.data = (unsigned char *)calloc(1, w * h * c);

	ret = Lite_InitModel(&ctx, config);
	assert(ret == 0);

	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, w / 2, h / 2 };
	obj_list.obj[0].life = 160;

	int usleep_period = sleep * 1000000;

	for (int i = 0; i < num_time; i++) {
		TIC(start);
		ret = Lite_Invoke(&ctx, &img, &obj_list, &result);
		TOC("inference", start);
		Lite_ReleaseResult(&result);
		usleep(usleep_period);
	}

	ret = Lite_ReleaseResult(&result);
	assert(ret == 0);

	ret = Lite_ReleaseModel(&ctx);
	assert(ret == 0);

	free(img.data);
	assert(result.size == 0 && result.data == 0);
	assert(ctx.model == 0 && ctx.info == 0);

	return 0;
}
int main(int argc, char **argv)
{
	//return 0;
	if (argc != 7)
		help("Input argument number should be 7");

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	int c = atoi(argv[3]);
	float sleep = atof(argv[4]);
	int loop = atoi(argv[5]);
	char *config = argv[6];

	return test_speed(w, h, c, sleep, loop, config);
}
