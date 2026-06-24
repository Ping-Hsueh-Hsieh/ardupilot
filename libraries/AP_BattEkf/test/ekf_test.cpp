#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include "../AP_BattCagi.h"
#include "../AP_BattEkfImpl.h"
#include "../AP_BattRemTimeCalc.h"
#include "CsvReader.h"

namespace fs = std::filesystem;

void write_csv_ekf(fs::path path, std::vector<EkfRes> ress)
{
    std::ofstream outfile(path);
    if (!outfile.is_open()) {
        fs::remove(path);
    }
    outfile << "est_soc,est_ibv,est_volt,sigma_soc";
    outfile << std::endl;
    outfile << std::fixed << std::setprecision(4);

    for (const auto& res : ress) {
        outfile << res.est_soc;
        outfile << ",";
        outfile << res.est_ibv;
        outfile << ",";
        outfile << res.est_volt;
        outfile << ",";
        outfile << res.SigmaX[0];
        outfile << std::endl;
    }

    outfile.close();
}

void write_csv_cagi(fs::path path, std::vector<std::pair<float, float>> ress)
{
    std::ofstream outfile(path);
    if (!outfile.is_open()) {
        fs::remove(path);
    }
    outfile << "qhat,c_agi_pct";
    outfile << std::endl;
    outfile << std::fixed << std::setprecision(4);

    for (const auto& [qhat, c_agi_pct] : ress) {
        outfile << qhat;
        outfile << ",";
        outfile << c_agi_pct;
        outfile << std::endl;
    }

    outfile.close();
}

void write_csv_rem_time(fs::path path, std::vector<std::pair<float, float>> ress)
{
    std::ofstream outfile(path);
    if (!outfile.is_open()) {
        fs::remove(path);
    }
    outfile << "rem_time_fw_min,rem_time_vtol_min";
    outfile << std::endl;
    outfile << std::fixed << std::setprecision(4);

    for (const auto& [rem_time_fw_min, rem_time_vtol_min] : ress) {
        outfile << rem_time_fw_min;
        outfile << ",";
        outfile << rem_time_vtol_min;
        outfile << std::endl;
    }

    outfile.close();
}

int main(void)
{
    bool first = true;
    float c_agi_pct = 0.f;
    auto csv_data = CsvReader::read("./record/output_data.csv", ',');
    if (csv_data.empty()) {
        printf("[ERROR] no csv data\n");
        return 1;
    }
    size_t head_row_cnt = 1;

    AP_BattEkfImpl batt_ekf = {};
    const Bat& bat = batt_ekf.get_bat();
    AP_BattCagi c_agi_awtls = AP_BattCagi(batt_ekf.get_bat());
    AP_BattRemTimeCalc rem_calc;

    std::vector<EkfRes> ress(csv_data.size() - head_row_cnt);
    std::vector<std::pair<float, float>> cagi_ress(csv_data.size() - head_row_cnt);
    std::vector<std::pair<float, float>> rem_time_ress(csv_data.size() - head_row_cnt);

    for (size_t row_idx = 0; row_idx < csv_data.size(); ++row_idx) {
        const auto& row = csv_data[row_idx];

        assert(row.size() == 3);
        if (row_idx < head_row_cnt) {
            std::cout << "heading: ";
            std::cout << row[0] << ", " << row[1] << ", " << row[2] << std::endl;
        } else {
            uint32_t time = static_cast<uint32_t>(std::stof(row[0]) * 1e+6);
            float curr = std::stof(row[1]);
            float volt = std::stof(row[2]);
            Sample sample = {time, curr, volt};
            batt_ekf.process_sample(sample);
            ress[row_idx - head_row_cnt] = batt_ekf.res;

            float soc = batt_ekf.res.est_soc;

            if (first) {
                c_agi_awtls.update_init(soc, sample.time);
                first = false;
            } else {
                c_agi_awtls.awtls(sample.time, soc, curr);
                c_agi_pct = c_agi_awtls.Qhat / bat.Q_Ah * 100.f - 100.f;
            }
            cagi_ress[row_idx - head_row_cnt] = std::pair(c_agi_awtls.Qhat, c_agi_pct);

            rem_calc.update(c_agi_awtls.Qhat, soc, volt, curr);
            rem_time_ress[row_idx - head_row_cnt] = std::pair(rem_calc.rem_time_fw_min, rem_calc.rem_time_vtol_min);
        }
    }

    fs::path out_ekf_path("./out/samples_out.csv");
    write_csv_ekf(out_ekf_path, ress);

    fs::path out_cagi_path("./out/samples_out_c_agi.csv");
    write_csv_cagi(out_cagi_path, cagi_ress);

    fs::path out_rem_time_path("./out/samples_out_rem_time.csv");
    write_csv_rem_time(out_rem_time_path, rem_time_ress);

    return 0;
}
