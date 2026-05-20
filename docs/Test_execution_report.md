# SHealth BMI — 테스트 실행 결과 보고서

**문서 역할:** [테스트 계획 보고서](./Test_requirements_analysis_report.md) 기준 실행 결과  
**기준 문서:** [README.md](../README.md), [Test_requirements_analysis_report.md](./Test_requirements_analysis_report.md)  
**실행일:** 2026-05-20  
**실행 환경:** Windows 10 · CMake 4.3.1 · MinGW (ninja) · `E:/dev/shealth_bmi_13`

---

## 1. 실행 요약

| 항목 | 결과 |
|------|------|
| **빌드 (TC-NF-01)** | ✅ 성공 (`cmake --build .` — `ninja: no work to do`) |
| **ctest (TC-NF-02)** | ❌ **실패** — 0/1 통과 (1건 FAIL) |
| **FAIL() 스텁 (TC-NF-03)** | ❌ 미제거 — `SHealthBMITest.FailedTest` 의도적 실패 |
| **E2E 수동 (TC-G-03)** | ✅ `SHealthBMI.exe` exit 0, 골든 6연령대 출력 일치 |
| **계획 TC 자동화** | ⚠️ **30건 중 1건만 구현** (`FAIL()` 스텁), 나머지 미구현 |

### 종합 판정

| 구분 | 판정 |
|------|------|
| **자동화 테스트 (ctest)** | ❌ **미통과** — 수용 기준 AC-01 미충족 |
| **애플리케이션 동작 (수동 E2E)** | ✅ **정상** — `shealth.dat` 골든 24비율 일치 |
| **리팩토링 로직 (수동 spot-check)** | ✅ **정상** — TC-U-01, TC-U-02, TC-U-04 Python 대조 통과 |

---

## 2. 실행 환경·명령

```powershell
cd E:\dev\shealth_bmi_13\build
cmake ..
cmake --build .
ctest --output-on-failure -V
```

**메인 프로그램 (TC-G-03 보조):**

```powershell
cd E:\dev\shealth_bmi_13
.\build\SHealthBMI.exe
```

| 설정 | 값 |
|------|-----|
| ctest Working Directory | `E:/dev/shealth_bmi_13/build` (계획 권장: `${CMAKE_SOURCE_DIR}` 미적용) |
| Google Test | v1.14.0 (FetchContent, 기존 `_deps` 캐시 사용) |
| 테스트 실행 파일 | `build/SHealthBMITest.exe` |

---

## 3. ctest 상세 결과

| # | 테스트 이름 | 결과 | 소요 | 비고 |
|---|-------------|------|------|------|
| 1 | `SHealthBMITest.FailedTest` | ❌ FAIL | 0.04s | `SHealthBMITest.cpp:5` — `FAIL()` 매크로 |

**ctest 출력 요약:**

```
0% tests passed, 1 tests failed out of 1
FAILED: SHealthBMITest.FailedTest
```

**참고:** 동일 `build` 디렉터리의 `Testing/Temporary/LastTest.log`에는 **이전 실행**(2026-05-20 08:47) 기록으로 **11건 전부 PASS**가 남아 있으나, 현재 소스(`SHealthBMITest.cpp`)는 `FAIL()` 스텁 1건만 포함하며 **재빌드 후 ctest는 1건 실패**가 정확한 현재 상태이다.

---

## 4. 수동·보조 검증 결과

### 4.1 TC-G-01 / TC-G-03 — 골든 스냅샷 (`shealth.dat`)

**실행:** 프로젝트 루트에서 `SHealthBMI.exe`  
**결과:** exit code 0, 6연령대 × 4비율 출력이 계획서 §6.4 기대값과 **완전 일치**

| 연령대 | underweight | normal | overweight | obesity | 판정 |
|--------|-------------|--------|------------|---------|------|
| 20 | 3.511053 | 23.797139 | 11.833550 | 60.858257 | ✅ |
| 30 | 1.863354 | 15.527950 | 10.062112 | 72.546584 | ✅ |
| 40 | 0.521512 | 10.039113 | 9.126467 | 80.312907 | ✅ |
| 50 | 2.181401 | 12.629162 | 9.988519 | 75.200918 | ✅ |
| 60 | 0.862895 | 8.533078 | 10.642378 | 79.961649 | ✅ |
| 70 | 0.529101 | 12.345679 | 10.758377 | 76.366843 | ✅ |

