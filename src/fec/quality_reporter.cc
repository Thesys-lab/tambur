#include <stdexcept>
#include "quality_reporter.hh"
#include <algorithm>

void QualityReporter::receive_frame(const Frame & frame)
{
  frames_.emplace_back(frame);
  if (frames_are_out_of_order(frames_)) {
    throw std::runtime_error("Frames should be given to QualityReporter in order");
  }
}

uint8_t QualityReporter::get_quality_report(LossMetrics lossMetrics)
{
  return qualityReportGenerator_->get_quality_report(lossMetrics);
}

FeedbackDatagram QualityReporter::generate_quality_report()
{
  LossInfo lossInfo = LossComputer(frames_).get_loss_info();
  LossMetricComputer lossMetricComputer(lossInfo, qr_, 1, 3, 2,  3 );
  auto metrics = lossMetricComputer.compute_loss_metrics();
  qr_ = get_quality_report(metrics);
  metrics.qr = qr_;
  if ((qr_ > 0) and (model_ != nullptr) and (inputConverter_ != nullptr)) {
    inputConverter_->update_all_loss_metrics(metrics);
    qr_ = model_->get_quality_report(*inputConverter_, metrics);
  }
  FeedbackDatagram feedbackDatagram { qr_ };
  frames_.clear();
  return feedbackDatagram;
}
