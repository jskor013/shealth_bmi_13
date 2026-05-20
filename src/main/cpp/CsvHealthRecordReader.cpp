#include "CsvHealthRecordReader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

CsvHealthRecordReader::CsvHealthRecordReader(std::string filename)
    : filename_(std::move(filename)) {}

bool CsvHealthRecordReader::read(std::vector<bmi::HealthRecord>& out) {
    out.clear();
    out.reserve(bmi::kMaxRecords);
    skippedLines_ = 0;

    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename_ << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);  // header
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        try {
            const std::vector<std::string> tokens = split(line, ',');
            if (tokens.size() < 4) {
                ++skippedLines_;
                continue;
            }

            if (out.size() >= bmi::kMaxRecords) {
                std::cerr << "Record limit reached: " << bmi::kMaxRecords << std::endl;
                break;
            }

            bmi::HealthRecord record;
            record.id = std::stoi(tokens[0]);
            record.age = std::stoi(tokens[1]);
            record.weight = std::stod(tokens[2]);
            record.height = std::stod(tokens[3]);
            out.push_back(record);
        } catch (const std::invalid_argument&) {
            ++skippedLines_;
        } catch (const std::out_of_range&) {
            ++skippedLines_;
        }
    }

    return true;
}

std::vector<std::string> CsvHealthRecordReader::split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
