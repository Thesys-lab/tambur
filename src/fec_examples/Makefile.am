AM_CPPFLAGS = $(CXX17_FLAGS) -I$(srcdir)/../util -I$(srcdir)/../fec \
  -I$(srcdir)/../fec/reed_solomon -I$(srcdir)/../fec/quality_reporting \
  -I$(srcdir)/../fec/streaming_code -I$(srcdir)/../fec/logging \
  -I$(srcdir)/../fec/multi_fec \
  $(erasurecode_CFLAGS) -isystem$(srcdir)/../../third_party/libtorch/include \
  -I$(srcdir)/../../third_party/Jerasure-master/include \
  -I$(srcdir)/../../third_party/Jerasure-master/src \
  -I$(srcdir)/../../third_party/gf-complete-master/include \
  -I$(srcdir)/../../third_party/gf-complete-master/src \
  -I$(srcdir)/../../third_party/maxflow-master \
  -I$(srcdir)/../../third_party/maxflow-master/maxflow \
  -I$(srcdir)/../../third_party

AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = sender_trace get_ge_params multi_receiver fixed_ge_params fixed_frame_ge_params

sender_trace_SOURCES = sender_trace.cc \
  ge_channel.hh ge_channel.cc \
  ../fec/packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code.hh ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/multi_fec/block_code_factory.hh ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_helper.hh ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../../third_party/gf-complete-master/include/gf_rand.h \
  ../../third_party/gf-complete-master/src/gf_rand.c \
  ../../third_party/Jerasure-master/include/galois.h \
  ../../third_party/Jerasure-master/include/cauchy.h \
  ../../third_party/Jerasure-master/include/jerasure.h \
  ../../third_party/Jerasure-master/include/reed_sol.h \
  ../../third_party/Jerasure-master/include/liberation.h \
  ../../third_party/Jerasure-master/src/galois.c \
  ../../third_party/Jerasure-master/src/cauchy.c \
  ../../third_party/Jerasure-master/src/jerasure.c \
  ../../third_party/Jerasure-master/src/reed_sol.c \
  ../../third_party/Jerasure-master/src/liberation.c \
  ../../third_party/maxflow-master/maxflow.h \
  ../../third_party/maxflow-master/maxflow.cpp \
  ../../third_party/maxflow-master/maxflow/block.h \
  ../../third_party/maxflow-master/maxflow/graph.h \
  ../../third_party/maxflow-master/maxflow/graph.cpp \
  ../fec/fec_datagram.hh ../fec/fec_datagram.cc ../fec/header_code.hh \
  ../fec/logger.hh ../fec/logger.cc ../fec/metric_logger.hh \
  ../fec/logging/timing_logger.hh ../fec/logging/timing_logger.cc \
  ./fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc ../fec/logging/frame_log.h \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
  ../fec/fec_sender.hh ../fec/fec_sender.cc \
  ../../third_party/json.hpp
sender_trace_LDADD = ../util/libutil.a

get_ge_params_SOURCES = get_ge_params.cc ../fec/frame.hh ../fec/frame.cc \
  ../fec/quality_reporting/loss_metrics.hh ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../../third_party/json.hpp strategies/strategy.hh \
  strategies/guard_space_match_fixed_loss.hh
get_ge_params_LDADD = ../util/libutil.a

multi_receiver_SOURCES = multi_receiver.cc \
  ../fec/packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code.hh ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/multi_fec/block_code_factory.hh ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_helper.hh ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../../third_party/gf-complete-master/include/gf_rand.h \
  ../../third_party/gf-complete-master/src/gf_rand.c \
  ../../third_party/Jerasure-master/include/galois.h \
  ../../third_party/Jerasure-master/include/cauchy.h \
  ../../third_party/Jerasure-master/include/jerasure.h \
  ../../third_party/Jerasure-master/include/reed_sol.h \
  ../../third_party/Jerasure-master/include/liberation.h \
  ../../third_party/Jerasure-master/src/galois.c \
  ../../third_party/Jerasure-master/src/cauchy.c \
  ../../third_party/Jerasure-master/src/jerasure.c \
  ../../third_party/Jerasure-master/src/reed_sol.c \
  ../../third_party/Jerasure-master/src/liberation.c \
  ../../third_party/maxflow-master/maxflow.h \
  ../../third_party/maxflow-master/maxflow.cpp \
  ../../third_party/maxflow-master/maxflow/block.h \
  ../../third_party/maxflow-master/maxflow/graph.h \
  ../../third_party/maxflow-master/maxflow/graph.cpp \
  ../fec/fec_datagram.cc ../fec/fec_datagram.hh ../fec/header_code.hh \
  ../fec/code.hh \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
  ../fec/frame.hh ../fec/frame.cc \
  ../fec/quality_reporter.hh ../fec/quality_reporter.cc \
  ../fec/quality_report_generator.hh \
  ../fec/quality_reporting/simple_quality_report_generator.hh \
  ../fec/quality_reporting/simple_quality_report_generator.cc \
  ../fec/quality_reporting/model.hh ../fec/quality_reporting/model.cc \
  ../fec/quality_reporting/input_converter.hh \
  ../fec/quality_reporting/input_converter.cc \
  ../fec/quality_reporting/loss_metrics.hh \
  ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc \
  ../fec/logger.hh \
  ../fec/logger.cc ../fec/metric_logger.hh ../fec/logging/timing_logger.hh \
  ../fec/logging/timing_logger.cc ../fec/logging/frame_logger.hh \
  ../fec/logging/frame_logger.cc ../fec/logging/frame_log.hh \
  ../fec/fec_multi_receiver.hh ../fec/fec_multi_receiver.cc \
  ../../third_party/json.hpp
multi_receiver_LDADD = ../util/libutil.a \
  -ltorch -ltorch_cpu -lc10 -lmkldnn
multi_receiver_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'

fixed_ge_params_SOURCES = fixed_ge_params.cc ../fec/frame.hh ../fec/frame.cc \
  ../fec/quality_reporting/loss_metrics.hh ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../../third_party/json.hpp
fixed_ge_params_LDADD = ../util/libutil.a \
  -ltorch -ltorch_cpu -lc10 -lmkldnn
fixed_ge_params_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'

fixed_frame_ge_params_SOURCES = fixed_frame_ge_params.cc ../fec/frame.hh ../fec/frame.cc \
  ../fec/quality_reporting/loss_metrics.hh ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../../third_party/json.hpp
fixed_frame_ge_params_LDADD = ../util/libutil.a \
  -ltorch -ltorch_cpu -lc10 -lmkldnn
fixed_frame_ge_params_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'
