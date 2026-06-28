#pragma once

#include "AP_BattEkf_Model.h"

class AP_BattCagi
{
   public:
    AP_BattCagi(const Bat& bat);
    AP_BattCagi(const Bat& bat, float init_soc, uint64_t time_us);

    void calc_cc_Ah(uint64_t time_us, float curr);
    void awtls(uint64_t time_us, float soc, float curr);
    void update_init(float initial_soc, uint64_t time_us);

    float Qhat;
    float fit;
    float soh_pct;

   private:
    // Core parameters and states
    const Bat& bat;
    float init_soc = 0.f;
    uint64_t prev_time_cc_us = 0;
    float cc_Ah = 0.0f;

    // Filters and weights
    float gamma = 0.98;
    float SigmaX = 0.0;
    float SigmaY = 0.0;
    float K = 0.0;

    float C1 = 0.0, C2 = 0.0, C3 = 0.0, C4 = 0.0, C5 = 0.0, C6 = 0.0;

    float measX = 0.0;
    float measY = 0.0;

    int update_cnt = 0;
    const int C_AGI_UPDATE_SAMPLES = 1200;

    float min_r = 0.0;
    float max_r = 0.0;
};
