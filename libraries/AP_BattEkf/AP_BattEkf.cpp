#include "AP_BattEkf.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Logger/AP_Logger.h>
#include <GCS_MAVLink/GCS.h>
#include "AP_BattCagi.h"
#include "AP_BattRemTimeCalc.h"

#define MP_ENABLE (0)

static bool first = true;

#if MP_ENABLE
#define MP_START(x) (x).start()
#define MP_END(x) (x).end()
#define MP_SHOW()                                                                                                                            \
    do {                                                                                                                                     \
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (%.4f, %.4f, %.4f)ms", mps.ekf.delta_ms(), mps.c_agi.delta_ms(), mps.rem_time.delta_ms()); \
    } while (0)

#include <stdint.h>

struct Mp
{
    uint64_t start_ = 0;
    uint64_t end_ = 0;

    void start(void) { this->start_ = AP_HAL::micros64(); }

    void end(void) { this->end_ = AP_HAL::micros64(); }

    uint64_t delta_u64(void)
    {
        if (start_ <= end_) return end_ - start_;
        return end_ + (UINT64_MAX - start_);
    }

    double delta_ms(void)
    {
        uint64_t tmp_u64 = this->delta_u64();
        return (double)tmp_u64 * 1e-3;
    }
};

struct Mps
{
    Mp ekf;
    Mp c_agi;
    Mp rem_time;
};

static Mps mps;
#else  //  MP_ENABLE
#define MP_START(x) (void)0
#define MP_END(x) (void)0
#define MP_SHOW() (void)0
#endif  //  MP_ENABLE

struct Cfg
{
    float min_cell_volt = 3.5;
};

static Cfg cfg;

static AP_BattRemTimeCalc rem_calc;

void AP_BattEkf::update(void)
{

    AP_BattMonitor& batt = AP::battery();
    float volt = batt.voltage();
    if (volt <= cfg.min_cell_volt * this->get_bat().num_of_cell) return;  // NOTE: this will prevent segfault during SITL
    float curr = 0.f;
    if (!batt.current_amps(curr)) return;

    Sample sample = {
        .time = AP_HAL::micros64(),
        .curr = -curr,
        .volt = volt,
    };

    MP_START(mps.ekf);
    this->process_sample(sample);
    MP_END(mps.ekf);

    AP_BattCagi c_agi_awtls = AP_BattCagi(this->get_bat());

    if (first) {
        c_agi_awtls.update_init(this->res.est_soc, sample.time);
        first = false;
    } else {
        MP_START(mps.c_agi);
        c_agi_awtls.awtls(sample.time, this->res.est_soc, curr);
        MP_END(mps.c_agi);

        MP_START(mps.rem_time);
        rem_calc.update(c_agi_awtls.Qhat, this->res.est_soc, volt, curr);
        MP_END(mps.rem_time);
    }

    gcs().send_named_float("SOC[%]", this->res.est_soc * 100);
    gcs().send_named_float("REM_FW[MIN]", rem_calc.rem_time_fw_min);
    gcs().send_named_float("REM_RTL[MIN]", rem_calc.rem_time_vtol_min);
    gcs().send_named_float("SOH[%]", c_agi_awtls.soh_pct);
    gcs().send_named_float("EstCap[Ah]", c_agi_awtls.Qhat);

    AP::logger().WriteStreaming("CUST", "TimeUS,soc_pct,rem_fw_min,rem_rtl_min,soh_pct,est_cap_Ah", "Qfffff", AP_HAL::micros64(), this->res.est_soc * 100.f,
                                rem_calc.rem_time_fw_min, rem_calc.rem_time_vtol_min, c_agi_awtls.soh_pct, c_agi_awtls.Qhat);

    // GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (%.4f, %.4f, %.4f, %.4f, %.4f)", volt, curr, this->res.est_soc, c_agi_awtls.Qhat, c_agi_awtls.soh_pct);
    MP_SHOW();
}
