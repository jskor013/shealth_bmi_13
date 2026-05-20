#pragma once

#include "IHealthRecordReader.h"
#include "SHealthTypes.h"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class SHealth {
public:
    SHealth();
    explicit SHealth(std::unique_ptr<IHealthRecordReader> reader);

    [[nodiscard]] int calculateBmi(const std::string& filename);
    [[nodiscard]] int loadAndAnalyze();

    // Deprecated: prefer getRatio(AgeGroup, BmiCategory).
    double getBmiRatio(int ageClass, int type) const;
    [[nodiscard]] std::optional<double> getRatio(bmi::AgeGroup group, bmi::BmiCategory category) const;

    // Reserved for README step-4 features (stub signatures).
    [[nodiscard]] std::optional<bmi::BmiDistribution> distributionForAgeGroup(bmi::AgeGroup group) const;
    void imputeMissingHeights();
    std::vector<int> filterNormalUserIds() const;
    [[nodiscard]] std::optional<bmi::BmiDistribution> overallPopulationRatios() const;

private:
    std::unique_ptr<IHealthRecordReader> reader_;
    std::vector<bmi::HealthRecord> records_;
    std::array<bmi::BmiDistribution, bmi::kAgeGroupCount> distributions_{};

    bool loadRecords();
    void computeBmis();
};
