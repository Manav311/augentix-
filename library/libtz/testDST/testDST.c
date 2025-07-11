#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tz_upd.h"

int main(int argc __attribute__((unused)), char **argv)
{
	int value = 0;
	int enable = 0;
	char *TZ;

	if(atoi(argv[1]) < 0){
		return -1;
	} else{

		value = atoi(argv[1]);
		TZ = argv[2];

		if ( value == 0 || value == 1 ){
			if(setTimeinfo(value, TZ) !=0){
				printf("Set time info fail");
			}
			printf("The Timezone is %s\n", TZ);
			printf("The DST enable was setted to %d\n", value);
		}
		else if ( value == 2 ){
			enable = getDst();
			printf("The DST enable is %d\n", enable);
		}
		else {
			printf(" Value should be within 0~2\n");
		}
	}

return 0;
}
