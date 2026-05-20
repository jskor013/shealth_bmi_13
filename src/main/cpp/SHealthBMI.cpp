#include "SHealth.h"

#include "BmiLogic.h"
#include "SHealthTypes.h"

#include <cstdio>

namespace {

void printDistribution(int label, const bmi::BmiDistribution& dist) {
    printf("%d - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           label,
           dist.underweight,
           dist.normal,
           dist.overweight,
           dist.obesity);
}

void printAgeGroupReport(const SHealth& shealth, bmi::AgeGroup group) {
    const int decade = bmi::decadeStart(group);
    const auto dist = shealth.distributionForAgeGroup(group);
    if (dist) {
        printDistribution(decade, *dist);
        return;
    }
    printf("%d - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           decade,
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Underweight)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Normal)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Overweight)),
           shealth.getBmiRatio(decade, static_cast<int>(bmi::BmiCategory::Obesity)));
}

void printOverallPopulation(const SHealth& shealth) {
    const auto dist = shealth.overallPopulationRatios();
    if (!dist) {
        printf("overall - no records\n");
        return;
    }
    printf("overall - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           dist->underweight,
           dist->normal,
           dist->overweight,
           dist->obesity);
}

void printNormalUserIds(const SHealth& shealth) {
    const auto ids = shealth.filterNormalUserIds();
    printf("normal users count = %zu\n", ids.size());
    constexpr std::size_t kSampleLimit = 20;
    const std::size_t showCount = ids.size() < kSampleLimit ? ids.size() : kSampleLimit;
    for (std::size_t i = 0; i < showCount; ++i) {
        printf("  id = %d\n", ids[i]);
    }
    if (ids.size() > kSampleLimit) {
        printf("  ... (%zu more)\n", ids.size() - kSampleLimit);
    }
}

}  // namespace

int main() {
    SHealth shealth;
    try {
        if (shealth.calculateBmi("shealth.dat") <= 0) {
            return 1;
        }
    } catch (const std::exception& ex) {
        fprintf(stderr, "%s\n", ex.what());
        return 1;
    }

    const bmi::AgeGroup groups[] = {
        bmi::AgeGroup::Decade20, bmi::AgeGroup::Decade30, bmi::AgeGroup::Decade40,
        bmi::AgeGroup::Decade50, bmi::AgeGroup::Decade60, bmi::AgeGroup::Decade70,
    };

    for (const auto group : groups) {
        printAgeGroupReport(shealth, group);
    }

    printOverallPopulation(shealth);
    printNormalUserIds(shealth);

    return 0;
}
