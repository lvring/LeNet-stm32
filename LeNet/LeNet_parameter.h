

#define RELU1_OUT_DIM 1
#define DATA_OUT_CH 1
#define DATA_OUT_DIM 28

#define CONV1_IN_DIM 28
#define CONV1_IN_CH 1
#define CONV1_KER_DIM 5
#define CONV1_PAD 0
#define CONV1_STRIDE 1
#define CONV1_OUT_CH 20
#define CONV1_OUT_DIM 24

#define POOL1_IN_DIM 24
#define POOL1_IN_CH 20
#define POOL1_KER_DIM 2
#define POOL1_STRIDE 2
#define POOL1_PAD 0
#define POOL1_OUT_DIM 12

#define CONV2_IN_DIM 12
#define CONV2_IN_CH 20
#define CONV2_KER_DIM 5
#define CONV2_PAD 0
#define CONV2_STRIDE 1
#define CONV2_OUT_CH 32
#define CONV2_OUT_DIM 8

#define POOL2_IN_DIM 8
#define POOL2_IN_CH 32
#define POOL2_KER_DIM 2
#define POOL2_STRIDE 2
#define POOL2_PAD 0
#define POOL2_OUT_DIM 4

#define IP1_IN_DIM 512
#define IP1_OUT_DIM 32

#define RELU1_OUT_CH 32
#define IP2_IN_DIM 32
#define IP2_OUT_DIM 10

#define CONV1_BIAS_LSHIFT 4
#define CONV1_OUT_RSHIFT 9
#define CONV2_BIAS_LSHIFT 5
#define CONV2_OUT_RSHIFT 10
#define IP1_BIAS_LSHIFT 2
#define IP1_OUT_RSHIFT 9
#define IP2_BIAS_LSHIFT 0
#define IP2_OUT_RSHIFT 7
