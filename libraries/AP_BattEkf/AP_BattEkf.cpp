#include "AP_BattEkf.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include <stdint.h>
#include <vector>

enum FSM
{
    INIT = 0,
    PROCESS,
    RUN,
};

struct Ocv
{
    std::vector<float> ocv_soc;
    std::vector<float> ocv_vol;
    std::vector<float> docvdsoc_x;  // SOC -> docv/dsoc
    std::vector<float> docvdsoc_y;

    float luk_soc(float volt) {
        return 200.0f;
    }
    float luk_ocv(float soc) {
        return 999.0f;
    }
    float luk_docvdsoc(float soc) {
        return 0.0f;
    }

    Ocv(void) {
        this->ocv_soc = {};
        this->ocv_vol = {};
        this->docvdsoc_x = {};
        this->docvdsoc_y = {};
    }

};

static Ocv ocv = Ocv();

static FSM gfsm = FSM::INIT;

static float prior_curr = 0.f;
static uint32_t tnow = 0;
static uint32_t tprev = 0;
static float prior_dt = 0.f;
static float dt = 0.f;

// // [SOC, ibv]
// static constexpr size_t sys_dim = 2;
// static constexpr size_t in_dim = 1;
// static constexpr size_t out_dim = 1;

// static float xhat[sys_dim] = {};
// static float SigmaX[sys_dim * sys_dim] = {};
// static float SigmaW[in_dim * in_dim] = {};
// static float SigmaV[out_dim * out_dim] = {};
// static float A[sys_dim * sys_dim] = {};
// static float B[sys_dim * in_dim] = {};
// static float C[out_dim * sys_dim] = {};
// static float D[out_dim * in_dim] = {};
// static float L[sys_dim * out_dim] = {};

void AP_BattEkf::update(void)
{

    AP_BattMonitor& batt = AP::battery();
    float volt = batt.voltage();
    float curr = 0.f;
    if (!batt.current_amps(curr)) return;

    switch (gfsm) {
    case FSM::INIT: {
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] INIT");

        tprev = AP_HAL::micros();
        this->soc = ocv.luk_soc(volt);
        gfsm = FSM::PROCESS;
    } break;
    case FSM::PROCESS: {
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] PROCESS");

        tnow = AP_HAL::micros();
        dt = (float)(tnow - tprev) * 1e-6;
        tprev = tnow;

        this->soc = ocv.luk_soc(volt);

        gfsm = FSM::RUN;
    } break;
    case FSM::RUN: {
    } break;
    }

    tnow = tprev;
    prior_dt = dt;
    prior_curr = curr;

    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "[DEBUG] (dt, soc)=(%.4f, %.4f)", dt, this->soc);
}
