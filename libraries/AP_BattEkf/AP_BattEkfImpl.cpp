#include "AP_BattEkfImpl.h"
#include <stdint.h>

EkfRes AP_BattEkfImpl::handle_init(const Sample& sample)
{
    xhat[0] = this->bat.ocv.luk_soc(sample.volt / this->bat.num_of_cell);
    xhat[1] = 0.0f;
    yhat = sample.volt;
    prior_t = sample.time;
    prior_i = sample.curr;
    gfsm = FSM::PROCESS;
    return {xhat[0], xhat[1], yhat, SigmaX};
}

EkfRes AP_BattEkfImpl::handle_process(const Sample& sample)
{
    xhat[0] = this->bat.ocv.luk_soc(sample.volt / this->bat.num_of_cell);
    xhat[1] = 0.0f;
    yhat = sample.volt;

    float dt_us = static_cast<float>(sample.time - prior_t);
    float dt = dt_us / 1000000.0f;

    prior_t = sample.time;
    prior_dt = dt;
    prior_i = sample.curr;
    gfsm = FSM::RUN;
    return {xhat[0], xhat[1], yhat, SigmaX};
}

EkfRes AP_BattEkfImpl::handle_run(const Sample& sample)
{
    float volt_cell = sample.volt / this->bat.num_of_cell;
    float curr = sample.curr;

    // WARN: since the time diff should not exceed UINT32_MAX, float should do the faver
    float dt_us = static_cast<float>(sample.time - prior_t);
    float dt = dt_us / 1000000.0f;

    float tau_bv = this->bat.bv.tau;
    float rbv = this->bat.bv.rbv;
    float Q_As = this->bat.Q_As;

    // Prediction Block
    std::array<float, 4> A = {1.0f, 0.0f, 0.0f, tau_bv / (prior_dt + tau_bv)};
    std::array<float, 2> B = {prior_dt / Q_As, prior_dt / (prior_dt + tau_bv)};

    std::array<float, 2> x_pred = MathUtils::mat2_mul_vec2(A, xhat);
    x_pred[0] += B[0] * prior_i;
    x_pred[1] += B[1] * prior_i;
    x_pred[0] = MathUtils::clamp(x_pred[0], 0.0f, 1.0f);

    std::array<float, 4> A_Sigma = MathUtils::mat2_mul_mat2(A, SigmaX);
    A_Sigma = MathUtils::mat2_mul_mat2(A_Sigma, MathUtils::mat2_trans(A));
    std::array<float, 4> Bw = MathUtils::outer2(B, B);
    std::array<float, 4> Bw_scaled = MathUtils::mat2_scale(Bw, SigmaW);
    std::array<float, 4> SigmaX_pred = MathUtils::mat2_add(A_Sigma, Bw_scaled);

    float soc = x_pred[0];
    float ibv = x_pred[1];
    float ocv_cell = this->bat.ocv.luk_volt(soc);
    float vir = this->bat.ir.luk_ir(curr) * curr;
    float vbv = ibv * rbv;
    yhat = (ocv_cell + vir + vbv);

    // Correction Block
    float d = this->bat.ocv.luk_docvdsoc_y(soc);
    std::array<float, 2> C = {d, rbv};
    float SigmaY = MathUtils::quad1(C, SigmaX_pred) + SigmaV;
    float invSigmaY = MathUtils::inv1(SigmaY);

    std::array<float, 2> SxC = {SigmaX_pred[0] * C[0] + SigmaX_pred[1] * C[1], SigmaX_pred[2] * C[0] + SigmaX_pred[3] * C[1]};
    std::array<float, 2> L = {SxC[0] * invSigmaY, SxC[1] * invSigmaY};

    float r = volt_cell - yhat;
    if ((r * r) > (100.0f * SigmaY)) {
        L = {0.0f, 0.0f};
    }

    xhat[0] = x_pred[0] + L[0] * r;
    xhat[1] = x_pred[1] + L[1] * r;
    xhat[0] = MathUtils::clamp(xhat[0], 0.0f, 1.0f);

    std::array<float, 4> LL = MathUtils::outer2(L, L);
    std::array<float, 4> LLs = MathUtils::mat2_scale(LL, SigmaY);
    std::array<float, 4> SigmaX_upd = MathUtils::mat2_sub(SigmaX_pred, LLs);

    if ((r * r) > (4.0f * SigmaY)) {
        SigmaX_upd = MathUtils::mat2_scale(SigmaX_upd, SXbump);
    }

    SigmaX = MathUtils::symmetrize2(SigmaX_upd);

    prior_t = sample.time;
    prior_dt = dt;
    prior_i = curr;

    return {xhat[0], xhat[1], yhat * this->bat.num_of_cell, SigmaX};
}

void AP_BattEkfImpl::process_sample(const Sample& sample)
{
    this->SigmaV = this->SigmaV_Pack / this->bat.num_of_cell;
    switch (this->gfsm) {
    case FSM::INIT: {
        this->res = this->handle_init(sample);
    } break;
    case FSM::PROCESS: {
        this->res = this->handle_process(sample);
    } break;
    case FSM::RUN: {
        this->res = this->handle_run(sample);
    } break;
    }
}
