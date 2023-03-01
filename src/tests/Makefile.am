AM_CPPFLAGS = $(CXX17_FLAGS) -I$(srcdir)/../util -I$(srcdir)/../fec \
  -I$(srcdir)/../fec/reed_solomon -I$(srcdir)/../fec/logging \
  -I$(srcdir)/../fec/quality_reporting -I$(srcdir)/../fec/multi_fec \
  -I$(srcdir)/../fec/streaming_code \
  $(erasurecode_CFLAGS) -isystem$(srcdir)/../../third_party/libtorch/include \
  -I$(srcdir)/../../third_party/Jerasure-master/include \
  -I$(srcdir)/../../third_party/Jerasure-master/src \
  -I$(srcdir)/../../third_party/gf-complete-master/include \
  -I$(srcdir)/../../third_party/gf-complete-master/src \
  -I$(srcdir)/../../third_party/maxflow-master \
  -I$(srcdir)/../../third_party/maxflow-master/maxflow \
  -I$(srcdir)/../../third_party -I$(srcdir)/../fec_examples

AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = test_streaming_code_packetization \
  test_reed_solomon_multi_frame_packetization \
  test_fec_datagram \
  test_multi_fec_header_code \
  test_frame_generator \
  test_frame \
  test_wrap_helpers \
  test_loss_computer \
  test_fec_multi_receiver \
  test_loss_metric_computer \
  test_input_converter \
  test_streaming_code_theoretical \
  test_multi_frame_fec_helpers \
  test_puncture \
  test_streaming_code_helper \
  test_block_code \
  test_end_to_end \
  test_ge \
  test_streaming_code_auxiliary_functions \
  test_streaming_code

test_streaming_code_packetization_SOURCES = test_streaming_code_packetization.cc \
  ../fec/packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc
test_streaming_code_packetization_LDADD = ../util/libutil.a $(gtest_LIBS)

test_reed_solomon_multi_frame_packetization_SOURCES = \
  test_reed_solomon_multi_frame_packetization.cc \
  ../fec/packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc
test_reed_solomon_multi_frame_packetization_LDADD = \
  ../util/libutil.a $(gtest_LIBS)

test_fec_datagram_SOURCES = test_fec_datagram.cc ../fec/fec_datagram.hh \
  ../fec/fec_datagram.cc
test_fec_datagram_LDADD = ../util/libutil.a $(gtest_LIBS)

test_multi_fec_header_code_SOURCES = test_multi_fec_header_code.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/header_code.hh ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_multi_fec_header_code_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_frame_generator_SOURCES = test_frame_generator.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  helpers.hh helpers.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_frame_generator_LDADD = ../util/libutil.a $(gtest_LIBS)

test_frame_SOURCES = test_frame.cc helpers.hh helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  fec_datagram.hh ../fec/fec_datagram.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_frame_LDADD = ../util/libutil.a $(gtest_LIBS)

test_wrap_helpers_SOURCES = test_wrap_helpers.cc  helpers.hh helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  fec_datagram.hh ../fec/fec_datagram.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_wrap_helpers_LDADD = ../util/libutil.a $(gtest_LIBS)

test_loss_computer_SOURCES = test_loss_computer.cc helpers.hh helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  fec_datagram.hh ../fec/fec_datagram.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc
test_loss_computer_LDADD = ../util/libutil.a $(gtest_LIBS)

test_fec_multi_receiver_SOURCES = test_fec_multi_receiver.cc helpers.hh helpers.cc helpersQR.hh helpersQR.cc \
  ../fec/packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/fec_sender.hh ../fec/fec_sender.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
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
  ../fec/fec_multi_receiver.hh ../fec/fec_multi_receiver.cc
test_fec_multi_receiver_LDADD = ../util/libutil.a  \
  -ltorch -ltorch_cpu -lc10 -lmkldnn
test_fec_multi_receiver_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'

test_loss_metric_computer_SOURCES = test_loss_metric_computer.cc \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/frame.hh ../fec/frame.cc \
  ../fec/quality_reporting/loss_metrics.hh \
  ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc
test_loss_metric_computer_LDADD = ../util/libutil.a $(gtest_LIBS)

test_input_converter_SOURCES = test_input_converter.cc \
  helpers.hh helpers.cc ../fec/packetization.hh \
  helpersQR.hh helpersQR.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/fec_sender.hh ../fec/fec_sender.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
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
  ../fec/fec_multi_receiver.hh ../fec/fec_multi_receiver.cc
test_input_converter_LDADD = ../util/libutil.a $(gtest_LIBS) \
  -ltorch -ltorch_cpu -lc10 -lmkldnn
test_input_converter_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'

