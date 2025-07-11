#ifndef LIGHT_H
#define LIGHT_H

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

#define MPI_IIR_PRC (8)
#define MPI_IIR_UNIT (1 << MPI_IIR_PRC)

void initMpi();
void exitMpi();

int calcMpiSceneLuma(const int dev, const int path);
int calcMpiLumaIir(const int pre_luma, const int now_luma, const int current_weight);

int checkRange(char *str, int value, int low_val, int high_val);
int binarySearch(const int value, const int *arry, const int i0, const int i1);
int arrPixIntpl(const int val, const int curve_num, const int *curve_bin, const int *curve_val);
int alphaPixBlend(const int pix0, const int pix1, const int alpha, const int norm);

#endif
