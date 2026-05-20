#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "BmiLogic.h"
#include "CsvHealthRecordReader.h"
#include "Imputation.h"
#include "SHealth.h"
#include "Statistics.h"
#include "TestFixtures.h"

namespace {

constexpr double kRatioEpsilon = 1e-5;
constexpr double kBmiEpsilon = 1e-4;
constexpr double kSumEpsilon = 1e-3;

void expectDistributionSumsTo100(const bmi::BmiDistribution& dist) {
    const double sum = dist.underweight + dist.normal + dist.overweight + dist.obesity;
    EXPECT_NEAR(sum, 100.0, kSumEpsilon);
}

bool containsSubstring(const char* haystack, const std::string& needle) {
    return std::string(haystack).find(needle) != std::string::npos;
}

}  // namespace

TEST(BmiLogicTest, ClassifyBoundaryValues_TC_F_04a) {
    EXPECT_EQ(bmi::classifyBmi(18.5), bmi::BmiCategory::Underweight);
    EXPECT_EQ(bmi::classifyBmi(18.5001), bmi::BmiCategory::Normal);
    EXPECT_EQ(bmi::classifyBmi(22.999), bmi::BmiCategory::Normal);
    EXPECT_EQ(bmi::classifyBmi(23.0), bmi::BmiCategory::Overweight);
    EXPECT_EQ(bmi::classifyBmi(25.0), bmi::BmiCategory::Obesity);
}

TEST(ImputationTest, ImputeMissingHeightsReplacesZero_TC_F_03a) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 170.0),
        makeRecord(2, 27, 65.0, 0.0),
        makeRecord(3, 28, 80.0, 180.0),
    };
    imputation::imputeMissingHeights(records);
    EXPECT_DOUBLE_EQ(records[1].height, 175.0);
}

TEST(ImputationTest, ImputeMissingHeightsThrowsWhenNoValidHeight_TC_F_03b) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 0.0),
        makeRecord(2, 27, 65.0, 0.0),
    };
    EXPECT_THROW(imputation::imputeMissingHeights(records), std::runtime_error);
}

TEST(ImputationTest, ImputeMissingHeightsExceptionMessage_TC_F_03c) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 0.0),
        makeRecord(2, 27, 65.0, 0.0),
    };
    try {
        imputation::imputeMissingHeights(records);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& ex) {
        EXPECT_TRUE(containsSubstring(ex.what(), "No valid height"));
        EXPECT_TRUE(containsSubstring(ex.what(), "20"));
    }
}

TEST(ImputationTest, ImputeMissingHeightsSingleValidKey_TC_F_03d) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 170.0),
        makeRecord(2, 25, 65.0, 0.0),
    };
    imputation::imputeMissingHeights(records);
    EXPECT_DOUBLE_EQ(records[1].height, 170.0);
}

TEST(ImputationTest, ImputeMissingHeightsNoOpWhenNoZeros_TC_F_03e) {
    const std::vector<bmi::HealthRecord> before = {
        makeRecord(1, 25, 70.0, 170.0),
        makeRecord(2, 27, 65.0, 180.0),
    };
    std::vector<bmi::HealthRecord> records = before;
    imputation::imputeMissingHeights(records);
    ASSERT_EQ(records.size(), before.size());
    for (std::size_t i = 0; i < records.size(); ++i) {
        EXPECT_EQ(records[i].id, before[i].id);
        EXPECT_EQ(records[i].age, before[i].age);
        EXPECT_DOUBLE_EQ(records[i].weight, before[i].weight);
        EXPECT_DOUBLE_EQ(records[i].height, before[i].height);
    }
}

TEST(ImputationTest, ImputeMissingHeightsIndependentPerDecade_TC_F_03f) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 0.0),
        makeRecord(2, 25, 65.0, 170.0),
        makeRecord(3, 35, 80.0, 0.0),
        makeRecord(4, 35, 75.0, 165.0),
    };
    imputation::imputeMissingHeights(records);
    EXPECT_DOUBLE_EQ(records[0].height, 170.0);
    EXPECT_DOUBLE_EQ(records[2].height, 165.0);
}

