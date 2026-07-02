#include "AP_BattEkf.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Logger/AP_Logger.h>
#include <GCS_MAVLink/GCS.h>

#include "AP_BattEkfMp.h"

const AP_Param::GroupInfo AP_BattEkf::var_info[] = {
    // @Param: DIS
    // @DisplayName: Disable Battery EKF
    // @Description: Disable the Battery EKF
    // @Values: 0:False,1:True
    // @User: Advanced
    AP_GROUPINFO("DIS", 0, AP_BattEkf, param_disable, 0),

    AP_GROUPEND,
};

struct Cfg
{
    float min_cell_volt = 3.5;
};

static Cfg cfg;

void AP_BattEkf::init(void)
{
    if (param_disable.get() != 0) {
        _disable = true;
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[INFO] BattEkf disabled");
    } else {
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[INFO] BattEkf enabled");
    }
}

void AP_BattEkf::gcs_writer(void)
{
    switch (msg_id) {
    case SOC: {
        gcs().send_named_float("SOC[%]", res.est_soc * 100);
    } break;
    case REM_FW: {
        gcs().send_named_float("REM_FW[MIN]", rem_calc.rem_time_fw_min);
    } break;
    case REM_RTL: {
        gcs().send_named_float("REM_RTL[MIN]", rem_calc.rem_time_vtol_min);
    } break;
    case SOH: {
        gcs().send_named_float("SOH[%]", c_agi_awtls.soh_pct);
    } break;
    case ESTCAP: {
        gcs().send_named_float("EstCap[Ah]", c_agi_awtls.Qhat);
    } break;
    default: break;
    }

    msg_id++;
    msg_id = msg_id % gcs_writer_period;
}

void AP_BattEkf::update(void)
{
    if (_disable) return;
    AP_BattMonitor& batt = AP::battery();
    float volt = batt.voltage();
    if (volt <= cfg.min_cell_volt * this->get_bat().num_of_cell) return;  // NOTE: this will prevent segfault during SITL
    float curr = 0.f;
    if (!batt.current_amps(curr)) return;
    curr = -curr;

    Sample sample = {
        .time = AP_HAL::micros64(),
        .curr = curr,
        .volt = volt,
    };

    MP_START(mps.ekf);
    this->process_sample(sample);
    MP_END(mps.ekf);

    if (c_agi_first) {
        c_agi_awtls.update_init(this->res.est_soc, sample.time);
        c_agi_first = false;
    } else {
        MP_START(mps.c_agi);
        c_agi_awtls.awtls(sample.time, this->res.est_soc, curr);
        MP_END(mps.c_agi);

        MP_START(mps.rem_time);
        rem_calc.update(c_agi_awtls.Qhat, this->res.est_soc, volt, curr);
        MP_END(mps.rem_time);
    }

    this->gcs_writer();
    AP::logger().WriteStreaming("CUST", "TimeUS,soc_pct,rem_fw_min,rem_rtl_min,soh_pct,est_cap_Ah", "Qfffff", AP_HAL::micros64(), this->res.est_soc * 100.f,
                                rem_calc.rem_time_fw_min, rem_calc.rem_time_vtol_min, c_agi_awtls.soh_pct, c_agi_awtls.Qhat);

    // GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (%.4f, %.4f, %.4f, %.4f, %.4f)", volt, curr, this->res.est_soc, c_agi_awtls.Qhat, c_agi_awtls.soh_pct);
    MP_SHOW();
}
