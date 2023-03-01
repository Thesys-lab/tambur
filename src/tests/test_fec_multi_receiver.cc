#include <chrono>
#include <iostream>
#include "torch/script.h"

#include "helpers.hh"
#include "helpersQR.hh"

using namespace std;

bool test_frame_expired()
{
  Model * model;
  InputConverter * inputConverter;
  auto qualityReporter = get_QualityReporter(model, inputConverter);
  for (uint16_t tau = 0; tau < 4; tau++) {
    auto num_frames = num_frames_for_delay(tau);
    auto receiver = get_FECMultiReceiver(tau, num_frames, qualityReporter);
    for (uint16_t old = 0; old < 2 * num_frames; old++) {
      for (uint16_t new_frame = old; new_frame < old + num_frames; new_frame++) {
        assert(!receiver.frame_expired(new_frame, old));
      }
      for (uint16_t new_frame = old + num_frames; new_frame < old +
          num_frames * 3; new_frame++) {
        assert(receiver.frame_expired(new_frame, old));
      }
    }
  }
  return true;
}

int main(int argc, char * argv[])
{
  assert(test_frame_expired());
  printf("test_frame_expired: passed\n");
}
