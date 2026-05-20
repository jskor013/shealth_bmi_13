#pragma once

#include "SHealthTypes.h"

#include <optional>

namespace bmi {

BmiCategory classifyBmi(double bmi);
double computeBmi(double weightKg, double heightCm);
bool inAgeDecade(int age, int decadeStart);
int decadeStart(AgeGroup group);
std::size_t ageGroupIndex(AgeGroup group);
std::optional<AgeGroup> ageGroupFromDecade(int decade);
std::optional<BmiCategory> categoryFromLegacyType(int type);

}  // namespace bmi
