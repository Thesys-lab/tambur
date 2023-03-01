#include <random>
#include <cassert>
#include <cmath>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <iterator>
#include <string>
#include <vector>

#include "helpersQR.hh"

QualityReporter get_QualityReporter(Model * model,
    InputConverter * inputConverter, uint8_t num_qrs_no_reduce, uint8_t max_qr) {
  static QualityReportGenerator * qualityReportGenerator;
  qualityReportGenerator = new SimpleQualityReportGenerator(num_qrs_no_reduce, max_qr);

  QualityReporter qualityReporter(model, inputConverter, qualityReportGenerator);
  return qualityReporter;
}

FECMultiReceiver get_FECMultiReceiver(uint16_t tau, uint16_t memory, QualityReporter & qualityReporter)
{
  uint16_t w = 32;
  uint16_t packet_size = 8;
  uint16_t stripe_size = packet_size * w;
  uint16_t max_data_stripes_per_frame = 100;
  uint16_t max_fec_stripes_per_frame = 50;
  uint16_t num_frames = num_frames_for_delay(tau);
  uint16_t n_cols = num_frames;
  uint16_t n_rows = 256;
  while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
  static CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
  static CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
      max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
  static BlockCode blockCodeHeader(codingMatrixInfoHeader, tau,
      pair<uint16_t, uint16_t>{1, 0 }, 8, false);
  static MultiFECHeaderCode headerCode(&blockCodeHeader, tau,
      codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
  static BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
      pair<uint16_t, uint16_t>{1, 0 }, packet_size, false);
  static StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
  static Logger logger = get_Logger();
  FECMultiReceiver fECMultiReceiver(&code, tau, memory,
      &headerCode, &qualityReporter, &logger, 0);
  return fECMultiReceiver;
}