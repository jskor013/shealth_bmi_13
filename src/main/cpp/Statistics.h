#pragma once

#include "SHealthTypes.h"

#include <array>
#include <optional>
#include <vector>

namespace statistics {

using AgeGroupDistributions = std::array<bmi::BmiDistribution, bmi::kAgeGroupCount>;

[[nodiscard]] AgeGroupDistributions aggregateByAgeGroup(const std::vector<bmi::HealthRecord>& records);
[[nodiscard]] std::optional<bmi::BmiDistribution> overallPopulationRatios(
    const std::vector<bmi::HealthRecord>& records);

}  // namespace statistics
