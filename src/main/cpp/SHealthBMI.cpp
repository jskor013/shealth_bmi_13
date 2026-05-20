#include "SHealth.h"

#include "BmiLogic.h"
#include "SHealthTypes.h"

#include <cstdio>

namespace {

void printAgeGroupReport(const SHealth& shealth, bmi::AgeGroup group) {
    const int decade = bmi::decadeStart(group);
    printf("%d - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           decade,
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Underweight)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Normal)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Overweight)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Obesity)));
}

}  // namespace

int main() {
    SHealth shealth;
    if (shealth.calculateBmi("shealth.dat") <= 0) {
        return 1;
    }

    const bmi::AgeGroup groups[] = {
        bmi::AgeGroup::Decade20, bmi::AgeGroup::Decade30, bmi::AgeGroup::Decade40,
        bmi::AgeGroup::Decade50, bmi::AgeGroup::Decade60, bmi::AgeGroup::Decade70,
    };

    for (const auto group : groups) {
        printAgeGroupReport(shealth, group);
    }

    return 0;
}
