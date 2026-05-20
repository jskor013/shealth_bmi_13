#include "SHealth.h"

#include "BmiLogic.h"
#include "CsvHealthRecordReader.h"
#include "Imputation.h"
#include "Statistics.h"

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
    imputation::imputeMissingWeights(records_);
    imputation::imputeMissingHeights(records_);
    computeBmis();
    distributions_ = statistics::aggregateByAgeGroup(records_);
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

void SHealth::computeBmis() {
    for (auto& record : records_) {
        record.bmi = bmi::computeBmi(record.weight, record.height);
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
    imputation::imputeMissingHeights(records_);
}

std::vector<int> SHealth::filterNormalUserIds() const {
    std::vector<int> ids;
    ids.reserve(records_.size());
    for (const auto& record : records_) {
        if (bmi::classifyBmi(record.bmi) == bmi::BmiCategory::Normal) {
            ids.push_back(record.id);
        }
    }
    return ids;
}

std::optional<bmi::BmiDistribution> SHealth::overallPopulationRatios() const {
    return statistics::overallPopulationRatios(records_);
}
