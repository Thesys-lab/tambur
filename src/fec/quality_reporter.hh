#ifndef QUALITY_REPORTER_HH
#define QUALITY_REPORTER_HH
#include <deque>

#include "model.hh"
#include "frame.hh"
#include "feedback_datagram.hh"
#include "wrap_helpers.hh"
#include "loss_metric_computer.hh"
#include "loss_computer.hh"
#include "quality_report_generator.hh"

class QualityReporter
{
public:
  QualityReporter(Model * model, InputConverter * inputConverter,
      QualityReportGenerator * qualityReportGenerator)
      : qr_(0), model_(model), inputConverter_(inputConverter),
      qualityReportGenerator_(qualityReportGenerator) {};

  void receive_frame(const Frame & frame);

  uint8_t get_quality_report(LossMetrics lossMetrics);

  FeedbackDatagram generate_quality_report();

private:
  uint8_t qr_;
  Model * model_;
  InputConverter * inputConverter_;
  QualityReportGenerator * qualityReportGenerator_;
  std::deque<Frame> frames_;
};

#endif /* QUALITY_REPORTER_HH */
