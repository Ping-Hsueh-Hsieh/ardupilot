#include "AP_BattCagi.h"
#include "AP_BattEkf_Math.h"

AP_BattCagi::AP_BattCagi(const Bat& b) : Qhat(b.Q_Ah), fit(0.f), bat(b)
{
    float sx = std::sqrt(2.0) * 0.01;
    float max_curr = 100.0;
    float precisionI = 1024.0;  // 1 << 10
    float binsize = 2.0 * max_curr / precisionI;
    float sy = binsize * std::sqrt(C_AGI_UPDATE_SAMPLES / 12.0) / 3600.0;

    SigmaX = sx * sx;
    SigmaY = sy * sy;

    K = std::sqrt(SigmaX / SigmaY);

    float K2 = K * K;
    float Qnom = bat.Q_Ah;
    float Qnom2 = Qnom * Qnom;

    C1 = 1.0 / (K2 * SigmaY);
    C2 = K * Qnom / (K2 * SigmaY);
    C3 = K2 * Qnom2 / (K2 * SigmaY);
    C4 = 1.0 / SigmaX;
    C5 = K * Qnom / SigmaX;
    C6 = K2 * Qnom2 / SigmaX;

    update_cnt = 0;

    float min_Q_Ah = bat.Q_Ah * 0.5;
    float max_Q_Ah = bat.Q_Ah * 1.5;

    min_r = min_Q_Ah * K;
    max_r = max_Q_Ah * K;

    this->soh_pct = (1.f - b.c_agi) * 100.f;
}

AP_BattCagi::AP_BattCagi(const Bat& b, float initial_soc, uint64_t time_us) : AP_BattCagi(b)
{
    this->update_init(initial_soc, time_us);
}

void AP_BattCagi::update_init(float initial_soc, uint64_t time_us)
{
    this->init_soc = initial_soc;
    this->prev_time_cc_us = time_us;
}

void AP_BattCagi::calc_cc_Ah(uint64_t time_us, float curr)
{
    float dif_time_s = static_cast<float>(time_us - prev_time_cc_us) * 1e-6;
    cc_Ah += dif_time_s * curr / 3600.0f;
    prev_time_cc_us = time_us;
}

void AP_BattCagi::awtls(uint64_t time_us, float soc, float curr)
{
    calc_cc_Ah(time_us, curr);

    if (update_cnt < C_AGI_UPDATE_SAMPLES) {
        update_cnt++;
        return;
    }

    measX = soc - init_soc;
    measY = cc_Ah;

    float K2 = K * K;
    C1 = gamma * C1 + (measX * measX) / (K2 * SigmaY);
    C2 = gamma * C2 + (K * measX * measY) / (K2 * SigmaY);
    C3 = gamma * C3 + (K2 * measY * measY) / (K2 * SigmaY);
    C4 = gamma * C4 + (measX * measX) / SigmaX;
    C5 = gamma * C5 + (K * measX * measY) / SigmaX;
    C6 = gamma * C6 + (K2 * measY * measY) / SigmaX;

    std::array<float, 5> coeffs = {C5, (-C1 + 2.0f * C4 - C6), (3.0f * C2 - 3.0f * C5), (C1 - 2.0f * C3 + C6), -C2};

    std::vector<float> r = MathUtils::find_positive_roots(coeffs, min_r, max_r, 1.0, 1e-10);
    if (r.empty()) return;

    std::vector<float> jr;
    jr.reserve(r.size());

    for (float rv : r) {
        float rv2 = rv * rv;
        float rv3 = rv2 * rv;
        float rv4 = rv2 * rv2;
        float numerator = rv4 * C4 - 2.0 * C5 * rv3 + (C1 + C6) * rv2 - 2.0 * C2 * rv + C3;
        float denom = (rv2 + 1.0) * (rv2 + 1.0);
        jr.push_back((1.0 / denom) * numerator);
    }

    size_t min_idx = MathUtils::argmin(jr);
    float Q = r[min_idx];

    // Update states
    Qhat = Q / K;
    update_cnt = 0;
    soh_pct = Qhat / bat.Q_Ah_orig * 100.f;
}
