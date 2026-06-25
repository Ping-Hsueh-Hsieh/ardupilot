from matplotlib import pyplot as plt
import numpy as np
import pandas as pd

Q_Ah = 21.671
Q_As = Q_Ah * 3600

def calc_cc_soc(times, currs, Q_As):
    delta_cc = np.diff(times) * currs[1:]
    cc = np.cumsum(delta_cc)
    cc = np.hstack([[0], cc])
    return cc / Q_As

if __name__ == '__main__':
    df_data = pd.read_csv("./record/output_data.csv")
    times = df_data['times'].to_numpy()
    currs = df_data['currs'].to_numpy()
    volts = df_data['volts'].to_numpy()

    df_res = pd.read_csv("./out/samples_out.csv")
    est_soc = df_res['est_soc'].to_numpy()
    est_ibv = df_res['est_ibv'].to_numpy()
    est_volt = df_res['est_volt'].to_numpy()
    sigma_soc = df_res['sigma_soc'].to_numpy()

    df_c_agi = pd.read_csv("./out/samples_out_c_agi.csv")
    qhats = df_c_agi['qhat'].to_numpy()
    c_agi_pcts = df_c_agi['c_agi_pct'].to_numpy()

    df_rem_time = pd.read_csv("./out/samples_out_rem_time.csv")
    rem_time_fw_mins = df_rem_time['rem_time_fw_min'].to_numpy()
    rem_time_vtol_mins = df_rem_time['rem_time_vtol_min'].to_numpy()

    c_agi = -0.032
    soc_cc = calc_cc_soc(times, currs, Q_As * (1 + c_agi)) + est_soc[0]

    soc_cc_no_agi = calc_cc_soc(times, currs, Q_As) + est_soc[0]

    ####################################################################################################

    fig = plt.figure(figsize=(16, 10))
    gs = fig.add_gridspec(6, 1)

    ax_curr = fig.add_subplot(gs[0])
    ax_curr.plot(times, currs, 'ko--', lw=1, ms=2, label='orig')
    ax_curr.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_curr.set_ylabel('Current[A]')

    ax_volt = fig.add_subplot(gs[1], sharex=ax_curr)
    ax_volt.plot(times, volts, 'ko--', lw=1, ms=2, label='orig')
    ax_volt.plot(times, est_volt, 'bo--', lw=1, ms=2, label='est')
    ax_volt.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_volt.legend()
    ax_volt.set_ylabel('Voltage[V]')

    ax_soc = fig.add_subplot(gs[2], sharex=ax_curr)
    ax_soc.plot(times, est_soc, 'ko--', lw=1, ms=2, label='ekf')
    ax_soc.plot(times, soc_cc, 'bo--', lw=1, ms=2, label='cc')
    ax_soc.plot(times, soc_cc_no_agi, 'ro--', lw=1, ms=2, label='cc_no_agi')
    ax_soc.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_soc.legend()
    ax_soc.set_ylabel('SOC[--]')

    soc_err = est_soc - soc_cc
    ax_soc_err = fig.add_subplot(gs[3], sharex=ax_curr)
    ax_soc_err.plot(times, soc_err, 'ko--', lw=1, ms=2)
    ax_soc_err.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_soc_err.set_ylabel('soc error[--]')

    ax_sigma = fig.add_subplot(gs[4], sharex=ax_curr)
    ax_sigma.plot(times, sigma_soc, 'ko--', lw=1, ms=2)
    ax_sigma.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_sigma.set_ylabel('3*sqrt(sigma)[--]')

    ax_qhat = fig.add_subplot(gs[5], sharex=ax_curr)
    ax_qhat.plot(times, c_agi_pcts, 'ko--', lw=1, ms=2)
    ax_qhat.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_qhat.set_ylabel('agi[%]')

    fig.supxlabel('Time[s]')

    fig.tight_layout()

    ####################################################################################################

    fig = plt.figure(figsize=(16, 10))
    gs = fig.add_gridspec(2, 1)

    ax_fw = fig.add_subplot(gs[0])
    ax_fw.plot(rem_time_fw_mins, 'ko--', lw=1, ms=2, label='orig')
    ax_fw.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_fw.set_ylabel('FW[min]')

    ax_vtol = fig.add_subplot(gs[1], sharex=ax_fw)
    ax_vtol.plot(rem_time_vtol_mins, 'ko--', lw=1, ms=2, label='orig')
    ax_vtol.grid(c='gray', lw=0.5, ls='--', alpha=0.5)
    ax_vtol.legend()
    ax_vtol.set_ylabel('VTOL[min]')

    fig.tight_layout()

    plt.show()
