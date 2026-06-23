#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include "AP_BattEkfImpl.h"
#include "CsvReader.h"

namespace fs = std::filesystem;

int main(void)
{
    auto csv_data = CsvReader::read("./record/output_data.csv", ',');
    if (csv_data.empty()) {
        printf("[ERROR] no csv data\n");
        return 1;
    }
    size_t head_row_cnt = 1;

    AP_BattEkfImpl batt_ekf = {};
    std::vector<EkfRes> ress(csv_data.size() - head_row_cnt);

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
        }
    }

    fs::path out_path("./out/samples_out.csv");
    std::ofstream outfile(out_path);
    if (!outfile.is_open()) {
        fs::remove(out_path);
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

    return 0;
}
