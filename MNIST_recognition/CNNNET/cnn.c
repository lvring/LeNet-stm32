#include "cnn.h"

static q7_t conv1_wt[CONV1_IN_CH*CONV1_KER_DIM*CONV1_KER_DIM*CONV1_OUT_CH] = CONV1_WT;
static q7_t conv1_bias[CONV1_OUT_CH] = CONV1_BIAS;

static q7_t conv2_wt[CONV2_IN_CH*CONV2_KER_DIM*CONV2_KER_DIM*CONV2_OUT_CH] = CONV2_WT;
static q7_t conv2_bias[CONV2_OUT_CH] = CONV2_BIAS;

static q7_t ip1_wt[IP1_IN_DIM*IP1_OUT_DIM] = IP1_WT;
static q7_t ip1_bias[IP1_OUT_DIM] = IP1_BIAS;

static q7_t ip2_wt[IP2_IN_DIM*IP2_OUT_DIM] = IP2_WT;
static q7_t ip2_bias[IP2_OUT_DIM] = IP2_BIAS;

int input_data[DATA_OUT_CH*DATA_OUT_DIM*DATA_OUT_DIM] = IMG_DATA;
q7_t output_data[IP2_OUT_DIM];

q7_t col_buffer[3200];
q7_t scratch_buffer[14400];

q7_t output[IP2_OUT_DIM];


uint8_t cnn_net(uint8_t (*inputimage)[220])
{
	q7_t* buffer1 = scratch_buffer;
  q7_t* buffer2 = buffer1 + 11520;
	q7_t image[28*28];
	
	
//	  /* input pre-processing */
  int mean_data = 33;
  unsigned int scale_data = 1;
	int x_ratio = (int)((220<<8)/28)+1;
  int y_ratio = (int)((220<<8)/28)+1; 
	int sx_prev=0;
	//int sy_prev;
	int temp_p;
	int count_p;
	for(int y=0,i=0;y<28;y++)
	{
		int sy = (y*y_ratio)>>8;
		sx_prev = 0;
		for(int x = 0;x<28;x++,i++)
		{
			int sx = (x*x_ratio)>>8;
			count_p =0;
			for (int k=sx_prev;k<sx;k++)
			{
				if((int)inputimage[k][sy]==255)
				{	
					count_p ++;
				}
				if(count_p >= (int)((sx-sx_prev)/2))
				{
					temp_p = 255;
				}
				else
				{
					temp_p = 0;
				}
			}
			sx_prev = sx;
			int p = temp_p;
			//int p = (int)inputimage[sx][sy];
			image[i] =  (q7_t)__SSAT((((p - (int) mean_data)<<7) + (1<<(scale_data-1))) >> scale_data, 8);
		}
	}	
//  for (int i=0;i<28*28; i++) {
//    image[i] =   (q7_t)__SSAT((((input_data[i] - (int) mean_data)<<7) + (1<<(scale_data-1))) >> scale_data, 8);
//  }
	

  arm_convolve_HWC_q7_basic(image, CONV1_IN_DIM, CONV1_IN_CH, conv1_wt, CONV1_OUT_CH, CONV1_KER_DIM, CONV1_PAD, CONV1_STRIDE, conv1_bias, CONV1_BIAS_LSHIFT, CONV1_OUT_RSHIFT, buffer1, CONV1_OUT_DIM, (q15_t*)col_buffer, NULL);
  arm_maxpool_q7_HWC(buffer1, POOL1_IN_DIM, POOL1_IN_CH, POOL1_KER_DIM, POOL1_PAD, POOL1_STRIDE, POOL1_OUT_DIM, col_buffer, buffer2);
  arm_convolve_HWC_q7_fast(buffer2, CONV2_IN_DIM, CONV2_IN_CH, conv2_wt, CONV2_OUT_CH, CONV2_KER_DIM, CONV2_PAD, CONV2_STRIDE, conv2_bias, CONV2_BIAS_LSHIFT, CONV2_OUT_RSHIFT, buffer1, CONV2_OUT_DIM, (q15_t*)col_buffer, NULL);
  arm_maxpool_q7_HWC(buffer1, POOL2_IN_DIM, POOL2_IN_CH, POOL2_KER_DIM, POOL2_PAD, POOL2_STRIDE, POOL2_OUT_DIM, col_buffer, buffer2);
  arm_fully_connected_q7_opt(buffer2, ip1_wt, IP1_IN_DIM, IP1_OUT_DIM, IP1_BIAS_LSHIFT, IP1_OUT_RSHIFT, ip1_bias, buffer1, (q15_t*)col_buffer);
  arm_relu_q7(buffer1, RELU1_OUT_DIM*RELU1_OUT_DIM*RELU1_OUT_CH);
  arm_fully_connected_q7_opt(buffer1, ip2_wt, IP2_IN_DIM, IP2_OUT_DIM, IP2_BIAS_LSHIFT, IP2_OUT_RSHIFT, ip2_bias, output_data, (q15_t*)col_buffer);
	
	
	arm_softmax_q7(output_data, 10, output);
	uint8_t max = 0;
	for (int i = 0; i < 10; i++)
	{
		if(output[max]<output[i])
		{
			max=i;
		}
	}
	return (max+'0');
}



