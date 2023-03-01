#ifndef MATRIX_HH
#define MATRIX_HH

#include <stdexcept>
#include <vector>

template<typename T>
class Matrix
{
public:
  Matrix(const size_t max_rows, const size_t max_cols)
  {
    if (max_rows == 0 or max_cols == 0) {
      throw std::runtime_error("invalid number of rows or columns to allocate");
    }

    max_rows_ = max_rows;
    max_cols_ = max_cols;

    buf_ = new T*[max_rows_];
    for (size_t i = 0; i < max_rows_; i++) {
      buf_[i] = new T[max_cols_];
    }

    rows_ = max_rows;
    cols_ = max_cols;
  }

  ~Matrix()
  {
    for (size_t i = 0; i < max_rows_; i++) {
      delete[] buf_[i];
    }

    delete[] buf_;
  }

  // accessors
  size_t max_rows() const { return max_rows_; }
  size_t max_cols() const { return max_cols_; }
  T ** data() { return buf_; }

  size_t rows() const { return rows_; }
  size_t cols() const { return cols_; }

  // access an element
  T & at(const size_t row, const size_t col)
  {
    bounds_check(row, col);
    return buf_[row][col];
  }

  const T & at(const size_t row, const size_t col) const
  {
    bounds_check(row, col);
    return buf_[row][col];
  }

  // resize the matrix to a size equal to or smaller than the original size
  void resize(const size_t rows, const size_t cols)
  {
    if (rows > max_rows_ or cols > max_cols_) {
      throw std::runtime_error("cannot resize beyond originally allocated size");
    }

    rows_ = rows;
    cols_ = cols;
  }

  // fill a row / a column / all elements with 'val'
  void fill_row(const size_t row, const T & val)
  {
    bounds_check(row, 0);
    for (size_t j = 0; j < cols_; j++) {
      buf_[row][j] = val;
    }
  }

  void fill_col(const size_t col, const T & val)
  {
    bounds_check(0, col);
    for (size_t i = 0; i < rows_; i++) {
      buf_[i][col] = val;
    }
  }

  void fill_row(const size_t row, const std::vector<T> & vals)
  {
    bounds_check(row, 0);
    for (size_t j = 0; j < cols_; j++) {
      buf_[row][j] = j < vals.size() ? vals.at(j) : T(0);
    }
  }

  void fill_col(const size_t col, const std::vector<T> & vals)
  {
    bounds_check(0, col);
    for (size_t i = 0; i < rows_; i++) {
      buf_[i][col] = i < vals.size() ? vals.at(i) : T(0);
    }
  }

  void fill(const T & val)
  {
    for (size_t i = 0; i < rows_; i++) {
      for (size_t j = 0; j < cols_; j++) {
        buf_[i][j] = val;
      }
    }
  }

  // iterators with lambda functions
  template<typename lambda>
  void for_each(const lambda callback)
  {
    for (size_t i = 0; i < rows_; i++) {
      for (size_t j = 0; j < cols_; j++) {
        callback(buf_[i][j]);
      }
    }
  }

  template<typename lambda>
  void enumerate(const lambda callback)
  {
    for (size_t i = 0; i < rows_; i++) {
      for (size_t j = 0; j < cols_; j++) {
        callback(i, j, buf_[i][j]);
      }
    }
  }

  // forbid copying
  Matrix(const Matrix & other) = delete;
  const Matrix & operator=(const Matrix & other) = delete;

  // forbid moving
  Matrix(Matrix && other) = delete;
  Matrix & operator=(Matrix && other) = delete;

private:
  void bounds_check(const size_t row, const size_t col) const
  {
    if (row >= rows_ or col >= cols_) {
      throw std::out_of_range("invalid row or column");
    }
  }

  // allocated size: max_rows_ * max_cols_
  size_t max_rows_;
  size_t max_cols_;
  T ** buf_;

  // effective size: rows_ * cols_
  size_t rows_;
  size_t cols_;
};

#endif /* MATRIX_HH */
