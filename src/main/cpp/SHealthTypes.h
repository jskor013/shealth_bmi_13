#pragma once

#include <array>
#include <cstddef>

namespace bmi {

constexpr double kUnderweightMax = 18.5;
constexpr double kNormalMax = 23.0;
constexpr double kOverweightMax = 25.0;
constexpr std::size_t kMaxRecords = 10'000;
constexpr std::size_t kAgeGroupCount = 6;
constexpr int kMinAgeDecade = 20;
constexpr int kMaxAgeDecade = 70;
constexpr int kAgeDecadeStep = 10;

enum class BmiCategory : int {
    Underweight = 100,
    Normal = 200,
    Overweight = 300,
    Obesity = 400
};

enum class AgeGroup : int {
    Decade20 = 20,
    Decade30 = 30,
    Decade40 = 40,
    Decade50 = 50,
    Decade60 = 60,
    Decade70 = 70
};

struct HealthRecord {
    int id = 0;
    int age = 0;
    double weight = 0.0;
    double height = 0.0;
    double bmi = 0.0;
};

struct BmiDistribution {
    double underweight = 0.0;
    double normal = 0.0;
    double overweight = 0.0;
    double obesity = 0.0;

    double& at(BmiCategory category);
    double get(BmiCategory category) const;
};

}  // namespace bmi
