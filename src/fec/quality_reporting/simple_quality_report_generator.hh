#ifndef SIMPLE_QUALITY_REPORT_GENERATOR_HH
#define SIMPLE_QUALITY_REPORT_GENERATOR_HH

#include "quality_report_generator.hh"

class SimpleQualityReportGenerator : public QualityReportGenerator
{
public:
  SimpleQualityReportGenerator(uint8_t num_qrs_non_reduce, uint8_t max_qr) :
      num_qrs_non_reduce_(num_qrs_non_reduce), max_qr_(max_qr) {};

  uint8_t get_quality_report(LossMetrics lossMetrics) override;

private:
  uint8_t num_qrs_non_reduce_;
  uint8_t max_qr_;
};

#endif /* SIMPLE_QUALITY_REPORT_GENERATOR_HH */
