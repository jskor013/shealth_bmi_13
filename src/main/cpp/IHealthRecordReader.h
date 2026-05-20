#pragma once

#include "SHealthTypes.h"

#include <vector>

class IHealthRecordReader {
public:
    virtual ~IHealthRecordReader() = default;
    virtual bool read(std::vector<bmi::HealthRecord>& out) = 0;
};
