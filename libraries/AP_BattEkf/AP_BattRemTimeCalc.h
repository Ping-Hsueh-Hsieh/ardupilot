#pragma once

#include <AP_Math/AP_Math.h>
#include <cstdint>
#include <string.h>

class AP_BattRemTimeCalc {
public:
    AP_BattRemTimeCalc() = default;

    // Main update function acting as the entry point
    void update(float Q_Ah, float lsoc, float volt, float curr);

    float rem_time_fw_min;
    float rem_time_vtol_min;

private:
    // Compile-time fixed-size Moving Average Filter (Idiomatic ArduPilot style)
    template <size_t WindowSize>
    class MovingAverage {
    public:
        MovingAverage() : idx(0), is_full(false), sum(0.0f) {
            memset(arr, 0, sizeof(arr));
        }

        float update(float v) {
            if (is_full) {
                sum -= arr[idx];
            }
            arr[idx] = v;
            sum += v;

            idx++;
            if (idx >= WindowSize) {
                idx = 0;
                is_full = true;
            }

            const size_t den = is_full ? WindowSize : idx;
            if (den == 0) {
                return 0.0f;
            }
            return sum / den;
        }

    private:
        float arr[WindowSize];
        size_t idx;
        bool is_full;
        float sum;
    };

    // Buffer Dimensions
    static constexpr size_t MAVG_FW_DIM = 300;
    static constexpr size_t MAVG_VTOL_DIM = 300;

    MovingAverage<MAVG_FW_DIM> fw_pwr_ring_buf;
    MovingAverage<MAVG_VTOL_DIM> vtol_pwr_ring_buf;

    // Private helper calculations
    float get_vtol_landing_est_curr_A() const { return -60.0f; }
    uint32_t get_vtol_reserved_landing_tries() const { return 3; }
    uint32_t get_vtol_landing_time_s() const { return 90; }

    float get_vtol_landing_coulumb_As_single() const {
        return get_vtol_landing_time_s() * get_vtol_landing_est_curr_A();
    }

    float get_vtol_landing_coulumb_As() const {
        return get_vtol_landing_coulumb_As_single() * get_vtol_reserved_landing_tries();
    }

    float get_reserved_vtol_soc(float Q_As) const {
        if (is_zero(Q_As)) {
            return 0.0f;
        }
        return -get_vtol_landing_coulumb_As() / Q_As;
    }

    float calc_rem_time_fw_min(float Q_Ah, float lmin_soc_thr, float volt, float curr, float lsoc);
    float calc_rem_time_vtol_min(float Q_Ah, float volt, float lsoc);
};
