# SHealth BMI — 신규 기능 테스트 실행 결과 보고서

**문서 역할:** [Feature_test_plan.md](./Feature_test_plan.md) 기준 실행 결과  
**기준 문서:** [Feature_requirements_report.md](./Feature_requirements_report.md), [Feature_implementation_result_report.md](./Feature_implementation_result_report.md)  
**실행일:** 2026-05-20  
**실행 환경:** Windows 10 · CMake 4.3.1 · MinGW (ninja) · `E:/dev/shealth_bmi_13`

---

## 1. 실행 요약

| 항목 | 결과 |
|------|------|
| **빌드** | ✅ 성공 (`cmake --build build`) |
| **ctest** | ✅ **26/26 통과** (0 실패) |
| **기존 기준선 (7건)** | ✅ 유지·TC-ID 접미사 반영 |
| **신규 자동 TC** | ✅ **19건 추가** (P0·P1·P2) |
| **CLI 수동 (TC-CLI)** | 🔶 3/4 수동 확인 (TC-CLI-04는 main 고정 경로로 미실행) |

### 종합 판정

| 구분 | 판정 |
|------|------|
| **자동화 테스트 (ctest)** | ✅ **통과** — AC-07 충족 |
| **P0 TC (FR-003)** | ✅ TC-F-03c, 03f, 03g, 03h 전부 PASS |
| **P1 TC (FR-002/004/005)** | ✅ TC-F-02c, 04b, 04e, 04f, 05b~05e, 01b PASS |
| **MVP 인수선 (12건)** | ✅ 초과 달성 (26건 자동) |
| **CLI E2E (AC-09)** | 🔶 shealth.dat 기준 01~03 확인, 04는 gtest로 대체 검증 |

---

## 2. 실행 환경·명령

