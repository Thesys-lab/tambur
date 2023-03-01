#include "block_code.hh"
#include "timestamp.hh"

using namespace std;

BlockCode::BlockCode(CodingMatrixInfo codingMatrixInfo,
                     uint16_t delay, pair<uint16_t, uint16_t> v_to_u_ratio,
                     uint16_t packet_size, bool use_bit_matrix, Logger *logger) : codingMatrixInfo_(codingMatrixInfo), delay_(delay), packet_size_(packet_size), timeslot_(0),
                                                                                  use_bit_matrix_(use_bit_matrix), logger_(logger)
{
  assert(v_to_u_ratio.first > 0);
  if ((packet_size % sizeof(long)) != 0)
  {
    throw runtime_error("Invalid packet size\n");
  }
  if (codingMatrixInfo.w != 8 and codingMatrixInfo.w != 16 and
      codingMatrixInfo.w != 24 and codingMatrixInfo.w != 32)
  {
    throw runtime_error("Invalid w\n");
  }
  streamingCodeHelper_.emplace(codingMatrixInfo, delay, v_to_u_ratio);
  BlockCodeFactory factory(codingMatrixInfo, streamingCodeHelper_.value(),
                           delay);
  init_matrices(factory.get_generator_matrix().value(), packet_size,
                codingMatrixInfo);
  initialize_data();
  validate_matrix(v_to_u_ratio);
}

void BlockCode::validate_matrix(pair<uint16_t, uint16_t> v_to_u_ratio)
{
  auto num_frames = num_frames_for_delay(delay_);
  for (uint16_t frame = 0; frame < num_frames; frame++) {
    auto rows = rows_of_frame(frame, delay_, codingMatrixInfo_);
    vector<bool> zeros(num_frames, false);
    for (uint16_t z = frame + 1; z <= frame + delay_; z++) {
      zeros.at(z % num_frames) = true;
    }
    for (uint16_t row = rows.first; row <= rows.second; row++) {
      for (uint16_t col = 0; col < codingMatrixInfo_.n_cols; col++) {
        auto other_frame = frame_of_col(col, delay_, codingMatrixInfo_);
        bool zerod = matrix_->at(row, col) == int(0);
        bool u_conflict = false;
        for (uint16_t j = frame + delay_ + 2; j < frame + num_frames; j++) {
          if ((other_frame % num_frames) == (j % num_frames)) {
            u_conflict = true;
          }
        }
        u_conflict = u_conflict and (v_to_u_ratio.second > 0) and
            (((col % (codingMatrixInfo_.n_cols / num_frames) ) % (v_to_u_ratio.first + v_to_u_ratio.second)) >= v_to_u_ratio.first);
        bool should_zero = zeros.at(other_frame) or u_conflict;
        assert(zerod == should_zero);
      }
    }
  }
}

void BlockCode::init_matrices(const Matrix<int> &matrix,
                              uint16_t packet_size, CodingMatrixInfo codingMatrixInfo)
{
  matrix_.emplace(matrix.max_rows(), matrix.max_cols());
  for (uint16_t row = 0; row < matrix.max_rows(); row++)
  {
    for (uint16_t col = 0; col < matrix.max_cols(); col++)
    {
      matrix_->at(row, col) = matrix.at(row, col);
    }
  }
  if (codingMatrixInfo.w % sizeof(long) != 0)
  {
    throw runtime_error("Invalid w");
  }
  data_matrix_.emplace(codingMatrixInfo.n_cols, packet_size *
                                                    codingMatrixInfo.w);
  parity_matrix_.emplace(codingMatrixInfo.n_rows, packet_size *
                                                      codingMatrixInfo.w);
}

void BlockCode::initialize_data()
{
  erased_ = vector<bool>(codingMatrixInfo_.n_cols + codingMatrixInfo_.n_rows,
                         true);
  for (uint16_t pos = 0; pos < codingMatrixInfo_.n_cols; pos++)
  {
    place_payload(pos, false, vector<char>());
    erased_.at(pos) = false;
  }
  for (uint16_t pos = 0; pos < codingMatrixInfo_.n_rows; pos++)
  {
    place_payload(pos, true, vector<char>());
    erased_.at(codingMatrixInfo_.n_cols + pos) = true;
  }
}

void BlockCode::place_payload(uint16_t row, bool is_parity, vector<char> vals)
{
  (is_parity ? parity_matrix_ : data_matrix_)->fill_row(row, vals);
  erased_.at(row + (is_parity ? codingMatrixInfo_.n_cols : 0)) = false;
}

string BlockCode::get_row(uint16_t row, bool is_parity)
{
  string data;
  auto n_cols = (is_parity ? parity_matrix_ : data_matrix_)->max_cols();
  data.reserve(n_cols);
  for (uint16_t col = 0; col < n_cols; col++)
  {
    auto next_val = (is_parity ? parity_matrix_ : data_matrix_)->at(row, col);
    data.push_back(next_val);
  }
  return data;
}