### 4.2 TC-U-01, TC-U-02, TC-U-04 — 순수 로직 (수동 대조)

`BmiLogic.cpp`와 동일 규칙으로 Python 스크립트 대조 실행. **전 항목 PASS.**

| TC-ID | 검증 내용 | 결과 |
|-------|-----------|------|
| TC-U-01 | `computeBmi(70, 170)` = 24.221453 (±1e-4) | ✅ |
| TC-U-02 | 경계 7점 (18.5, 23, 25.0 **Obesity** 등) | ✅ |
| TC-U-04 | `inAgeDecade` 6경계 (19/20/29/30/79/80) | ✅ |

### 4.3 TC-G-02 — 로드 건수

| 항목 | 값 |
|------|-----|
| `shealth.dat` 총 라인 | 4,823 (헤더 1 + 데이터 4,822) |
| 자동 `calculateBmi` 반환값 검증 | ⏭️ 미실행 (Google Test 미구현) |

---

## 5. 테스트 계획 TC별 추적 매트릭스

**범례:** ✅ PASS · ❌ FAIL · ⏭️ NOT RUN (미구현) · 🔶 MANUAL (수동 검증)

### 5.1 Unit — `BmiLogic`

| TC-ID | 명세 요약 | 자동화 | 수동 | 종합 |
|-------|-----------|--------|------|------|
| TC-U-01 | 표준 BMI 계산 | ⏭️ | 🔶 ✅ | ✅ |
| TC-U-02 | 분류 경계 7점 (25.0 Obesity) | ⏭️ | 🔶 ✅ | ✅ |
| TC-U-03 | 극단 BMI (0, 50) | ⏭️ | ⏭️ | ⏭️ |
| TC-U-04 | `inAgeDecade` 경계 | ⏭️ | 🔶 ✅ | ✅ |
| TC-U-05 | enum 매핑 | ⏭️ | ⏭️ | ⏭️ |
| TC-U-06 | height=0 (정책 미정) | ⏭️ | ⏭️ | 보류 |

### 5.2 Integration — `SHealth`

| TC-ID | 명세 요약 | 자동화 | 종합 |
|-------|-----------|--------|------|
| TC-I-01 | 체중 0 연령대 평균 보정 | ⏭️ | ⏭️ |
| TC-I-02 | 전원 weight=0 div0 방어 | ⏭️ | ⏭️ |
| TC-I-03 | 단일 연령대 100% 한 카테고리 | ⏭️ | ⏭️ |
| TC-I-04 | 4비율 합 ≈ 100% | ⏭️ | ⏭️ |
| TC-I-05 | 19·80세 집계 제외 | ⏭️ | ⏭️ |
| TC-I-06 | 잘못된 API → 0 | ⏭️ | ⏭️ |
| TC-I-07 | Reader 미설정 | ⏭️ | ⏭️ |
| TC-I-08 | `distributionForAgeGroup` | ⏭️ | ⏭️ |

### 5.3 Component — `CsvHealthRecordReader`

| TC-ID | 명세 요약 | 자동화 | 종합 |
|-------|-----------|--------|------|
| TC-C-01 | 정상 CSV | ⏭️ | ⏭️ |
| TC-C-02 | 잘못된 행 스킵 | ⏭️ | ⏭️ |
| TC-C-03 | 파일 없음 | ⏭️ | ⏭️ |
| TC-C-04 | 헤더만 빈 파일 | ⏭️ | ⏭️ |
| TC-C-05 | 10,001건 상한 | ⏭️ | ⏭️ |

### 5.4 Golden / E2E

| TC-ID | 명세 요약 | 자동화 | 수동 | 종합 |
|-------|-----------|--------|------|------|
| TC-G-01 | 24비율 골든 스냅샷 | ⏭️ | 🔶 ✅ | ✅ |
| TC-G-02 | 로드 건수 | ⏭️ | ⏭️ | ⏭️ |
| TC-G-03 | main subprocess | ⏭️ | 🔶 ✅ | ✅ |

### 5.5 비기능

