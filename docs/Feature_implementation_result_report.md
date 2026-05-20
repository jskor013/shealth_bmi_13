# SHealth BMI — 신규 기능 구현 결과 보고서

**문서 역할:** [Feature_requirements_report.md](./Feature_requirements_report.md) v1.1 기준 구현 산출물 정리  
**기준 문서:** [README.md](../README.md) §4, [Refactoring_result_report.md](./Refactoring_result_report.md)  
**구현일:** 2026-05-20  
**상태:** FR-001~FR-005 구현 완료 · `ctest` 7/7 통과

---

## 1. 요약

README 4단계(기능 개선) 5개 항목과 고객 확인 사항(Q-01~Q-10)을 반영하여 신규 기능을 구현했다. 핵심 변경은 **키(height) 0 보정**, **CSV id 저장·정상 BMI 사용자 목록**, **전체 인구 BMI 비율**, **Imputation/Statistics 책임 분리**, **CLI 확장**이다.

| FR | 요구 요약 | 구현 상태 |
|----|-----------|-----------|
| FR-001 | SRP — `Imputation` / `Statistics` Extract | ✅ |
| FR-002 | 연령대별 분포 · 무인원 0% · CLI | ✅ |
| FR-003 | Height 0 연령대 평균 보정 · 예외 | ✅ |
| FR-004 | 정상 BMI 사용자 id 목록 (CSV 순) | ✅ |
| FR-005 | 전체 레코드 4비율 (19·80세 포함) | ✅ |

**회귀:** `shealth.dat` 골든 24비율(6연령대 × 4범주) **변경 없음** — `SHealthGoldenTest.ShealthDatSnapshot` 통과.

---

## 2. 파이프라인 (확정안)

`loadAndAnalyze()` 내부 오케스트레이션 (Q-02, Q-10 옵션 A):

```text
loadRecords
  → imputation::imputeMissingWeights
  → imputation::imputeMissingHeights
  → computeBmis
  → statistics::aggregateByAgeGroup
```

`public void imputeMissingHeights()`는 `records_` 로드 후 **단독 호출**도 가능하며, 내부적으로 동일 `imputation::imputeMissingHeights`를 호출한다.

---

## 3. 모듈별 구현 위치

### 3.1 FR-001 — 책임 분리 (Extract Class)

| 모듈 | 파일 | 책임 |
|------|------|------|
| **Imputation** | `src/main/cpp/Imputation.h`, `Imputation.cpp` | `imputeMissingWeights`, `imputeMissingHeights` |
| **Statistics** | `src/main/cpp/Statistics.h`, `Statistics.cpp` | `aggregateByAgeGroup`, `overallPopulationRatios` |
| **SHealth** | `src/main/cpp/SHealth.h`, `SHealth.cpp` | Reader 주입, 파이프라인, 공개 API 위임 |
| **BmiLogic** | `src/main/cpp/BmiLogic.*` | `computeBmi`, `classifyBmi` 등 순수 함수 (기존 유지) |

### 3.2 FR-003 — Height 0 보정

**파일:** `Imputation.cpp` — `imputeMissingHeights`

| 규칙 | 구현 |
|------|------|
| 연령대 판별 | `bmi::inAgeDecade(age, decade)` (20~70, step 10) |
| 평균 산출 | 동 decade 내 `height != 0` 레코드만 |
| 대체 | `height == 0` 레코드에 평균 할당 |
| 유효 키 0건 + 보정 필요 | `std::runtime_error("No valid height in age group {decade}")` (Q-01) |
| 순서 | 체중 보정 후 키 보정 (Q-02) |

### 3.3 FR-004 — 정상 BMI 사용자 ID

| 항목 | 구현 |
|------|------|
| `HealthRecord.id` | `SHealthTypes.h` 필드 추가 |
| CSV 파싱 | `CsvHealthRecordReader.cpp` — `tokens[0]` → `record.id` |
| API | `SHealth::filterNormalUserIds()` — `classifyBmi == Normal`, **CSV 입력 순**, 정렬 없음 (Q-08) |
| 정상 범위 | `(18.5, 23)` — 기존 `classifyBmi` / `BmiCategory::Normal` (Q-04) |

### 3.4 FR-005 — 전체 인구 비율

**파일:** `Statistics.cpp` — `overallPopulationRatios`

- 분모: `records_` **전체** (연령대 필터 없음, 19·80세 포함 — Q-09)
- 분자: `classifyBmi(record.bmi)` 기준 카운트
- 레코드 0건: `std::nullopt`
- `SHealth::overallPopulationRatios()`는 위 함수에 위임

### 3.5 FR-002 — 연령대별 분포 정합

- 무인원 연령대: `distributions_` 기본값 `0,0,0,0` 유지 → `distributionForAgeGroup` 항상 값 반환 (Q-06)
- 인원 ≥1: 4비율 합 100% (기존 집계 로직, `Statistics::aggregateByAgeGroup`)

---

## 4. 공개 API 변경 요약

| API | 변경 |
|-----|------|
| `loadAndAnalyze()` | 파이프라인에 `imputeMissingHeights` 단계 추가 |
| `imputeMissingHeights()` | stub → `Imputation` 위임 구현 |
| `filterNormalUserIds()` | stub → id 목록 반환 |
| `overallPopulationRatios()` | stub → 전체 4비율 반환 |
| `distributionForAgeGroup()` | 동작 유지 (무인원 0%) |
| `getRatio` / `getBmiRatio` / `calculateBmi` | 하위 호환 유지 |

