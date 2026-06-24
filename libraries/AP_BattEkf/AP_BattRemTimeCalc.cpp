#include "AP_BattRemTimeCalc.h"

void AP_BattRemTimeCalc::update(float Q_Ah, float lsoc, float volt, float curr)
{
    float reserved_vtol_soc = get_reserved_vtol_soc(Q_Ah * 3600.0f);
    this->rem_time_fw_min = calc_rem_time_fw_min(Q_Ah, reserved_vtol_soc, volt, curr, lsoc);
    this->rem_time_vtol_min = calc_rem_time_vtol_min(Q_Ah, volt, lsoc);
}

float AP_BattRemTimeCalc::calc_rem_time_fw_min(float Q_Ah, float lmin_soc_thr, float volt, float curr, float lsoc)
{
    // Using fminf from AP_Math / standard library
    float curr_pwr = volt * fminf(curr, -0.1f);
    float avg_pwr = fw_pwr_ring_buf.update(curr_pwr);

    float rem_time_min = 0.0f;
    if (lmin_soc_thr <= lsoc) {
        float rem_Ih = (lmin_soc_thr - lsoc) * Q_Ah;
        float rem_Wh = rem_Ih * volt;
        if (!is_zero(avg_pwr)) {
            rem_time_min = (rem_Wh / avg_pwr) * 60.0f;
        }
    }

    return rem_time_min;
}

float AP_BattRemTimeCalc::calc_rem_time_vtol_min(float Q_Ah, float volt, float lsoc)
{
    float est_curr = get_vtol_landing_est_curr_A();
    float curr_pwr = volt * est_curr;
    float avg_pwr = vtol_pwr_ring_buf.update(curr_pwr);

    float rem_Ih = -lsoc * Q_Ah;
    float rem_Wh = rem_Ih * volt;

    float rem_time_min = 0.0f;
    if (!is_zero(avg_pwr)) {
        rem_time_min = (rem_Wh / avg_pwr) * 60.0f;
    }

    return rem_time_min;
}
