#pragma once


#define MP_ENABLE (0)

#include <GCS_MAVLink/GCS.h>
#include "AP_HAL/system.h"

#if MP_ENABLE
#define MP_START(x) (x).start()
#define MP_END(x) (x).end()

#define MP_SHOW()                                                                                                         \
    do {                                                                                                                  \
        if (mps.ekf.is_set) GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] ekf = %.4f ms", mps.ekf.delta_ms());                \
        if (mps.c_agi.is_set) GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] c_agi = %.4f ms", mps.c_agi.delta_ms());          \
        if (mps.rem_time.is_set) GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] rem_time = %.4f ms", mps.rem_time.delta_ms()); \
    } while (0)

#include <stdint.h>

struct Mp
{
    uint64_t start_ = 0;
    uint64_t end_ = 0;
    double delta = 0.0;
    bool is_set = false;

    void start(void)
    {
        this->is_set = false;
        this->start_ = AP_HAL::micros64();
    }

    void end(void)
    {
        this->end_ = AP_HAL::micros64();
        this->delta = delta_ms();
        this->is_set = true;
    }

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

extern Mps mps;

#else  //  MP_ENABLE
#define MP_START(x) (void)0
#define MP_END(x) (void)0
#define MP_SHOW() (void)0
#endif  //  MP_ENABLE
