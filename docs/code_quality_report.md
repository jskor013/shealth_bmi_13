# SHealth BMI — 코드 품질 분석 보고서

**분석 대상:** `src/main/cpp/SHealth.h`, `SHealth.cpp`, `SHealthBMI.cpp`, `src/test/cpp/SHealthBMITest.cpp`  
**관점:** SOLID, Clean Code, Code Smell, C++17 모던 스타일  
**작성일:** 2026-05-19

---

## 1. 코드 구조 요약

| 파일 | 역할 | 핵심 이슈 |
|------|------|-----------|
| `SHealth.h` | 데이터·통계 상태 + 공개 API | God Class, 24개 중복 멤버, 고정 배열 |
| `SHealth.cpp` | CSV 로드 → 보정 → BMI → 분포 계산 | God Method, 대규모 중복 if-else |
| `SHealthBMI.cpp` | `main`, 결과 출력 | 반복 `printf`, 매직 타입 코드 |
| `SHealthBMITest.cpp` | 단위 테스트 | `FAIL()` 고정, 실질 검증 없음 |

`calculateBmi()` 한 함수가 **파일 I/O, 파싱, 결측치 보정, BMI 산출, 연령대별 분포 집계**까지 모두 수행한다. 이는 SRP 위반의 전형적인 God Method 패턴이다.

---

## 2. 문제점 분석 표

| # | 문제점 | 위반 원칙 / 스멜 | 영향 | 개선 방향 | 우선순위 |
|---|--------|------------------|------|-----------|----------|
| 1 | `calculateBmi()`가 100줄 이상, 4가지 이상 책임 수행 | **SRP** 위반, **Long Method**, **God Method** | 변경 시 회귀 위험↑, 단위 테스트 불가, 가독성 저하 | `loadRecords`, `imputeMissingWeights`, `computeBmis`, `aggregateByAgeGroup` 등으로 분리 | **P0** |
| 2 | `ages[10000]` 등 고정 크기 C 배열 + `count` 무검증 증가 | **Code Smell: 고정 배열**, 방어 코드 부재 | 레코드 10,000건 초과 시 **버퍼 오버플로** (UB) | `std::vector<HealthRecord>` + capacity 검사, 또는 `reserve` + `emplace_back` | **P0** |
| 3 | `sum / ageCount`, `(double)underweight * 100 / sum`에서 0 나눗셈 가능 | **방어적 프로그래밍 부재** | 해당 연령대 데이터 없을 때 **크래시/NaN** | `ageCount == 0` / `sum == 0` 조기 반환, `std::optional<double>` | **P0** |
| 4 | BMI 경계값 불일치 (`> 25` vs 요구사항 `25 이상`, `18.5초과` vs `> 18.5`) | **스펙 불일치 버그** | BMI=23, 25 등 경계에서 **잘못된 분류·비율** | `enum class BmiCategory` + `constexpr` 임계값, 단일 `classifyBmi(double)` 함수 | **P0** |
| 5 | 연령대(20~70) × 카테고리(4)마다 동일 블록 if-else 6회 반복 (`76~106행`) | **DRY** 위반, **Shotgun Surgery**, **OCP** 위반 | 연령대 추가 시 6곳 이상 수정 | `std::array<std::array<double, 4>, 6>` 또는 `map<AgeGroup, BmiDistribution>` | **P1** |
| 6 | `getBmiRatio()` 24분기 if-else 체인 | **Switch 문 남용**, **Primitive Obsession**, **OCP** | 타입 코드 추가 시 분기 폭증 | `enum class AgeGroup`, `enum class BmiCategory` + 2D 배열/맵 조회 | **P1** |
| 7 | 멤버 변수 24개 (`underweight20` … `obesity70`) | **Data Clumps**, **Duplicate Observed Data** | 헤더 비대화, 초기화·직렬화 부담 | `BmiDistribution ratios[6]` 구조체 배열로 통합 | **P1** |
| 8 | 매직 넘버 다수: `18.5`, `23`, `25`, `100`, `10000`, `20~70`, `100/200/300/400` | **Magic Number** | 의미 불명확, 유지보수 시 오타 위험 | `namespace BmiThreshold { constexpr double kUnderweightMax = 18.5; ... }`, `enum class` | **P1** |
| 9 | 연령대 필터 `ages[i] >= a && ages[i] < a + 10` 3회 중복 | **Duplicate Code** | 로직 변경 시 누락 가능 | `bool inAgeGroup(int age, int decadeStart)` 헬퍼 추출 | **P2** |
| 10 | `std::stoi` / `std::stod` 예외·범위 검사 없음 | **에러 처리 부재** | 잘못된 CSV 한 줄로 **전체 프로세스 중단** | `try/catch` 또는 `std::from_chars`(C++17), 파싱 실패 레코드 스킵 | **P2** |
| 11 | `SHealth`가 파일 경로·CSV 형식에 강결합 | **DIP** 위반, **경계 분리 부재** | 데이터 소스 변경(DB, API) 시 클래스 전면 수정 | `IHealthRecordReader` 인터페이스 + `CsvHealthRecordReader` | **P2** |
| 12 | `getBmiRatio(int ageClass, int type)` — 의미 없는 정수 API | **Primitive Obsession**, **Intention Revealing 부족** | 호출부 오용 (`getBmiRatio(25, 999)`) | `getRatio(AgeGroup age, BmiCategory cat) const` | **P2** |
| 13 | `(double)underweight` C 스타일 캐스트 | **구식 C++ 관행** | 타입 안전성 저하 | `static_cast<double>(underweight)` 또는 정수 나눗셈 전 `double` 승격 | **P3** |
| 14 | `file.close()` 수동 호출 | **불필요 코드** (RAII) | 없음 (미미) | 제거 — `std::ifstream` 소멸자에 위임 | **P3** |
| 15 | `SHealthBMI.cpp`에서 동일 `printf` 패턴 6회 | **Duplicate Code** | 출력 형식 변경 시 6곳 수정 | `printAgeGroupReport(AgeGroup)` + range-for | **P3** |
| 16 | `printf` vs `std::cerr`/`iostream` 혼용 | **일관성 부족** | 스타일 불일치 | C++17: `std::cout` + `std::format`(C++20) 또는 iomanip | **P3** |
| 17 | 멤버 변수 기본 초기화·`const` 메서드 없음 | **const-correctness** 부재 | API 계약 불명확 | `getBmiRatio(...) const`, 계산 후 불변 스냅샷 | **P3** |
| 18 | `SHealthBMITest.cpp`가 `FAIL()`만 존재 | **테스트 부재** | 리팩토링 안전망 없음 | BMI·보정·분류·예외 TC 추가 (README 3단계) | **P1** |

