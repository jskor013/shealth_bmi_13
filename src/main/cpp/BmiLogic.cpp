#include "BmiLogic.h"

namespace bmi {

double& BmiDistribution::at(BmiCategory category) {
    switch (category) {
        case BmiCategory::Underweight:
            return underweight;
        case BmiCategory::Normal:
            return normal;
        case BmiCategory::Overweight:
            return overweight;
        case BmiCategory::Obesity:
            return obesity;
    }
    return underweight;
}

double BmiDistribution::get(BmiCategory category) const {
    switch (category) {
        case BmiCategory::Underweight:
            return underweight;
        case BmiCategory::Normal:
            return normal;
        case BmiCategory::Overweight:
            return overweight;
        case BmiCategory::Obesity:
            return obesity;
    }
    return 0.0;
}

BmiCategory classifyBmi(double bmi) {
    if (bmi <= kUnderweightMax) {
        return BmiCategory::Underweight;
    }
    if (bmi < kNormalMax) {
        return BmiCategory::Normal;
    }
    if (bmi < kOverweightMax) {
        return BmiCategory::Overweight;
    }
    return BmiCategory::Obesity;
}

double computeBmi(double weightKg, double heightCm) {
    const double heightM = heightCm / 100.0;
    return weightKg / (heightM * heightM);
}

bool inAgeDecade(int age, int decadeStart) {
    return age >= decadeStart && age < decadeStart + kAgeDecadeStep;
}

int decadeStart(AgeGroup group) {
    return static_cast<int>(group);
}

std::size_t ageGroupIndex(AgeGroup group) {
    return static_cast<std::size_t>((decadeStart(group) - kMinAgeDecade) / kAgeDecadeStep);
}

std::optional<AgeGroup> ageGroupFromDecade(int decade) {
    switch (decade) {
        case 20:
            return AgeGroup::Decade20;
        case 30:
            return AgeGroup::Decade30;
        case 40:
            return AgeGroup::Decade40;
        case 50:
            return AgeGroup::Decade50;
        case 60:
            return AgeGroup::Decade60;
        case 70:
            return AgeGroup::Decade70;
        default:
            return std::nullopt;
    }
}

std::optional<BmiCategory> categoryFromLegacyType(int type) {
    switch (type) {
        case 100:
            return BmiCategory::Underweight;
        case 200:
            return BmiCategory::Normal;
        case 300:
            return BmiCategory::Overweight;
        case 400:
            return BmiCategory::Obesity;
        default:
            return std::nullopt;
    }
}

}  // namespace bmi
