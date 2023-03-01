#ifndef HELPERS_QR_HH
#define HELPERS_QR_HH
#include "quality_reporter.hh"
#include "helpers.hh"
#include "input_converter.hh"
#include "loss_info.hh"
#include "fec_multi_receiver.hh"
#include "simple_quality_report_generator.hh"

QualityReporter get_QualityReporter(Model * model,
    InputConverter * inputConverter, uint8_t num_qrs_no_reduce = 2,
    uint8_t max_qr = 1);

FECMultiReceiver get_FECMultiReceiver(uint16_t tau, uint16_t memory, QualityReporter & qualityReporter);

static std::vector<float> test_means = { 0.11727489854735787,
    0.11790285228930882, 0.11829486965422617, 0.08946478778985126,
    0.09017111599844223, 0.09088586181276088, 0.010302075313209563,
    0.010264637387639254, 0.010192042220665306, 0.021567542475964034,
    0.021645348116416076, 0.02141317931469647, 0.7864755988944132,
    0.784547877656634, 0.7759893351403132, 0.33350359449246864,
    0.3357564994088231, 0.33703082043481136, 0.031057223939657826,
    0.031125576987297763, 0.031000126001923204, 0.05846550688098845,
    0.058777734291188204, 0.05878465371300759, 0.044013883160094504,
    0.044360097384214654, 0.044720861651988145, 38.718023596035415,
    38.7807214901715, 38.84926656349525, 9.0040205642031, 8.994461778598213,
    8.9900029528024, 0.25530457481287044, 0.2553485992577553,
    0.25524287762251896, 5.918885073575852, 6.152737950910715,
    6.398206157576461 };
static std::vector<float> test_vars = { 0.10352149671806479,
    0.10400176971135426, 0.10430119346771578, 0.07272183787659424,
    0.07325148881585104, 0.0737983164215071, 0.0013611662378156938,
    0.0013497374178720678, 0.0013226271335397737, 0.00994447989051198,
    0.010003817657190368, 0.009807414866704197, 252.53359654997448,
    251.543204834042, 247.45832127934395, 0.9304342160325443,
    0.9343967258264945, 0.9347152696545886, 0.008817290791213009,
    0.008835690500056951, 0.008725971120878516, 0.027958993006257336,
    0.028094071049168196, 0.02799871681697071, 0.023937229118422075,
    0.024098709677406054, 0.024276363974497832, 2946.30985786643,
    2936.228696240609, 2925.1094784724464, 105.35796754388265,
    105.16423818210681, 105.02264050583312, 0.1736868922396258,
    0.17363445667370103, 0.1734759931624557, 5.482814121115941,
    4.251362767655765, 2.843518985557333 };

InputConverter get_InputConverter(uint16_t num_indicators,
    float epsilon, std::string means_fname,
    std::string vars_fname);

static std::vector<LossInfo> lossInfosForTests = std::vector<LossInfo>{
    LossInfo{ {0, 2, 5, 8, 28, 31, 34, 37, 57, 58, 59, 60,
    61, 62}, {1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 1, 1, 1, 0,
    0, 0, 1, 0, 0, 0} },
    LossInfo{ {0, 2, 5, 8}, {1, 1, 0, 0, 0, 1, 0, 1, 0,
    0, 0}, {1, 0, 1, 0} },
    LossInfo{ {0, 2, 5, 8}, {0, 0, 0, 0, 0, 1, 0, 1, 0,
    0, 0}, {0, 0, 1, 0} },
    LossInfo{ {0, 2, 5, 8}, {1, 1, 0, 0, 0, 1, 0, 1, 0,
    1, 1}, {1, 0, 1, 1} } };

#endif /* HELPERS_QR_HH */