```powershell
cd E:\dev\shealth_bmi_13
$env:CMAKE_TLS_VERIFY = "0"   # FetchContent SSL (환경 이슈 시)
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

**CLI 수동 (TC-CLI):**

```powershell
cd E:\dev\shealth_bmi_13
.\build\SHealthBMI.exe
```

| 설정 | 값 |
|------|-----|
| ctest Working Directory | `${CMAKE_SOURCE_DIR}` (프로젝트 루트) |
| Google Test | v1.14.0 (FetchContent) |
| 테스트 실행 파일 | `build/SHealthBMITest.exe` |

---

## 3. ctest 상세 결과

**결과:** 26 tests, **100% passed**, Total Test time ≈ 1.6s

| # | gtest 이름 | TC-ID | 결과 |
|---|------------|-------|------|
| 1 | `BmiLogicTest.ClassifyBoundaryValues_TC_F_04a` | TC-F-04a | ✅ |
| 2 | `ImputationTest.ImputeMissingHeightsReplacesZero_TC_F_03a` | TC-F-03a | ✅ |
| 3 | `ImputationTest.ImputeMissingHeightsThrowsWhenNoValidHeight_TC_F_03b` | TC-F-03b | ✅ |
| 4 | `ImputationTest.ImputeMissingHeightsExceptionMessage_TC_F_03c` | TC-F-03c | ✅ |
| 5 | `ImputationTest.ImputeMissingHeightsSingleValidKey_TC_F_03d` | TC-F-03d | ✅ |
| 6 | `ImputationTest.ImputeMissingHeightsNoOpWhenNoZeros_TC_F_03e` | TC-F-03e | ✅ |
| 7 | `ImputationTest.ImputeMissingHeightsIndependentPerDecade_TC_F_03f` | TC-F-03f | ✅ |
| 8 | `SHealthTest.LoadAndAnalyzeWeightThenHeightImputation_TC_F_03g` | TC-F-03g | ✅ |
| 9 | `SHealthTest.LoadAndAnalyzeThrowsWhenNoValidHeight_TC_F_03h` | TC-F-03h | ✅ |
| 10 | `SHealthTest.ImputeMissingHeightsPublicApi_TC_F_03i` | TC-F-03i | ✅ |
| 11 | `SHealthTest.EmptyAgeGroupReturnsZeroRatios_TC_F_02a` | TC-F-02a | ✅ |
| 12 | `SHealthTest.EmptyAgeGroupLoadAndAnalyzeNoThrow_TC_F_02b` | TC-F-02b | ✅ |
| 13 | `SHealthTest.AgeGroupWithMembersSumsTo100_TC_F_02c` | TC-F-02c | ✅ |
| 14 | `SHealthTest.FilterNormalUserIdsPreservesCsvOrder_TC_F_04c` | TC-F-04c | ✅ |
| 15 | `SHealthTest.FilterNormalUserIdsBoundaryCategories_TC_F_04b` | TC-F-04b | ✅ |
| 16 | `SHealthTest.FilterNormalUserIdsBeforeAnalyzeEmpty_TC_F_04d` | TC-F-04d | ✅ |
| 17 | `SHealthTest.FilterNormalUserIdsNoneWhenAllObese_TC_F_04f` | TC-F-04f | ✅ |
| 18 | `SHealthTest.OverallPopulationIncludesAllAges_TC_F_05a` | TC-F-05a | ✅ |
| 19 | `SHealthTest.OverallPopulationEmptyReturnsNullopt_TC_F_05b` | TC-F-05b | ✅ |
| 20 | `SHealthTest.OverallPopulationIncludesAge80_TC_F_05c` | TC-F-05c | ✅ |
| 21 | `SHealthTest.OverallVsAgeGroupDenominatorDifference_TC_F_05d` | TC-F-05d | ✅ |
| 22 | `SHealthTest.GetBmiRatioLegacyCompatibility_TC_F_01b` | TC-F-01b | ✅ |
| 23 | `StatisticsTest.OverallPopulationRatiosUnit_TC_F_05e` | TC-F-05e | ✅ |
| 24 | `CsvReaderTest.ParsesIdColumn_TC_F_04e` | TC-F-04e | ✅ |
| 25 | `SHealthGoldenTest.ShealthDatSnapshot_TC_G_01` | TC-G-01 | ✅ |
| 26 | `SHealthGoldenTest.LoadHeightImputeOkCsv_TC_F_03g_csv` | TC-F-03g (CSV) | ✅ |

---

## 4. 구현·추가 산출물

### 4.1 수정·신규 파일

| 경로 | 내용 |
|------|------|
| `src/test/cpp/SHealthBMITest.cpp` | TC-F 23건 중 **자동화 가능 20건** 구현 (기존 7 + 신규 19, TC-G-01 포함 26 gtest) |
| `src/test/fixtures/csv_id_parse.csv` | TC-F-04e id 파싱 |
| `src/test/fixtures/height_impute_ok.csv` | TC-F-03g CSV 파이프라인 |
| `src/test/fixtures/height_impute_fail.csv` | TC-CLI-04 / TC-F-03h용 (gtest에서 VectorReader로 검증) |

### 4.2 계획 대비 커버리지 갱신

| 구분 | 계획 | 구현(자동) | 미구현 |
|------|------|------------|--------|
| TC-F | 23 | **22** | TC-F-01a (TC-G-01과 동일·중복), TC-F-01c (간접) |
| TC-G-01 | 1 | 1 | 0 |
| TC-CLI | 4 | 0 (수동 3건 확인) | TC-CLI-04 (CLI 진입점 미지원) |
| **합계** | 28 | **26 gtest + 3 CLI** | **TC-CLI-04 CLI 단독** |

---

## 5. FR·AC 추적 (실행 후)

| FR/AC | 대표 TC | 실행 결과 |
|-------|---------|-----------|
| FR-001 | TC-F-01b, TC-G-01 | ✅ |
| FR-002 | TC-F-02a~c, TC-G-01 | ✅ |
| FR-003 | TC-F-03a~i | ✅ |
| FR-004 | TC-F-04a~f | ✅ |
| FR-005 | TC-F-05a~e | ✅ |
| AC-01 | TC-F-03a | ✅ |
| AC-02 | TC-F-03g | ✅ |
| AC-02b | TC-F-03b,c,h | ✅ |
| AC-03 | TC-F-04b,c | ✅ |
| AC-04 | TC-F-05a | ✅ |
| AC-05 | TC-G-01, TC-F-02c | ✅ |
| AC-05b | TC-F-02a,b | ✅ |
| AC-06 | TC-G-01 | ✅ |
| AC-07 | ctest 26/26 | ✅ |
| AC-09 | TC-CLI-01~03 | 🔶 수동 확인 |

---

## 6. CLI 수동 검증 (TC-CLI)

### TC-CLI-01: 6연령대 출력 — ✅

`shealth.dat` 로드 후 exit 0, 20~70 decade 6줄 출력. 골든 값과 일치.

### TC-CLI-02: 전체 인구 4비율 — ✅

```
overall - underweight = 1.596848, normal = 13.562837, overweight = 10.389880, obesity = 74.450436
```

4비율 합 ≈ 100% (수동 합산 일치).

### TC-CLI-03: 정상 사용자 요약 — ✅

`normal users count = 654`, `id =` 샘플 20건 + `... (more)` 형식 확인.

### TC-CLI-04: Height 보정 실패 stderr·exit 1 — ⏭️

`SHealthBMI.cpp`가 `shealth.dat`만 사용. **동등 검증:** `SHealthTest.LoadAndAnalyzeThrowsWhenNoValidHeight_TC_F_03h` PASS (`No valid height`, decade `20`).

---

## 7. Definition of Done 체크

| # | 기준 | 현재 |
|---|------|------|
| DOD-01 | AC-01~AC-07 자동 TC 매핑 | ✅ (AC-09만 수동) |
| DOD-02 | TC-F P0·P1 gtest | ✅ |
| DOD-03 | ctest FAIL 없음 | ✅ 26/26 |
| DOD-04 | TC-CLI-01~03 shealth.dat 수동 | ✅ |
| DOD-05 | TC-ID ↔ gtest 이름 추적 | ✅ 접미사 `_TC_F_*` |
| DOD-06 | Q-01 vs Q-06 전용 TC 2건+ | ✅ 02a, 02b, 03b, 03c |

---

## 8. 결론 및 권장 후속

- **신규 기능 자동 테스트:** 기준선 7건 → **26건**으로 확대, `Feature_test_plan.md` P0·P1·대부분 P2 **PASS**.
- **회귀:** `shealth.dat` 골든 24비율 및 legacy `getBmiRatio` **불변** 확인.
- **권장 후속:** `SHealthBMI`에 입력 파일 인자 추가 시 TC-CLI-04 CLI 자동화; `Feature_test_plan.md` §4.2·§8 상태 열을 본 결과로 갱신.

---

*본 보고서는 [Feature_test_plan.md](./Feature_test_plan.md) 실행 직후(2026-05-20) 기준입니다.*
