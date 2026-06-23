#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class CsvReader
{
   public:
    // Reads a CSV file and returns a 2D vector of strings (Rows x Columns)
    static std::vector<std::vector<std::string>> read(const std::string& filename, char delimiter = ',')
    {
        std::vector<std::vector<std::string>> data;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return data;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Handle potential Windows carriage return (\r\n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            data.push_back(parseLine(line, delimiter));
        }

        file.close();
        return data;
    }

   private:
    // Parses a single line while respecting quotes wrapping strings with commas
    static std::vector<std::string> parseLine(const std::string& line, char delimiter)
    {
        std::vector<std::string> row;
        std::string cell;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); ++i) {
            char ch = line[i];

            if (ch == '"') {
                // Handle escaped quotes ("") inside a quoted field
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    cell += '"';
                    ++i;  // Skip the escaped quote
                } else {
                    inQuotes = !inQuotes;  // Toggle quote state
                }
            } else if (ch == delimiter && !inQuotes) {
                row.push_back(cell);
                cell.clear();
            } else {
                cell += ch;
            }
        }
        // Add the last cell
        row.push_back(cell);
        return row;
    }
};