---

## 5. CLI 출력 (`SHealthBMI.cpp`)

| 섹션 | 내용 |
|------|------|
| 기존 | 20~70대 6줄 — `distributionForAgeGroup` 기반 (형식 유지) |
| 신규 | `overall - underweight = …` 전체 인구 4비율 (FR-005, Q-07) |
| 신규 | `normal users count = N` + id 최대 20건 샘플 (FR-004, Q-07) |
| 오류 | Height 보정 예외 시 `stderr`에 메시지 출력 후 exit 1 |

**실측 예 (`shealth.dat`):**

```text
20 - underweight = 3.511053, normal = 23.797139, ...
...
overall - underweight = 1.596848, normal = 13.562837, overweight = 10.389880, obesity = 74.450436
normal users count = 654
  id = 93711
  ...
```

---

## 6. 데이터 모델

```cpp
struct HealthRecord {
    int id = 0;           // NEW — CSV column 0
    int age = 0;
    double weight = 0.0;
    double height = 0.0;
    double bmi = 0.0;
};
```

---

## 7. 빌드·테스트

### 7.1 CMake

`CMakeLists.txt`에 `Imputation.cpp`, `Statistics.cpp` 추가.  
`gtest_discover_tests`에 `WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}` 설정 (`shealth.dat` 골든 접근).

### 7.2 테스트 (`src/test/cpp/`)

| 파일 | 역할 |
|------|------|
| `SHealthBMITest.cpp` | 단위·통합·골든 7건 (`FAIL()` 스텁 제거) |
| `TestFixtures.h` | `VectorHealthRecordReader`, `makeRecord(id, …)` |

| TC | 검증 |
|----|------|
| `BmiLogicTest.ClassifyBoundaryValues` | 18.5 / 23 / 25 경계 |
| `ImputationTest.ImputeMissingHeightsReplacesZero` | FR-003 보정 |
| `ImputationTest.ImputeMissingHeightsThrowsWhenNoValidHeight` | Q-01 예외 |
| `SHealthTest.EmptyAgeGroupReturnsZeroRatios` | Q-06 무인원 0% |
| `SHealthTest.FilterNormalUserIdsPreservesCsvOrder` | FR-004, Q-08 |
| `SHealthTest.OverallPopulationIncludesAllAges` | FR-005 (19세 포함) |
| `SHealthGoldenTest.ShealthDatSnapshot` | AC-06 골든 24비율 |

### 7.3 실행 결과

```text
cmake --build build
ctest --test-dir build
→ 100% tests passed, 7 tests

./build/SHealthBMI.exe  (프로젝트 루트)
→ exit 0, 골든 6연령대 출력 기존과 동일
```

---

## 8. 요구사항 추적 (AC)

| AC | 내용 | 결과 |
|----|------|------|
| AC-01 | Height 0 → 연령대 평균 대체 | ✅ `ImputationTest` |
| AC-02 | 보정 후 inf/nan 없음 | ✅ (height≠0 전제) |
| AC-02b | 유효 키 0건 → 예외·메시지 | ✅ |
| AC-03 | 정상 BMI id · BR-04 | ✅ |
| AC-04 | 전체 4비율 합 ≈ 100% | ✅ |
| AC-05 | 6연령대 분포 합 ≈ 100% | ✅ 골든 |
| AC-05b | 무인원 0% | ✅ |
| AC-06 | 골든 24비율 불변 | ✅ |
| AC-07 | `ctest` 통과 | ✅ 7/7 |
| AC-09 | CLI 6연령대 + 전체 | ✅ |

---

## 9. Q-01 vs Q-06 (구현 확인)

| 상황 | 동작 |
|------|------|
| 연령대 **인원 0명** (집계·조회) | `distributionForAgeGroup` → **0,0,0,0**, 예외 없음 |
| decade에 레코드 있으나 height=0 보정 필요 + 유효 키 0건 | **`std::runtime_error`** |

---

## 10. 범위 외 (미구현·유지)

- 웹/UI, DB 연동
- 19·80세용 **연령대 집계 슬롯** 추가 (FR-005만 전체 레코드에 포함)
- P3 스타일 리팩토링 (C 캐스트, iostream 통일)
- 테스트 계획서 전체 30건 TC — 핵심 7건만 구현 (추가 TC는 QA 스프린트)

---

## 11. 파일 변경 목록

| 구분 | 경로 |
|------|------|
| 신규 | `src/main/cpp/Imputation.h`, `Imputation.cpp` |
| 신규 | `src/main/cpp/Statistics.h`, `Statistics.cpp` |
| 신규 | `src/test/cpp/TestFixtures.h` |
| 수정 | `src/main/cpp/SHealth.h`, `SHealth.cpp` |
| 수정 | `src/main/cpp/SHealthTypes.h` |
| 수정 | `src/main/cpp/CsvHealthRecordReader.cpp` |
| 수정 | `src/main/cpp/SHealthBMI.cpp` |
| 수정 | `src/test/cpp/SHealthBMITest.cpp` |
| 수정 | `CMakeLists.txt` |
| 신규 문서 | `docs/Feature_implementation_result_report.md` (본 문서) |

---

*본 문서는 [Feature_requirements_report.md](./Feature_requirements_report.md) v1.1 구현 완료 시점 기준입니다.*
