#ifndef QUALITY_REPORT_GENERATOR_HH
#define QUALITY_REPORT_GENERATOR_HH

#include "loss_metrics.hh"

class QualityReportGenerator
{
public:
  virtual uint8_t get_quality_report(LossMetrics lossMetrics) = 0;
};

#endif /* QUALITY_REPORT_GENERATOR_HH */
