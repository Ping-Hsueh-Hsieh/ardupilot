#include "AP_BattEkf.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include <stdint.h>

void AP_BattEkf::update(void)
{

    AP_BattMonitor& batt = AP::battery();
    float volt = batt.voltage();
    float curr = 0.f;
    if (!batt.current_amps(curr)) return;

    Sample sample = {
        .time = AP_HAL::micros(),
        .curr = -curr,
        .volt = volt,
    };

    this->process_sample(sample);

    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (volt, curr, soc)=(%.4f, %.4f, %.4f)", volt, curr, this->res.est_soc);
}