test_streaming_code_theoretical_SOURCES = test_streaming_code_theoretical.cc \
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
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc
test_streaming_code_theoretical_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_multi_frame_fec_helpers_SOURCES = helpers.hh helpers.cc \
  test_multi_frame_fec_helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_multi_frame_fec_helpers_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_puncture_SOURCES = test_puncture.cc \
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
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc
test_puncture_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_streaming_code_helper_SOURCES = helpers.hh helpers.cc \
  test_streaming_code_helper.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_streaming_code_helper_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_block_code_SOURCES = test_block_code.cc helpers.hh helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_block_code_LDADD = $(gtest_LIBS) ../util/libutil.a -lgf_complete

test_end_to_end_SOURCES = test_end_to_end.cc \
  helpers.hh helpers.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/fec_multi_receiver.hh ../fec/fec_multi_receiver.cc \
  ../fec/quality_reporting/model.hh ../fec/quality_reporting/model.cc \
  ../fec/quality_reporting/input_converter.hh \
  ../fec/quality_reporting/input_converter.cc \
  ../fec/quality_reporting/loss_metrics.hh \
  ../fec/quality_reporting/loss_info.hh \
  ../fec/quality_reporting/loss_metric_computer.hh \
  ../fec/quality_reporting/loss_metric_computer.cc \
  ../fec/quality_reporting/loss_computer.hh \
  ../fec/quality_reporting/loss_computer.cc \
  ../fec/quality_reporter.hh ../fec/quality_reporter.cc \
    ../fec/quality_report_generator.hh \
  ../fec/quality_reporting/simple_quality_report_generator.hh \
  ../fec/quality_reporting/simple_quality_report_generator.cc \
  ../fec/fec_datagram.hh ../fec/fec_datagram.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc \
  ../fec/logging/timing_logger.hh ../fec/logging/timing_logger.cc
test_end_to_end_LDADD = ../util/libutil.a -lgf_complete -ltorch \
  -ltorch_cpu -lc10 -lmkldnn
test_end_to_end_LDFLAGS = -L../../third_party/libtorch/lib \
  '-Wl,-rpath,$$ORIGIN/../../third_party/libtorch/lib'

test_ge_SOURCES = test_ge.cc ../fec_examples/ge_channel.hh \
  ../fec_examples/ge_channel.cc
test_ge_LDADD = $(gtest_LIBS) ../util/libutil.a

test_streaming_code_auxiliary_functions_SOURCES = \
  test_streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  helpers.hh helpers.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_streaming_code_auxiliary_functions_LDADD = ../util/libutil.a $(gtest_LIBS)

test_streaming_code_SOURCES = \
  test_streaming_code.cc \
  ../fec/streaming_code/streaming_code.hh \
  ../fec/streaming_code/streaming_code.cc \
  ../fec/streaming_code/streaming_code_auxiliary_functions.hh \
  ../fec/streaming_code/streaming_code_auxiliary_functions.cc \
  ../fec/streaming_code/streaming_code_packetization.hh \
  ../fec/streaming_code/streaming_code_packetization.cc \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.hh \
  ../fec/reed_solomon/reed_solomon_multi_frame_packetization.cc \
  helpers.hh helpers.cc \
  ../fec/frame_generator.hh ../fec/frame_generator.cc \
  ../fec/frame.hh ../fec/frame.cc ../fec/fec_sender.hh \
  ../fec/fec_sender.cc ../fec/logger.hh ../fec/logger.cc \
  ../fec/feedback_datagram.hh ../fec/feedback_datagram.cc \
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
  ../fec/multi_fec/flow_decode.hh ../fec/multi_fec/flow_decode.cc \
  ../fec/packetization.hh ../fec/multi_fec/coding_matrix_info.hh \
  ../fec/multi_fec/position.hh \
  ../fec/wrap_helpers.hh ../fec/wrap_helpers.cc \
  ../fec/multi_fec/multi_frame_fec_helpers.hh \
  ../fec/multi_fec/multi_frame_fec_helpers.cc \
  ../fec/multi_fec/streaming_code_theoretical.hh \
  ../fec/multi_fec/streaming_code_theoretical.cc \
  ../fec/multi_fec/streaming_code_helper.hh \
  ../fec/multi_fec/streaming_code_helper.cc \
  ../fec/multi_fec/puncture.hh ../fec/multi_fec/puncture.cc \
  ../fec/multi_fec/block_code_factory.hh \
  ../fec/multi_fec/block_code_factory.cc \
  ../fec/multi_fec/block_code.hh ../fec/multi_fec/block_code.cc \
  ../fec/streaming_code/multi_fec_header_code.hh \
  ../fec/streaming_code/multi_fec_header_code.cc \
  ../fec/logging/frame_logger.hh ../fec/logging/frame_logger.cc
test_streaming_code_LDADD = ../util/libutil.a $(gtest_LIBS)
