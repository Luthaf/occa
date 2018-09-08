/* The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 David Medina and Tim Warburton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 */
#include <stdlib.h>
#include <stdio.h>

#include <occa.h>

occaJson parseArgs(int argc, const char **argv);

int main(int argc, const char **argv) {
  occaJson args = parseArgs(argc, argv);

  // occaSetDeviceFromString("mode: 'Serial'");
  // occaSetDeviceFromString("mode: 'OpenMP'");
  // occaSetDeviceFromString("mode: 'CUDA'  , device_id: 0");
  // occaSetDeviceFromString("mode: 'OpenCL', platform_id: 0, device_id: 0");
  occaSetDeviceFromString(
    occaJsonGetString(
      occaJsonObjectGet(args,
                        "options/device",
                        occaDefault)
    )
  );

  // Choosing something not divisible by 256
  int i;
  int entries = 10000;
  int block   = 256;
  int blocks  = (entries + block - 1)/block;

  float *vec      = (float*) malloc(entries * sizeof(float));
  float *blockSum = (float*) malloc(blocks  * sizeof(float));

  float sum = 0;

  // Initialize device memory
  for (i = 0; i < entries; ++i) {
    vec[i] = 1;
    sum += vec[i];
  }

  for (i = 0; i < blocks; ++i) {
    blockSum[i] = 0;
  }

  // Allocate memory on the device
  occaMemory o_vec      = occaMalloc(entries * sizeof(float),
                                     NULL, occaDefault);
  occaMemory o_blockSum = occaMalloc(blocks * sizeof(float),
                                     NULL, occaDefault);

  // Pass value of 'block' at kernel compile-time
  occaProperties reductionProps = occaCreateProperties();
  occaPropertiesSet(reductionProps,
                    "defines/block",
                    occaInt(block));

  occaKernel reduction = occaBuildKernel("reduction.okl",
                                         "reduction",
                                         reductionProps);

  // Host -> Device
  occaCopyPtrToMem(o_vec, vec,
                   occaAllBytes, 0, occaDefault);

  occaKernelRun(reduction,
                occaInt(entries), o_vec, o_blockSum);

  // Host <- Device
  occaCopyMemToPtr(blockSum, o_blockSum,
                   occaAllBytes, 0, occaDefault);

  // Finalize the reduction in the host
  for (i = 1; i < blocks; ++i) {
    blockSum[0] += blockSum[i];
  }

  // Validate
  if (blockSum[0] != sum) {
    printf("sum      = %f\n", sum);
    printf("blockSum = %f\n", blockSum[0]);
    printf("Reduction failed\n");
    exit(1);
  }
  else {
    printf("Reduction = %f\n", blockSum[0]);
  }

  // Free host memory
  free(vec);
  free(blockSum);

  occaFree(args);
  occaFree(reductionProps);
  occaFree(reduction);
  occaFree(o_vec);
  occaFree(o_blockSum);

  return 0;
}

occaJson parseArgs(int argc, const char **argv) {
  occaJson args = occaCliParseArgs(
    argc, argv,
    "{"
    "  description: 'Example showing how to use background devices, allowing passing of the device implicitly',"
    "  options: ["
    "    {"
    "      name: 'device',"
    "      shortname: 'd',"
    "      description: 'Device properties (default: \"mode: \\'Serial\\'\")',"
    "      with_arg: true,"
    "      default_value: { mode: 'Serial' },"
    "    },"
    "    {"
    "      name: 'verbose',"
    "      shortname: 'v',"
    "      description: 'Compile kernels in verbose mode',"
    "      default_value: false,"
    "    },"
    "  ],"
    "}"
  );

  occaProperties settings = occaSettings();
  occaPropertiesSet(settings,
                    "kernel/verbose",
                    occaJsonObjectGet(args, "options/verbose", occaBool(0)));

  return args;
}