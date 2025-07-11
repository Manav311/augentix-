/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <cstdio>
#include <ctime>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

// This is an example that is minimal to read a model
// from disk and perform inference. There is no data being loaded
// that is up to you to add As a user.
//
// NOTE: Do not add any dependencies to this that cannot be built with
// the minimal makefile. This example must remain trivial to build with
// the minimal build tool.
//
// Usage: minimal <tflite model>

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

int main(int argc, char* argv[]) {
  int dim[2] = {};
  char *verbose = getenv("VERBOSE");
  char *num = getenv("NUM");
  int v = (verbose==0) ? 0 : atoi(verbose);
  int n = (num==0) ? 10 : atoi(num);
  if (!(argc == 2 || argc == 4)) {
    fprintf(stderr, "minimal <tflite model>\n");
    return 1;
  }
  if (argc == 4) {
    dim[0] = atoi(argv[2]);
    dim[1] = atoi(argv[3]);
  }
  const char* filename = argv[1];

  // Load model
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromFile(filename);
  TFLITE_MINIMAL_CHECK(model != nullptr);

  // Build the interpreter with the InterpreterBuilder.
  // Note: all Interpreters should be built with the InterpreterBuilder,
  // which allocates memory for the Interpreter and does various set up
  // tasks so that the Interpreter can read the provided model.
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);
  TFLITE_MINIMAL_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  if (argc == 4) {
    int ind = interpreter->inputs()[0];
    int chn = interpreter->tensor(ind)->dims->data[3];
    std::vector<int> in_dim{1, dim[0], dim[1], chn};
    interpreter->ResizeInputTensor(ind, in_dim);
  }
  TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
  printf("=== Pre-invoke Interpreter State ===\n");
  if (v) tflite::PrintInterpreterState(interpreter.get());

  // Fill input buffers
  // TODO(user): Insert code to fill input tensors.
  // Note: The buffer of the input tensor with index `i` of type T can
  // be accessed with `T* input = interpreter->typed_input_tensor<T>(i);`

  // Run inference
  TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  printf("\n\n=== Post-invoke Interpreter State ===\n");
  if (v) tflite::PrintInterpreterState(interpreter.get());

  for (int i = 0; i < n; i++) {
    struct timespec s, e;
    clock_gettime(CLOCK_MONOTONIC_RAW, &s);
    interpreter->Invoke();
    clock_gettime(CLOCK_MONOTONIC_RAW, &e);
    float delta = e.tv_sec - s.tv_sec + (float)(e.tv_nsec - s.tv_nsec)/1e9;
    printf("\n\n== Invoke interpreter time #%d :%.8f===\n", i, delta);
  }

  // Read output buffers
  // TODO(user): Insert getting data out code.
  // Note: The buffer of the output tensor with index `i` of type T can
  // be accessed with `T* output = interpreter->typed_output_tensor<T>(i);`

  return 0;
}
