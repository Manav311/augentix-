#ifndef MICROLITE_MODEL_H_
#define MICROLITE_MODEL_H_
#ifdef USE_MICROLITE

//#define STATIC_MODEL
#define SHUFFLENET_MODEL

#ifdef STATIC_MODEL
#include "shufflenet.h"
#endif
//extern const unsigned char g_model_data[];
//extern const int g_model_len;
//extern const char g_model_name[];

#ifdef SHUFFLENET_MODEL
#define REGISTER_STATIC_OP_RESOLVER(micro_op_resolver)               \
	static int op_registered = 0;                                \
	static tflite::MicroMutableOpResolver<12> micro_op_resolver; \
	if (!op_registered) {                                        \
		micro_op_resolver.AddConcatenation();                \
		micro_op_resolver.AddConv2D();                       \
		micro_op_resolver.AddDepthwiseConv2D();              \
		micro_op_resolver.AddFullyConnected();               \
		micro_op_resolver.AddMaxPool2D();                    \
		micro_op_resolver.AddMean();                         \
		micro_op_resolver.AddPad();                          \
		micro_op_resolver.AddQuantize();                     \
		micro_op_resolver.AddRelu();                         \
		micro_op_resolver.AddReshape();                      \
		micro_op_resolver.AddStridedSlice();                 \
		micro_op_resolver.AddTranspose();                    \
		op_registered = 1;                                   \
	}

#define MODEL_ARENA_BASESIZE (675968)
#define MODEL_ARENA_SIZE (((MODEL_ARENA_BASESIZE/4096)+1)*4096) // 0.7MB

#endif // SHUFFLENET_MODEL


#endif // USE_MICROLITE
#endif // MICROLITE_MODEL_H_
