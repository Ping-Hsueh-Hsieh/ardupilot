#pragma once
#include <array>
#include "AP_BattEkf_Math.h"
#include "AP_BattEkf_Model.h"

class AP_BattEkfImpl
{
   public:
    void process_sample(const Sample& sample);
    Bat& get_bat(void) {
        return this->bat;
    }

    EkfRes res = {};

   private:
    enum FSM
    {
        INIT = 0,
        PROCESS,
        RUN,
    };

    FSM gfsm = FSM::INIT;
    Bat bat = Bat::create_default();

    const float SXbump = 5.0f;
    std::array<float, 2> xhat = {0.0f, 0.0f};  // [soc, ibv]
    std::array<float, 4> SigmaX = {1e-2f, 0.0f, 0.0f, 1e-2f};
    const float SigmaW = 0.1f * 0.1f;
    const float SigmaV_Pack = 0.05f * 0.05f;
    float SigmaV = 0.0f;

    float prior_i = 0.0f;
    uint64_t prior_t = 0.0;
    float prior_dt = 0.0f;
    float yhat = 0.0f;

    EkfRes handle_init(const Sample& sample);
    EkfRes handle_process(const Sample& sample);
    EkfRes handle_run(const Sample& sample);
};
