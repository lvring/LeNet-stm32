#ifndef __LeNet_H
#define __LeNet_H

#include <stdint.h>
#include <stdio.h>

#include "arm_math.h"
#include "arm_nnfunctions.h"


#include "LeNet_parameter.h"
#include "LeNet_weights.h"
#include "LeNet_inputs.h"



uint8_t cnn_net(void);
#endif
