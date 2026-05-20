#include "Imputation.h"

#include "BmiLogic.h"

#include <stdexcept>
#include <string>

namespace imputation {

void imputeMissingWeights(std::vector<bmi::HealthRecord>& records) {
    for (int decade = bmi::kMinAgeDecade; decade <= bmi::kMaxAgeDecade; decade += bmi::kAgeDecadeStep) {
        double sum = 0.0;
        int validCount = 0;
        for (const auto& record : records) {
            if (bmi::inAgeDecade(record.age, decade) && record.weight != 0.0) {
                sum += record.weight;
                ++validCount;
            }
        }
        if (validCount == 0) {
            continue;
        }
        const double average = sum / validCount;
        for (auto& record : records) {
            if (bmi::inAgeDecade(record.age, decade) && record.weight == 0.0) {
                record.weight = average;
            }
        }
    }
}

void imputeMissingHeights(std::vector<bmi::HealthRecord>& records) {
    for (int decade = bmi::kMinAgeDecade; decade <= bmi::kMaxAgeDecade; decade += bmi::kAgeDecadeStep) {
        double sum = 0.0;
        int validCount = 0;
        bool needsImputation = false;

        for (const auto& record : records) {
            if (!bmi::inAgeDecade(record.age, decade)) {
                continue;
            }
            if (record.height != 0.0) {
                sum += record.height;
                ++validCount;
            } else {
                needsImputation = true;
            }
        }

        if (!needsImputation) {
            continue;
        }

        if (validCount == 0) {
            throw std::runtime_error("No valid height in age group " + std::to_string(decade));
        }

        const double average = sum / validCount;
        for (auto& record : records) {
            if (bmi::inAgeDecade(record.age, decade) && record.height == 0.0) {
                record.height = average;
            }
        }
    }
}

}  // namespace imputation
