#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

#include "BmiLogic.h"
#include "Imputation.h"
#include "SHealth.h"
#include "Statistics.h"
#include "TestFixtures.h"

namespace {

constexpr double kRatioEpsilon = 1e-5;

}  // namespace

TEST(BmiLogicTest, ClassifyBoundaryValues) {
    EXPECT_EQ(bmi::classifyBmi(18.5), bmi::BmiCategory::Underweight);
    EXPECT_EQ(bmi::classifyBmi(18.5001), bmi::BmiCategory::Normal);
    EXPECT_EQ(bmi::classifyBmi(22.999), bmi::BmiCategory::Normal);
    EXPECT_EQ(bmi::classifyBmi(23.0), bmi::BmiCategory::Overweight);
    EXPECT_EQ(bmi::classifyBmi(25.0), bmi::BmiCategory::Obesity);
}

TEST(ImputationTest, ImputeMissingHeightsReplacesZero) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 170.0),
        makeRecord(2, 27, 65.0, 0.0),
        makeRecord(3, 28, 80.0, 180.0),
    };
    imputation::imputeMissingHeights(records);
    EXPECT_DOUBLE_EQ(records[1].height, 175.0);
}

TEST(ImputationTest, ImputeMissingHeightsThrowsWhenNoValidHeight) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 0.0),
        makeRecord(2, 27, 65.0, 0.0),
    };
    EXPECT_THROW(imputation::imputeMissingHeights(records), std::runtime_error);
}

TEST(SHealthTest, EmptyAgeGroupReturnsZeroRatios) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 70.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto dist = shealth.distributionForAgeGroup(bmi::AgeGroup::Decade20);
    ASSERT_TRUE(dist.has_value());
    EXPECT_NEAR(dist->underweight, 0.0, kRatioEpsilon);
    EXPECT_NEAR(dist->normal, 0.0, kRatioEpsilon);
    EXPECT_NEAR(dist->overweight, 0.0, kRatioEpsilon);
    EXPECT_NEAR(dist->obesity, 0.0, kRatioEpsilon);
}

TEST(SHealthTest, FilterNormalUserIdsPreservesCsvOrder) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(300, 25, 58.0, 170.0),
        makeRecord(100, 25, 50.0, 170.0),
        makeRecord(200, 25, 63.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto ids = shealth.filterNormalUserIds();
    ASSERT_EQ(ids.size(), 2u);
    EXPECT_EQ(ids[0], 300);
    EXPECT_EQ(ids[1], 200);
}

TEST(SHealthTest, OverallPopulationIncludesAllAges) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 50.0, 160.0),
        makeRecord(2, 25, 70.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto dist = shealth.overallPopulationRatios();
    ASSERT_TRUE(dist.has_value());
    const double sum = dist->underweight + dist->normal + dist->overweight + dist->obesity;
    EXPECT_NEAR(sum, 100.0, 1e-3);
}

TEST(SHealthGoldenTest, ShealthDatSnapshot) {
    SHealth shealth;
    ASSERT_GT(shealth.calculateBmi("shealth.dat"), 0);

    const struct Expected {
        bmi::AgeGroup group;
        double underweight;
        double normal;
        double overweight;
        double obesity;
    } expected[] = {
        {bmi::AgeGroup::Decade20, 3.511053, 23.797139, 11.833550, 60.858257},
        {bmi::AgeGroup::Decade30, 1.863354, 15.527950, 10.062112, 72.546584},
        {bmi::AgeGroup::Decade40, 0.521512, 10.039113, 9.126467, 80.312907},
        {bmi::AgeGroup::Decade50, 2.181401, 12.629162, 9.988519, 75.200918},
        {bmi::AgeGroup::Decade60, 0.862895, 8.533078, 10.642378, 79.961649},
        {bmi::AgeGroup::Decade70, 0.529101, 12.345679, 10.758377, 76.366843},
    };

    for (const auto& row : expected) {
        const auto dist = shealth.distributionForAgeGroup(row.group);
        ASSERT_TRUE(dist.has_value());
        EXPECT_NEAR(dist->underweight, row.underweight, kRatioEpsilon);
        EXPECT_NEAR(dist->normal, row.normal, kRatioEpsilon);
        EXPECT_NEAR(dist->overweight, row.overweight, kRatioEpsilon);
        EXPECT_NEAR(dist->obesity, row.obesity, kRatioEpsilon);
    }
}