---

## 3. SOLID 관점 상세

### SRP (단일 책임 원칙) — **심각 위반**

`SHealth` 클래스가 담당하는 책임:

1. CSV 파일 읽기 및 파싱  
2. 연령대별 결측 체중 보정  
3. BMI 계산  
4. 연령대별 BMI 분포 비율 집계  
5. 비율 조회 API  

→ 최소 **3~4개 클래스**로 분리 권장: `HealthRecord`, `BmiCalculator`, `AgeGroupStatistics`, `HealthDataRepository`(또는 Reader).

### OCP (개방-폐쇄 원칙) — **위반**

- 새 연령대(예: 80대) 추가: 멤버 변수 4개 + `calculateBmi` if-else + `getBmiRatio` 4분기 추가.  
- 새 BMI 등급 추가: 모든 분기 로직 수정.  
→ **데이터 구조(배열/맵) + enum**으로 변경 시 코드 수정 없이 확장 가능하도록 설계.

### LSP / ISP / DIP

- 인터페이스 부재로 ISP·DIP 논의는 제한적이나, 파일 I/O를 추상화하면 DIP 적용 여지가 있다.

---

## 4. Code Smell 목록

| 스멜 | 위치 | 설명 |
|------|------|------|
| God Class | `SHealth` | 상태·로직·I/O 모두 보유 |
| God Method | `calculateBmi()` | 다단계 파이프라인 단일 함수 |
| Long Method | `calculateBmi()`, `getBmiRatio()` | 100줄 / 26분기 |
| Magic Number | 전역 | 임계값·타입코드·배열크기 |
| Primitive Obsession | `ageClass`, `type` | enum 미사용 |
| Data Clumps | 24 ratio 멤버 | 함께 생성·사용되는 데이터 |
| Duplicate Code | 76~106, 111~135, main printf | 복사-붙여넣기 구조 |
| Shotgun Surgery | 연령대 변경 | 여러 파일·함수 동시 수정 |
| Feature Envy | 내부 루프 | 연령대 로직이 메서드 곳곳에 산재 |
| Dead Code | `file.close()` | RAII로 중복 |
| Speculative Generality | — | 해당 없음 (오히려 추상화 부족) |

---

## 5. C++17 스타일 개선 제안

### 5.1 도메인 모델

```cpp
enum class AgeGroup { Twenties = 20, Thirties = 30, /* ... */ };
enum class BmiCategory { Underweight, Normal, Overweight, Obesity };

struct HealthRecord {
    int id;
    int age;
    double weightKg;
    double heightCm;
};

struct BmiDistribution {
    double underweightPct{};
    double normalPct{};
    double overweightPct{};
    double obesityPct{};
};
```

### 5.2 상수화 (Magic Number 제거)

```cpp
namespace bmi {
    constexpr int kMaxRecords = 10'000;
    constexpr int kMinAgeDecade = 20;
    constexpr int kMaxAgeDecade = 70;
    constexpr int kAgeDecadeSpan = 10;
    constexpr double kCmPerMeter = 100.0;
    constexpr double kUnderweightMax = 18.5;
    constexpr double kNormalMax = 23.0;
    constexpr double kOverweightMax = 25.0;
}
```

### 5.3 함수 추출 예시

