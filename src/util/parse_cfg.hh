#ifndef PARSE_CFG_HH
#define PARSE_CFG_HH

#include <string>
#include <filesystem>
#include <iostream>

#include "../../third_party/json.hpp"
#include "../fec/multi_fec/multi_frame_fec_helpers.hh"

using json = nlohmann::json;
namespace fs = std::filesystem;

class Config
{
public:
    struct RSParams
    {
        static inline uint16_t max_data_pkts_per_group{64};
        static inline uint16_t max_fec_pkts_per_group{32};
    };

    struct RSMFParams
    {
        static inline uint16_t max_data_pkts_per_group{64};
        static inline uint16_t max_fec_pkts_per_group{32};
    };

    struct SCParams
    {
        static inline uint16_t max_data_pkts_per_group{64};
        static inline uint16_t max_fec_pkts_per_group{32};
        static inline std::string model{""};
        static inline std::string model_mean{""};
        static inline std::string model_var{""};
    };

    static void parse(const char *cfg_file)
    {
        cfg_file_name = std::string(cfg_file);
        cfg_file_base_name = fs::path(cfg_file_name).filename().string();

        std::ifstream config_file(cfg_file_name);
        config_file >> cfg_;
        auto it = cfg_.find("tau");
        if (it != cfg_.end())
        {
            Config::tau = *it;
        }
        it = cfg_.find("parity_delay");
        if (it != cfg_.end())
        {
            Config::parity_delay = *it;
        }
        it = cfg_.find("num_qrs_no_reduce");
        if (it != cfg_.end())
        {
            Config::num_qrs_no_reduce = *it;
        }
        it = cfg_.find("ge_gMin_pkt");
        if (it != cfg_.end())
        {
            Config::ge_gMin_pkt = *it;
        }
        it = cfg_.find("strategy");
        if (it != cfg_.end())
        {
            Config::strategy_name = *it;
        }
        it = cfg_.find("coding_scheme");
        if (it != cfg_.end())
        {
            Config::coding_scheme = *it;
        }
        it = cfg_.find("experiment_log_folder");
        if (it != cfg_.end())
        {
            Config::experiment_log_folder = *it;
        }
        it = cfg_.find("log_folder");
        if (it != cfg_.end())
        {
            Config::exp_log_folder = *it;
        }
        it = cfg_.find("trace_folder");
        if (it != cfg_.end())
        {
            Config::trace_folder = *it;
        }
        it = cfg_.find("port");
        if (it != cfg_.end())
        {
            Config::port = *it;
        }
        it = cfg_.find("sender_port");
        if (it != cfg_.end())
        {
            Config::sender_port = *it;
        }
        it = cfg_.find("one_way_delay");
        if (it != cfg_.end())
        {
            Config::one_way_delay = *it;
        }

        it = cfg_.find("w");
        if (it != cfg_.end())
        {
            Config::w = *it;
        }
        it = cfg_.find("size_factor");
        if (it != cfg_.end())
        {
            Config::size_factor = *it;
        }
        it = cfg_.find("max_frame_size_possible");
        if (it != cfg_.end())
        {
            Config::max_frame_size_possible = *it;
        }
        it = cfg_.find("ReedSolomonCode");
        if (it != cfg_.end())
        {
            auto cit = it->find("max_data_pkts_per_group");
            if (cit != it->end())
            {
                Config::RSParams::max_data_pkts_per_group = *cit;
            }
            cit = it->find("max_fec_pkts_per_group");
            if (cit != it->end())
            {
                Config::RSParams::max_fec_pkts_per_group = *cit;
            }
        }

        it = cfg_.find("ReedSolomonMultiFrame");
        if (it != cfg_.end())
        {
            auto cit = it->find("max_data_pkts_per_group");
            if (cit != it->end())
            {
                Config::RSMFParams ::max_data_pkts_per_group = *cit;
            }
            cit = it->find("max_fec_pkts_per_group");
            if (cit != it->end())
            {
                Config::RSMFParams::max_fec_pkts_per_group = *cit;
            }
        }

        it = cfg_.find("StreamingCode");
        if (it != cfg_.end())
        {
            auto cit = it->find("max_data_pkts_per_group");
            if (cit != it->end())
            {
                Config::SCParams ::max_data_pkts_per_group = *cit;
            }
            cit = it->find("max_fec_pkts_per_group");
            if (cit != it->end())
            {
                Config::SCParams::max_fec_pkts_per_group = *cit;
            }
            cit = it->find("model");
            if (cit != it->end())
            {
                Config::SCParams::model = *cit;
            }
            cit = it->find("model_mean");
            if (cit != it->end())
            {
                Config::SCParams::model_mean = *cit;
            }
            cit = it->find("model_var");
            if (cit != it->end())
            {
                Config::SCParams::model_var = *cit;
            }
        }

        it = cfg_.find("t_gb");
        if (it != cfg_.end())
        {
            Config::t_gb = *it;
        }
        it = cfg_.find("t_bg");
        if (it != cfg_.end())
        {
            Config::t_bg = *it;
        }
        it = cfg_.find("l_b");
        if (it != cfg_.end())
        {
            Config::l_b = *it;
        }
        it = cfg_.find("l_g");
        if (it != cfg_.end())
        {
            Config::l_g = *it;
        }
        it = cfg_.find("seed");
        if (it != cfg_.end())
        {
            Config::seed = *it;
        }
        it = cfg_.find("width");
        if (it != cfg_.end())
        {
            Config::width = *it;
        }
        it = cfg_.find("height");
        if (it != cfg_.end())
        {
            Config::height = *it;
        }
        it = cfg_.find("timeout");
        if (it != cfg_.end())
        {
            Config::timeout = *it;
        }
        it = cfg_.find("video_name");
        if (it != cfg_.end())
        {
            Config::video_name = *it;
        }

        it = cfg_.find("interval_ms_feedback");
        if (it != cfg_.end())
        {
            Config::interval_ms_feedback = *it;
        }

        // Infer
        Config::packet_size = 8 * Config::size_factor;
        Config::stripe_size = Config::w * Config::packet_size;

        int modulo = Config::max_frame_size_possible % Config::stripe_size;

        Config::max_data_stripes_per_frame = Config::max_frame_size_possible / Config::stripe_size + (modulo > 0);

        Config::max_fec_stripes_per_frame = (Config::max_data_stripes_per_frame + 1) / 2;
        if (Config::coding_scheme.rfind("ReedSolomonMultiFrame", 0) == 0)
        {
            if (Config::tau > 0)
            {
                Config::max_fec_stripes_per_frame *= ((Config::tau + 1));
                Config::max_fec_stripes_per_frame++;
                Config::RSMFParams::max_fec_pkts_per_group *= ((Config::tau + 1));
                Config::RSMFParams::max_fec_pkts_per_group++;
                Config::is_multi_frame_block_code = true;
            }
        }        
        assert(Config::max_fec_stripes_per_frame * Config::stripe_size >= Config::max_frame_size_possible / 2);

        assert(Config::max_data_stripes_per_frame * Config::stripe_size >= Config::max_frame_size_possible);
        assert(uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau)) * Config::max_data_stripes_per_frame * Config::max_fec_stripes_per_frame < pow(2, Config::w));
    }

    friend std::ostream &operator<<(std::ostream &oss, Config &cfg)
    {
        oss << cfg << std::endl;
        oss << "\n-----------Inferred--------\n"
            << "packet_size:" << Config::packet_size << "\n"
            << "stripe_size:" << Config::stripe_size << "\n"
            << "max_data_stripes_per_frame:" << Config::max_data_stripes_per_frame << "\n"
            << "max_fec_stripes_per_frame:" << Config::max_fec_stripes_per_frame << "\n";

        return oss;
    }

    static inline json cfg_;
    static inline std::string cfg_file_name{""};
    static inline std::string cfg_file_base_name{""};
    static inline uint16_t tau{3};
    static inline uint16_t parity_delay{0};
    static inline uint8_t num_qrs_no_reduce{2};
    static inline uint16_t ge_gMin_pkt{1};
    static inline std::string strategy_name{""};
    static inline std::string coding_scheme{""};
    static inline std::string experiment_log_folder{""};
    static inline std::string exp_log_folder{""};
    static inline std::string trace_folder{""};
    static inline int port{8080};
    static inline int sender_port{8081};
    static inline int one_way_delay{50};
    static inline uint16_t w{32};
    static inline int size_factor{1};
    static inline int max_frame_size_possible{16383};
    static inline int max_data_stripes_per_frame{64};
    static inline int max_fec_stripes_per_frame{32};
    static inline int packet_size{8};
    static inline int stripe_size{256};
    static inline uint8_t max_qr{1};
    static inline bool is_multi_frame_block_code{false};
    static inline float t_gb{0.01};
    static inline float t_bg{0.5};
    static inline float l_g{0.01};
    static inline float l_b{0.5};
    static inline float interval_ms_feedback{2000.0};
    static inline int seed{0};
    static inline std::string video_name{""};
    static inline uint16_t width{854};
    static inline uint16_t height{480};
    static inline uint64_t timeout{900};

};

#endif /* PARSE_CFG_HH */
