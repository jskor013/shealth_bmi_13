#pragma once

#include "SHealthTypes.h"

#include <vector>

namespace imputation {

void imputeMissingWeights(std::vector<bmi::HealthRecord>& records);
void imputeMissingHeights(std::vector<bmi::HealthRecord>& records);

}  // namespace imputation
