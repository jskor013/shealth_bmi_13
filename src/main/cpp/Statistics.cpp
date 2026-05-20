#include "Statistics.h"

#include "BmiLogic.h"

namespace statistics {

namespace {

void countCategory(const bmi::HealthRecord& record, std::array<int, 4>& counts) {
    switch (bmi::classifyBmi(record.bmi)) {
        case bmi::BmiCategory::Underweight:
            ++counts[0];
            break;
        case bmi::BmiCategory::Normal:
            ++counts[1];
            break;
        case bmi::BmiCategory::Overweight:
            ++counts[2];
            break;
        case bmi::BmiCategory::Obesity:
            ++counts[3];
            break;
    }
}

bmi::BmiDistribution ratiosFromCounts(const std::array<int, 4>& counts, int total) {
    bmi::BmiDistribution dist;
    dist.underweight = static_cast<double>(counts[0]) * 100.0 / total;
    dist.normal = static_cast<double>(counts[1]) * 100.0 / total;
    dist.overweight = static_cast<double>(counts[2]) * 100.0 / total;
    dist.obesity = static_cast<double>(counts[3]) * 100.0 / total;
    return dist;
}

}  // namespace

AgeGroupDistributions aggregateByAgeGroup(const std::vector<bmi::HealthRecord>& records) {
    AgeGroupDistributions distributions{};

    for (int decade = bmi::kMinAgeDecade; decade <= bmi::kMaxAgeDecade; decade += bmi::kAgeDecadeStep) {
        const auto group = bmi::ageGroupFromDecade(decade);
        if (!group) {
            continue;
        }

        std::array<int, 4> counts{};
        int total = 0;
        for (const auto& record : records) {
            if (!bmi::inAgeDecade(record.age, decade)) {
                continue;
            }
            ++total;
            countCategory(record, counts);
        }

        if (total == 0) {
            continue;
        }

        const std::size_t index = bmi::ageGroupIndex(*group);
        distributions[index] = ratiosFromCounts(counts, total);
    }

    return distributions;
}

std::optional<bmi::BmiDistribution> overallPopulationRatios(
    const std::vector<bmi::HealthRecord>& records) {
    if (records.empty()) {
        return std::nullopt;
    }

    std::array<int, 4> counts{};
    for (const auto& record : records) {
        countCategory(record, counts);
    }

    return ratiosFromCounts(counts, static_cast<int>(records.size()));
}

}  // namespace statistics