| TC-ID | 명세 | 결과 |
|-------|------|------|
| TC-NF-01 | `cmake --build .` 성공 | ✅ |
| TC-NF-02 | `ctest` 전체 PASS | ❌ |
| TC-NF-03 | `FAIL()` 스텁 제거 | ❌ |

---

## 6. 수용 기준 (Definition of Done) 충족 여부

| # | 기준 | 충족 | 비고 |
|---|------|------|------|
| AC-01 | `ctest` 전체 통과 | ❌ | 0/1 — `FailedTest` |
| AC-02 | TC 20건 이상 | ❌ | 구현 1건 (`FAIL()`만) |
| AC-03 | TC-U-02 BMI 25.0 → Obesity | 🔶 | 수동 대조 ✅, 자동화 ❌ |
| AC-04 | TC-G-01 24비율 골든 | 🔶 | E2E 수동 ✅, gtest ❌ |
| AC-05 | TC명·fixture로 GWT 추적 | ❌ | 스텁만 존재 |
| AC-06 | `WORKING_DIRECTORY` = 소스 루트 | ❌ | 현재 `build/` |

---

## 7. 발견 이슈·리스크

| ID | 심각도 | 내용 | 권장 조치 |
|----|--------|------|-----------|
| **DEF-01** | 높음 | `SHealthBMITest.cpp`가 `FAIL()` 스텁만 포함 — 회귀 무방비 (R-T01) | 계획서 §7~9대로 P0 TC부터 gtest 구현 |
| **DEF-02** | 높음 | ctest 실패로 CI/AC-01 불통과 | `FailedTest` 제거 후 TC 이관 |
| **DEF-03** | 중간 | CMake `WORKING_DIRECTORY` 미설정 (R-T04) | `gtest_discover_tests(... WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})` |
| **DEF-04** | 낮음 | `LastTest.log` 11 PASS와 현재 소스 불일치 | 로그는 과거 빌드 잔재 — 무시 또는 clean rebuild |
| **OBS-01** | 정보 | 리팩토링 코드·골든 출력은 정상 | UT 구현 시 회귀 기준으로 TC-G-01 우선 고정 |

---

## 8. 결론 및 권장 다음 단계

1. **애플리케이션 기능:** 리팩토링된 `SHealthBMI`는 `shealth.dat` 기준 **골든 출력이 기대값과 일치**하며, 핵심 BMI 경계(25.0 포함) 로직은 소스와 일치함을 수동 확인했다.
2. **자동화 테스트:** 테스트 계획 대비 **자동화 커버리지는 사실상 0%**에 가깝다. `ctest`는 의도적 `FAIL()` 1건으로 **실패**한다.
3. **우선 구현 순서 (계획서 §8):**
   - `TestFixtures.h` + `VectorHealthRecordReader`
   - P0: `TC-U-02`, `TC-G-01`, `TC-I-02`
   - CMake `WORKING_DIRECTORY` 설정
   - `FailedTest` 삭제 → `ctest` green

---

## 9. 부록 — 실행 로그 발췌

### 9.1 ctest (2026-05-20)

```
1/1 Test #1: SHealthBMITest.FailedTest ........***Failed
E:/dev/shealth_bmi_13/src/test/cpp/SHealthBMITest.cpp:5: Failure
Failed
0% tests passed, 1 tests failed out of 1
```

### 9.2 SHealthBMI.exe stdout

```
20 - underweight = 3.511053, normal = 23.797139, overweight = 11.833550, obesity = 60.858257
30 - underweight = 1.863354, normal = 15.527950, overweight = 10.062112, obesity = 72.546584
40 - underweight = 0.521512, normal = 10.039113, overweight = 9.126467, obesity = 80.312907
50 - underweight = 2.181401, normal = 12.629162, overweight = 9.988519, obesity = 75.200918
60 - underweight = 0.862895, normal = 8.533078, overweight = 10.642378, obesity = 79.961649
70 - underweight = 0.529101, normal = 12.345679, overweight = 10.758377, obesity = 76.366843
```

---

*본 보고서는 [Test_requirements_analysis_report.md](./Test_requirements_analysis_report.md)에 정의된 TC를 기준으로 2026-05-20에 실행·기록하였다.*
