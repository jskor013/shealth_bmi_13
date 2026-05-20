#pragma once

#include "IHealthRecordReader.h"
#include "SHealthTypes.h"

#include <utility>
#include <vector>

class VectorHealthRecordReader final : public IHealthRecordReader {
public:
    explicit VectorHealthRecordReader(std::vector<bmi::HealthRecord> data)
        : data_(std::move(data)) {}

    bool read(std::vector<bmi::HealthRecord>& out) override {
        out = data_;
        return true;
    }

private:
    std::vector<bmi::HealthRecord> data_;
};

inline bmi::HealthRecord makeRecord(int id, int age, double weight, double height) {
    bmi::HealthRecord record;
    record.id = id;
    record.age = age;
    record.weight = weight;
    record.height = height;
    return record;
}
