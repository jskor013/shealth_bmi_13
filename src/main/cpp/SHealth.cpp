#include "SHealth.h"

#include "BmiLogic.h"
#include "CsvHealthRecordReader.h"

#include <iostream>

SHealth::SHealth() = default;

SHealth::SHealth(std::unique_ptr<IHealthRecordReader> reader)
    : reader_(std::move(reader)) {}

int SHealth::calculateBmi(const std::string& filename) {
    reader_ = std::make_unique<CsvHealthRecordReader>(filename);
    return loadAndAnalyze();
}

int SHealth::loadAndAnalyze() {
    if (!loadRecords()) {
        return 0;
    }
    imputeMissingWeights();
    computeBmis();
    aggregateByAgeGroup();
    return static_cast<int>(records_.size());
}

bool SHealth::loadRecords() {
    records_.clear();
    if (!reader_) {
        std::cerr << "No health record reader configured." << std::endl;
        return false;
    }
    return reader_->read(records_);
}

void SHealth::imputeMissingWeights() {
    for (int decade = bmi::kMinAgeDecade; decade <= bmi::kMaxAgeDecade; decade += bmi::kAgeDecadeStep) {
        double sum = 0.0;
        int ageCount = 0;
        for (const auto& record : records_) {
            if (bmi::inAgeDecade(record.age, decade) && record.weight != 0.0) {
                sum += record.weight;
                ++ageCount;
            }
        }
        if (ageCount == 0) {
            continue;
        }
        const double average = sum / ageCount;
        for (auto& record : records_) {
            if (bmi::inAgeDecade(record.age, decade) && record.weight == 0.0) {
                record.weight = average;
            }
        }
    }
}

void SHealth::computeBmis() {
    for (auto& record : records_) {
        record.bmi = bmi::computeBmi(record.weight, record.height);
    }
}

void SHealth::aggregateByAgeGroup() {
    distributions_ = {};

    for (int decade = bmi::kMinAgeDecade; decade <= bmi::kMaxAgeDecade; decade += bmi::kAgeDecadeStep) {
        const auto group = bmi::ageGroupFromDecade(decade);
        if (!group) {
            continue;
        }

        std::array<int, 4> counts{};
        int total = 0;
        for (const auto& record : records_) {
            if (!bmi::inAgeDecade(record.age, decade)) {
                continue;
            }
            ++total;
            const bmi::BmiCategory category = bmi::classifyBmi(record.bmi);
            switch (category) {
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

        if (total == 0) {
            continue;
        }

        const std::size_t index = bmi::ageGroupIndex(*group);
        auto& dist = distributions_[index];
        dist.underweight = static_cast<double>(counts[0]) * 100.0 / total;
        dist.normal = static_cast<double>(counts[1]) * 100.0 / total;
        dist.overweight = static_cast<double>(counts[2]) * 100.0 / total;
        dist.obesity = static_cast<double>(counts[3]) * 100.0 / total;
    }
}

double SHealth::getBmiRatio(int ageClass, int type) const {
    const auto group = bmi::ageGroupFromDecade(ageClass);
    const auto category = bmi::categoryFromLegacyType(type);
    if (!group || !category) {
        return 0.0;
    }
    const auto ratio = getRatio(*group, *category);
    return ratio.value_or(0.0);
}

std::optional<double> SHealth::getRatio(bmi::AgeGroup group, bmi::BmiCategory category) const {
    switch (category) {
        case bmi::BmiCategory::Underweight:
        case bmi::BmiCategory::Normal:
        case bmi::BmiCategory::Overweight:
        case bmi::BmiCategory::Obesity:
            break;
        default:
            return std::nullopt;
    }

    const std::size_t index = bmi::ageGroupIndex(group);
    if (index >= distributions_.size()) {
        return std::nullopt;
    }
    return distributions_[index].get(category);
}

std::optional<bmi::BmiDistribution> SHealth::distributionForAgeGroup(bmi::AgeGroup group) const {
    const std::size_t index = bmi::ageGroupIndex(group);
    if (index >= distributions_.size()) {
        return std::nullopt;
    }
    return distributions_[index];
}

void SHealth::imputeMissingHeights() {
    // Stub for README step-4: height-0 imputation by age-group average.
}

std::vector<int> SHealth::filterNormalUserIds() const {
    // Stub for README step-4: return user IDs in normal BMI range.
    return {};
}

std::optional<bmi::BmiDistribution> SHealth::overallPopulationRatios() const {
    // Stub for README step-4: overall category ratios across all users.
    return std::nullopt;
}
