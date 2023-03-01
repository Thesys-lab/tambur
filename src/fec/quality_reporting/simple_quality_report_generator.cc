#include "simple_quality_report_generator.hh"

uint8_t SimpleQualityReportGenerator::get_quality_report(LossMetrics
    lossMetrics)
{
  if (lossMetrics.loss_fraction > 0 and num_qrs_non_reduce_) {
    return max_qr_;
  }
  return 0;
}
