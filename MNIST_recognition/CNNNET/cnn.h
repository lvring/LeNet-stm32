#ifndef __cnn_H
#define __cnn_H

#include <stdint.h>
#include <stdio.h>

#include "arm_math.h"
#include "arm_nnfunctions.h"


#include "arm_nnexamples_cifar10_parameter.h"
#include "arm_nnexamples_cifar10_weights.h"
#include "arm_nnexamples_cifar10_inputs.h"



uint8_t cnn_net(uint8_t (*inputimage)[220]);
#endif