void BlockCode::update_timeslot(uint16_t timeslot)
{
  if (timeslot != ((timeslot_ + 1) % 65536) and !(timeslot == 0 and timeslot_ == 0))
  {
    throw runtime_error("Invalid update timeslot");
  }
  auto time = timeslot;
  auto cols = cols_of_frame(time, delay_, codingMatrixInfo_);
  for (uint16_t col = cols.first; col <= cols.second; col++)
  {
    if (!erased_.at(col))
    {
      place_payload(col, false, vector<char>());
      erased_.at(col) = true;
    }
  }
  auto rows = rows_of_frame(time, delay_,
                            codingMatrixInfo_);
  for (uint16_t row = rows.first; row <= rows.second; row++)
  {
    if (!erased_.at(codingMatrixInfo_.n_cols + row))
    {
      place_payload(row, true, vector<char>());
      erased_.at(codingMatrixInfo_.n_cols + row) = true;
    }
  }
  timeslot_ = timeslot;
}

void BlockCode::coded_payload(uint16_t first_pos, uint16_t final_pos,
                              const Matrix<char> &puncture_mat)
{
  for (uint16_t row = first_pos; row <= final_pos; row++)
  {
    auto n_cols = puncture_mat.max_cols();
    for (uint16_t col = 0; col < n_cols; col++)
    {
      erased_.at(codingMatrixInfo_.n_cols + row) = false;
      parity_matrix_->at(row, col) = puncture_mat.at(row - first_pos, col);
    }
  }
}

void BlockCode::encode()
{
  uint16_t n_cols = matrix_->max_cols();
  auto rows = rows_of_frame(timeslot_, delay_, codingMatrixInfo_);
  uint16_t n_rows = (rows.second - rows.first + 1);
  vector<bool> retained_frame(num_frames_for_delay(delay_), false);
  retained_frame.at(timeslot_ % num_frames_for_delay(delay_)) = true;
  if (codingMatrixInfo_.w != 8 && logger_ != nullptr)
  {
    logger_->begin_function("encode");
  }
  auto dim = get_puncture_dim(delay_, parity_matrix_->max_rows(), retained_frame);
  optional<Matrix<char>> p;
  p.emplace(dim, parity_matrix_->max_cols());
  get_puncture<char>(codingMatrixInfo_, delay_, parity_matrix_, p.value(),
                     retained_frame);
  int *matrix = new int[n_cols * n_rows];
  int *matrix_copy = matrix;
  uint16_t index = 0;
  for (uint16_t row = 0; row < n_rows; row++)
  {
    for (uint16_t col = 0; col < n_cols; col++)
    {
      const auto val = matrix_->at(rows.first + row, col);
      matrix[index++] = val;
    }
  }
  if (use_bit_matrix_)
  {
    int *encode_bit_mat = jerasure_matrix_to_bitmatrix(n_cols,
                                                       n_rows, codingMatrixInfo_.w, matrix);
    jerasure_bitmatrix_encode(n_cols, dim, codingMatrixInfo_.w, encode_bit_mat,
                              data_matrix_->data(), p->data(),
                              codingMatrixInfo_.w * packet_size_, packet_size_);
    free(encode_bit_mat);
  }
  else
  {
    jerasure_matrix_encode(codingMatrixInfo_.n_cols, dim, codingMatrixInfo_.w,
                           matrix, data_matrix_->data(), p->data(),
                           packet_size_ * codingMatrixInfo_.w);
  }
  coded_payload(rows.first, rows.second, p.value());
  if (codingMatrixInfo_.w != 8 && logger_ != nullptr)
  {
    logger_->end_function();
  }
  delete[] matrix_copy;
}

bool BlockCode::can_recover()
{
  auto retain = streamingCodeHelper_->get_recoverable_data(erased_, timeslot_,
                                                           true);
  for (auto el : retain.first)
  {
    if (el)
    {
      return true;
    }
  }
  return false;
}

