#ifndef BLOCK_CODE_FACTORY
#define BLOCK_CODE_FACTORY

#include <optional>

#include "coding_matrix_info.hh"
#include "multi_frame_fec_helpers.hh"
#include "streaming_code_helper.hh"

class BlockCodeFactory
{
public:
  BlockCodeFactory(CodingMatrixInfo codingMatrixInfo,
      StreamingCodeHelper streamingCodeHelper, uint16_t delay);

  std::optional<Matrix<int>> & get_generator_matrix() {
    return matrix_;
  }

private:
  std::optional<Matrix<int>> matrix_;
};

#endif /* BLOCK_CODE_FACTORY */
