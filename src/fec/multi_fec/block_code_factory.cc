#include "block_code_factory.hh"

using namespace std;

BlockCodeFactory::BlockCodeFactory(CodingMatrixInfo codingMatrixInfo,
                                   StreamingCodeHelper streamingCodeHelper, uint16_t delay)
{
  uint16_t n_rows, n_cols, w;
  tie(n_rows, n_cols, w) = {codingMatrixInfo.n_rows, codingMatrixInfo.n_cols,
                            codingMatrixInfo.w};
  assert(n_rows % (num_frames_for_delay(delay)) == 0);
  assert(n_cols % (num_frames_for_delay(delay)) == 0);
  matrix_.emplace(n_rows, n_cols);
  vector<vector<bool>> zero_streaming(n_rows, vector<bool>(n_cols, false));
  for (uint16_t row = 0; row < n_rows; row++)
  {
    for (uint16_t col = 0; col < n_cols; col++)
    {
      Position position{row, col};
      if (streamingCodeHelper.zero_by_frame_u(position, codingMatrixInfo))
      {
        zero_streaming.at(row).at(col) = true;
      }
    }
  }
  set_cauchy_matrix(matrix_.value(), codingMatrixInfo, delay, zero_streaming);
}