vector<bool> BlockCode::decode()
{
  vector<bool> recoverable_data, retained_parities;
  tie(recoverable_data, retained_parities) = streamingCodeHelper_->get_recoverable_data(erased_, timeslot_);
  vector<int> erased_indices;
  uint16_t num_lost = 0;
  for (uint16_t j = 0; j < recoverable_data.size(); j++)
  {
    if (recoverable_data.at(j))
    {
      erased_indices.push_back(int(j));
      num_lost++;
    }
  }
  if (erased_indices.size() == 0)
  {
    return vector<bool>(recoverable_data.size(), false);
  }
  uint16_t num_retained = 0;
  uint16_t num_kept = 0;
  vector<bool> kept_frame_parities(num_frames_for_delay(delay_), false);
  for (uint16_t j = 0; j < retained_parities.size(); j++)
  {
    auto other_frame = frame_of_row(j, delay_, codingMatrixInfo_);
    if (retained_parities.at(j))
    {
      if (!kept_frame_parities.at(other_frame))
      {
        num_kept++;
      }
      kept_frame_parities.at(other_frame) = true;
    }
  }
  uint16_t ind = 0;
  for (uint16_t frame = 0; frame < num_frames_for_delay(delay_); frame++)
  {
    if (!kept_frame_parities.at(frame))
    {
      continue;
    }
    auto rows = rows_of_frame(frame, delay_, codingMatrixInfo_);
    for (uint16_t row = rows.first; row <= rows.second; row++)
    {
      if (!retained_parities.at(row))
      {
        erased_indices.push_back(int(recoverable_data.size() + ind));
      }
      else
      {
        num_retained += 1;
      }
      ind++;
    }
  }
  int *erased = new int[erased_indices.size() + 1]; 
  for (uint16_t pos = 0; pos < erased_indices.size(); pos++)
  {
    erased[pos] = erased_indices.at(pos);
  }
  erased[erased_indices.size()] = -1;
  auto dim = get_puncture_dim(delay_, matrix_->max_rows(), kept_frame_parities);
  if (dim == 0)
  {
    delete[] erased;
    return vector<bool>(recoverable_data.size(), false);
  }
  Matrix<int> p(dim, matrix_->max_cols());
  get_puncture<int>(codingMatrixInfo_, delay_, matrix_, p,
                    kept_frame_parities);
  Matrix<char> punc(get_puncture_dim(delay_, parity_matrix_->max_rows(), kept_frame_parities),
                    parity_matrix_->max_cols());
  get_puncture<char>(codingMatrixInfo_, delay_, parity_matrix_,
                     punc, kept_frame_parities);
  uint16_t n_cols = p.max_cols();
  uint16_t n_rows = p.max_rows();
  int *decode_mat = new int[n_rows * n_cols];
  uint16_t index = 0;
  for (uint16_t row = 0; row < n_rows; row++)
  {
    for (uint16_t col = 0; col < n_cols; col++)
    {
      decode_mat[index++] = p.at(row, col);
    }
  }
  int decoded;
  auto decode_start_time = timestamp_us();
  if (use_bit_matrix_)
  {
    int *decode_bit_mat = jerasure_matrix_to_bitmatrix(n_cols,
                                                       n_rows, codingMatrixInfo_.w, decode_mat);
    decoded = jerasure_bitmatrix_decode(codingMatrixInfo_.n_cols,
                                        dim, codingMatrixInfo_.w, decode_bit_mat,
                                        0, erased, data_matrix_->data(), punc.data(),
                                        codingMatrixInfo_.w * packet_size_, packet_size_);
    free(decode_bit_mat);
  }
  else
  {
    decoded = jerasure_matrix_decode(codingMatrixInfo_.n_cols,
                                     dim, codingMatrixInfo_.w, decode_mat,
                                     0, erased, data_matrix_->data(), punc.data(), packet_size_ * codingMatrixInfo_.w);
  }
  delete[] decode_mat;
  if (decoded != 0) {
    delete[] erased;
    return vector<bool>(recoverable_data.size(), false);
  }
  auto decode_end_time = timestamp_us();
  bool decode_this_frame = false;
  bool decode_part_of_other_frame = false;
  auto num_frames = num_frames_for_delay(delay_);
  for (auto el : erased_indices)
  {
    if (el < int(matrix_->max_cols()))
    {
      auto recover_part_frame = frame_of_col(el, delay_, codingMatrixInfo_);
      if ((recover_part_frame % num_frames) == timeslot_)
      {
        decode_this_frame = true;
      }
      else
      {
        decode_part_of_other_frame = true;
      }
      erased_.at(el) = false;
    }
  }
  bool decode_only_this_frame = decode_this_frame and
                                !decode_part_of_other_frame;
  bool decode_more_than_just_this_frame = !decode_only_this_frame and
                                          decode_part_of_other_frame;

  if (logger_ != nullptr)
  {
    logger_->log_value("decode_time", decode_end_time - decode_start_time, decode_only_this_frame, decode_more_than_just_this_frame, timeslot_);
  }
  delete[] erased;
  return recoverable_data;
}

void BlockCode::pad(uint16_t frame_num, uint16_t num_data_used)
{
  auto endpoints = cols_of_frame(frame_num, delay_, codingMatrixInfo_);
  for (uint16_t pos = endpoints.first + num_data_used; pos <= endpoints.second;
       pos++)
  {
    erased_.at(pos) = false;
  }
}
