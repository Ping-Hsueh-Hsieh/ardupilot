#include "AP_BattEkf.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include <stdint.h>
#include "AP_BattCagi.h"

#define MP_ENABLE (0)

static bool first = true;

#if MP_ENABLE
struct Mp {
    uint64_t start_ = 0;
    uint64_t end_ = 0;
    void start(void) { this->start_ = AP_HAL::micros64(); }
    void end(void) { this->end_ = AP_HAL::micros64(); }
    uint64_t delta_u64(void) {
        if (start_ <= end_) return end_ - start_;
        return end_ + (UINT64_MAX - start_);
    }
    double delta_ms(void) {
        uint64_t tmp_u64 = this->delta_u64();
        return (double)tmp_u64 * 1e-3;
    }
};

struct Mps {
    Mp ekf;
    Mp c_agi;
};

static Mps mps;
#endif //  MP_ENABLE

struct Cfg {
    float min_cell_volt = 3.5;
};

static Cfg cfg;

void AP_BattEkf::update(void)
{

    AP_BattMonitor& batt = AP::battery();
    float volt = batt.voltage();
    if (volt <= cfg.min_cell_volt * this->get_bat().num_of_cell) return;  // NOTE: this will prevent segfault during SITL
    float curr = 0.f;
    if (!batt.current_amps(curr)) return;

    Sample sample = {
        .time = AP_HAL::micros(),
        .curr = -curr,
        .volt = volt,
    };

#if MP_ENABLE
    mps.ekf.start();
#endif //  MP_ENABLE
    this->process_sample(sample);
#if MP_ENABLE
    mps.ekf.end();
#endif //  MP_ENABLE

    AP_BattCagi c_agi_awtls = AP_BattCagi(this->get_bat());

    if (first) {
        c_agi_awtls.update_init(this->res.est_soc, sample.time);
        first = false;
    } else {
#if MP_ENABLE
        mps.c_agi.start();
#endif //  MP_ENABLE
        c_agi_awtls.awtls(sample.time, this->res.est_soc, curr);
#if MP_ENABLE
        mps.c_agi.end();
#endif //  MP_ENABLE
    }

    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (%.4f, %.4f, %.4f, %.4f, %.4f)", volt, curr, this->res.est_soc, c_agi_awtls.Qhat, c_agi_awtls.soh_pct);
#if MP_ENABLE
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (%.4f, %.4f)ms", mps.ekf.delta_ms(), mps.c_agi.delta_ms());
#endif //  MP_ENABLE
}