TEST(SHealthTest, LoadAndAnalyzeWeightThenHeightImputation_TC_F_03g) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, 0.0, 0.0),
        makeRecord(2, 25, 60.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto dist = shealth.overallPopulationRatios();
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
    const auto ids = shealth.filterNormalUserIds();
    EXPECT_EQ(ids.size(), 2u);
    for (const int id : ids) {
        EXPECT_TRUE(id == 1 || id == 2);
    }
}

TEST(SHealthTest, LoadAndAnalyzeThrowsWhenNoValidHeight_TC_F_03h) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, 70.0, 0.0),
        makeRecord(2, 27, 65.0, 0.0),
    }));
    try {
        shealth.loadAndAnalyze();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& ex) {
        EXPECT_TRUE(containsSubstring(ex.what(), "No valid height"));
        EXPECT_TRUE(containsSubstring(ex.what(), "20"));
    }
}

TEST(SHealthTest, ImputeMissingHeightsPublicApi_TC_F_03i) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 25, 70.0, 170.0),
        makeRecord(2, 27, 65.0, 0.0),
        makeRecord(3, 28, 80.0, 180.0),
    };
    imputation::imputeMissingWeights(records);
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(records));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    EXPECT_NO_THROW(shealth.imputeMissingHeights());
    const auto dist = shealth.distributionForAgeGroup(bmi::AgeGroup::Decade20);
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
}

TEST(SHealthTest, EmptyAgeGroupReturnsZeroRatios_TC_F_02a) {
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

TEST(SHealthTest, EmptyAgeGroupLoadAndAnalyzeNoThrow_TC_F_02b) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 70.0, 170.0),
    }));
    EXPECT_NO_THROW({
        const int count = shealth.loadAndAnalyze();
        EXPECT_GT(count, 0);
    });
}

TEST(SHealthTest, AgeGroupWithMembersSumsTo100_TC_F_02c) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, 58.0, 170.0),
        makeRecord(2, 28, 63.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto dist = shealth.distributionForAgeGroup(bmi::AgeGroup::Decade20);
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
    EXPECT_NEAR(dist->normal, 100.0, kRatioEpsilon);
}

TEST(SHealthTest, FilterNormalUserIdsPreservesCsvOrder_TC_F_04c) {
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

TEST(SHealthTest, FilterNormalUserIdsBoundaryCategories_TC_F_04b) {
    constexpr double height = 170.0;
    const double underWeight = 50.0;
    const double normalWeight = 63.0;
    const double overWeight = 70.0;

    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, underWeight, height),
        makeRecord(2, 25, normalWeight, height),
        makeRecord(3, 25, overWeight, height),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto ids = shealth.filterNormalUserIds();
    ASSERT_EQ(ids.size(), 1u);
    EXPECT_EQ(ids[0], 2);
}

TEST(SHealthTest, FilterNormalUserIdsBeforeAnalyzeEmpty_TC_F_04d) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, 63.0, 170.0),
    }));
    const auto ids = shealth.filterNormalUserIds();
    EXPECT_TRUE(ids.empty());
}

TEST(SHealthTest, FilterNormalUserIdsNoneWhenAllObese_TC_F_04f) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 25, 90.0, 170.0),
        makeRecord(2, 26, 95.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto ids = shealth.filterNormalUserIds();
    EXPECT_TRUE(ids.empty());
}

TEST(SHealthTest, OverallPopulationIncludesAllAges_TC_F_05a) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 50.0, 160.0),
        makeRecord(2, 25, 70.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto dist = shealth.overallPopulationRatios();
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
}

TEST(SHealthTest, OverallPopulationEmptyReturnsNullopt_TC_F_05b) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{}));
    EXPECT_EQ(shealth.loadAndAnalyze(), 0);
    EXPECT_FALSE(shealth.overallPopulationRatios().has_value());
}

TEST(SHealthTest, OverallPopulationIncludesAge80_TC_F_05c) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 80, 90.0, 170.0),
        makeRecord(2, 25, 70.0, 170.0),
    }));
    ASSERT_EQ(shealth.loadAndAnalyze(), 2);
    const auto dist = shealth.overallPopulationRatios();
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
    EXPECT_NEAR(dist->obesity, 50.0, kRatioEpsilon);
    EXPECT_NEAR(dist->overweight, 50.0, kRatioEpsilon);
}