```cpp
// SHealth.cpp 내부 또는 별도 유틸
bool isInAgeDecade(int age, int decadeStart);
BmiCategory classifyBmi(double bmi);
double computeBmi(double weightKg, double heightCm);
void imputeMissingWeights(std::span<HealthRecord> records);
BmiDistribution computeDistribution(std::span<const HealthRecord> records, int decadeStart);
```

### 5.4 컨테이너·알고리즘

- `std::vector<HealthRecord> records_` — `reserve(10000)`  
- `std::array<BmiDistribution, 6> distributions_` — 연령대 인덱스 `(age - 20) / 10`  
- `std::optional<double>` — 조회 실패·0건 연령대 표현  

### 5.5 API 개선

```cpp
[[nodiscard]] std::size_t loadAndAnalyze(const std::string& filename);
[[nodiscard]] std::optional<double> getRatio(AgeGroup age, BmiCategory category) const;
```

---

## 6. 함수 추출·중복 제거 로드맵

```
calculateBmi()  ─┬─► loadRecordsFromCsv(path) -> vector<HealthRecord>
                 ├─► imputeMissingWeightsByAgeGroup(records)
                 ├─► computeBmis(records)
                 └─► aggregateDistributions(records) -> array<BmiDistribution,6>

getBmiRatio()   ───► distributions_[ageIndex][categoryIndex]  // if-else 24개 제거

main()          ───► for (auto decade : {20,30,...,70}) printReport(shealth, decade);
```

---

## 7. 리팩토링 우선순위 (실행 순서)

| 순위 | 항목 | 이유 | 예상 공수 |
|------|------|------|-----------|
| **1** | 버퍼·0나눗셈·BMI 경계 버그 수정 | 정확성·안정성 (기능 오류) | 소 |
| **2** | `calculateBmi` 책임 분리 (함수 추출) | SRP, 테스트 가능성 | 중 |
| **3** | 24 멤버 → `BmiDistribution` 배열 + `getBmiRatio` 단순화 | DRY, OCP | 중 |
| **4** | Magic Number → `constexpr` / `enum class` | 가독성, 타입 안전 | 소 |
| **5** | `std::vector` + `HealthRecord` 구조체 | 현대 C++, 확장성 | 중 |
| **6** | 연령대 필터·분류 헬퍼 추출 | 중복 제거 | 소 |
| **7** | 단위 테스트 작성 (BMI, 보정, 분류, 예외) | 리팩토링 안전망 | 중 |
| **8** | Reader 인터페이스 분리 (SRP/DIP) | 4단계 기능 확장 대비 | 대 |

---

## 8. 개선 방향 요약

### 즉시 (P0)

1. **정확성:** BMI 분류 경계를 README 스펙과 일치시키고, 0 나눗셈·배열 오버플로를 방어한다.  
2. **안전성:** 고정 배열 대신 `std::vector`와 bounds check를 도입한다.

### 1차 리팩토링 (P1, 클린코드)

3. **SRP:** `calculateBmi`를 “로드 → 보정 → 계산 → 집계” 4단계로 쪼개고, 클래스는 오케스트레이션만 담당하게 한다.  
4. **DRY/OCP:** 24개 멤버와 30개 이상의 if-else를 `std::array<BmiDistribution, N>` + enum 인덱스로 치환한다.  
5. **테스트:** `FAIL()` 테스트를 실제 TC로 교체해 리팩토링 회귀를 막는다.

### 2차 개선 (P2~P3, C++17)

6. **표현력:** `enum class`, `constexpr`, `optional`, `[[nodiscard]]`로 의도를 코드에 드러낸다.  
7. **일관성:** iostream 통일, C 스타일 캐스트 제거, `const` 메서드 적용.  
8. **확장:** README 4단계(키 0 보정, 정상 사용자 목록, 전체 비율)를 위해 Reader·Calculator·Statistics 서비스 분리.

### 기대 효과

| Before | After |
|--------|-------|
| God Method 1개 | 단일 책임 함수 5~8개 |
| if-else 50+ 분기 | 데이터 구조 조회 O(1) |
| 테스트 0건 실질 | 핵심 로직 TC 10건+ |
| 매직 넘버 15+ | 명명된 상수·enum |

---

## 9. 참고: 현재 BMI 분류 로직 vs 요구사항

**요구사항 (README):**

- ≤ 18.5: 저체중  
- 18.5 초과 ~ 23 미만: 정상  
- 23 이상 ~ 25 미만: 과체중  
- ≥ 25: 비만  

**현재 코드 (`SHealth.cpp` 65~72행):**

```cpp
if (bmis[i] <= 18.5)           // OK
else if (bmis[i] > 18.5 && bmis[i] < 23)   // OK
else if (bmis[i] >= 23 && bmis[i] < 25)    // OK
else if (bmis[i] > 25)         // BUG: BMI == 25.0 미분류
```

→ `else if (bmis[i] >= 25)` 또는 마지막 `else`로 비만 처리 필요.

---

*본 문서는 리팩토링 1~2단계(분석·클린코드) 입력 자료로 사용할 수 있습니다.*
