#pragma once

#include "IHealthRecordReader.h"

#include <string>

class CsvHealthRecordReader final : public IHealthRecordReader {
public:
    explicit CsvHealthRecordReader(std::string filename);

    bool read(std::vector<bmi::HealthRecord>& out) override;
    int skippedLineCount() const { return skippedLines_; }

private:
    std::string filename_;
    int skippedLines_ = 0;

    static std::vector<std::string> split(const std::string& line, char delimiter);
};