TEST(SHealthTest, OverallVsAgeGroupDenominatorDifference_TC_F_05d) {
    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 46.0, 160.0),
        makeRecord(2, 25, 90.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto overall = shealth.overallPopulationRatios();
    const auto decade20 = shealth.distributionForAgeGroup(bmi::AgeGroup::Decade20);
    ASSERT_TRUE(overall.has_value());
    ASSERT_TRUE(decade20.has_value());
    EXPECT_NEAR(overall->underweight, 50.0, kRatioEpsilon);
    EXPECT_NEAR(overall->obesity, 50.0, kRatioEpsilon);
    EXPECT_NEAR(decade20->obesity, 100.0, kRatioEpsilon);
}

TEST(SHealthTest, GetBmiRatioLegacyCompatibility_TC_F_01b) {
    SHealth shealth;
    ASSERT_GT(shealth.calculateBmi("shealth.dat"), 0);

    const auto dist = shealth.distributionForAgeGroup(bmi::AgeGroup::Decade20);
    ASSERT_TRUE(dist.has_value());
    EXPECT_NEAR(shealth.getBmiRatio(20, 100), dist->underweight, kRatioEpsilon);
    EXPECT_NEAR(shealth.getBmiRatio(20, 200), dist->normal, kRatioEpsilon);
    EXPECT_NEAR(shealth.getBmiRatio(20, 300), dist->overweight, kRatioEpsilon);
    EXPECT_NEAR(shealth.getBmiRatio(20, 400), dist->obesity, kRatioEpsilon);
}

TEST(StatisticsTest, OverallPopulationRatiosUnit_TC_F_05e) {
    std::vector<bmi::HealthRecord> records = {
        makeRecord(1, 19, 50.0, 160.0),
        makeRecord(2, 25, 70.0, 170.0),
    };
    for (auto& record : records) {
        record.bmi = bmi::computeBmi(record.weight, record.height);
    }
    const auto dist = statistics::overallPopulationRatios(records);
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);

    SHealth shealth(std::make_unique<VectorHealthRecordReader>(std::vector<bmi::HealthRecord>{
        makeRecord(1, 19, 50.0, 160.0),
        makeRecord(2, 25, 70.0, 170.0),
    }));
    ASSERT_GT(shealth.loadAndAnalyze(), 0);
    const auto integrated = shealth.overallPopulationRatios();
    ASSERT_TRUE(integrated.has_value());
    EXPECT_NEAR(dist->underweight, integrated->underweight, kRatioEpsilon);
    EXPECT_NEAR(dist->normal, integrated->normal, kRatioEpsilon);
    EXPECT_NEAR(dist->overweight, integrated->overweight, kRatioEpsilon);
    EXPECT_NEAR(dist->obesity, integrated->obesity, kRatioEpsilon);
}

TEST(CsvReaderTest, ParsesIdColumn_TC_F_04e) {
    CsvHealthRecordReader reader("src/test/fixtures/csv_id_parse.csv");
    std::vector<bmi::HealthRecord> records;
    ASSERT_TRUE(reader.read(records));
    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(records[0].id, 42);
    EXPECT_EQ(records[0].age, 25);
    EXPECT_DOUBLE_EQ(records[0].weight, 70.0);
    EXPECT_DOUBLE_EQ(records[0].height, 170.0);
}

TEST(SHealthGoldenTest, ShealthDatSnapshot_TC_G_01) {
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
        expectDistributionSumsTo100(*dist);
    }
}

TEST(SHealthGoldenTest, LoadHeightImputeOkCsv_TC_F_03g_csv) {
    SHealth shealth;
    ASSERT_GT(shealth.calculateBmi("src/test/fixtures/height_impute_ok.csv"), 0);
    const auto dist = shealth.overallPopulationRatios();
    ASSERT_TRUE(dist.has_value());
    expectDistributionSumsTo100(*dist);
}
