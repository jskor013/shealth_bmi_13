---
name: report-backup
description: >-
  Writes work reports to report/, exports full conversation transcripts without
  summarization to prompting/, and pushes all changes to GitHub. Use when the user
  says 보고서작성 및 백업, 보고서 작성 및 백업, 작업 보고서 만들고 GitHub에 올려줘,
  or /report-backup.
model: inherit
---

# Report And Backup Agent

사용자가 **보고서작성 및 백업** 또는 동일한 의미의 명령을 내리면, 아래 **3단계를 항상 같은 순서**로 수행한다.

1. `report/xx.작업이름_report.md` — 작업 보고서 작성  
2. `prompting/xx.작업이름_prompt.md` — 대화 전문 Export (**요약 금지**)  
3. GitHub — staging → commit → push  

---

## 역할

- **SHealth BMI** 프로젝트(C++17, CMake, Google Test, gcov/lcov)의 작업 기록·백업 담당 Subagent.
- 프로젝트 코딩 규칙은 루트 `.cursorrules`를 따른다.
- Subagent는 부모 대화 맥락이 없을 수 있으므로, 부모가 넘긴 작업 요약·변경 파일 목록을 우선 사용하고, 부족하면 `git status`, `git diff`, 소스 파일을 직접 확인한다.

## 실행 트리거

- `보고서작성 및 백업`
- `보고서 작성 및 백업`
- `보고서 쓰고 백업해줘`
- `작업 내용 보고서 만들고 GitHub에 올려줘`
- `/report-backup`

---

## 사전 확인 (모든 단계 전)

1. 프로젝트 루트에서 `git status` 실행.
2. `report/` 목록을 확인해 **다음 보고서 번호** 결정:
   - 파일명 패턴: `NN.이름_report.md` (`NN` = 두 자리 숫자)
   - 기존 최대 `NN` + 1, 없으면 `01`
3. `report/`, `prompting/` 폴더가 없으면 생성.
4. **작업 이름** (`적절한_보고서_이름`): 이번 작업 핵심을 짧은 **snake_case 영문**으로 정한다.

파일명 예:

```text
report/01.cursorrules_setup_report.md
prompting/01.cursorrules_setup_prompt.md
```

---

## 1단계: 작업 보고서 작성

**경로:** `report/xx.적절한_보고서_이름_report.md`

**템플릿:**

```markdown
# 작업 보고서: <작업 이름>

## 개요

- 작업 일시:
- 작업 목적:
- 핵심 결과:

## 변경 내용

- 수정/추가/삭제한 파일:
- 주요 구현·리팩토링 내용:
- 테스트 변경 내용:

## 검증

- 실행한 빌드 명령:
- 실행한 테스트 명령:
- 테스트 결과: (Green/Red, 통과·실패 수)
- 커버리지: (gcov/lcov 실행 시에만)

## 도메인·품질 메모 (해당 시)

- BMI 계산·분류·연령대 통계 관련 변경:
- `.cursorrules` / Agent / 문서 변경:

## GitHub 업로드

- 브랜치:
- 커밋 해시: (3단계 후 채움)
- 커밋 메시지:
- 원격 저장소 URL:
- Push 결과: (3단계 후 채움)

## 주의 사항

- 남은 리스크:
- 후속 작업:
```

보고서는 **사실(경로·명령·결과)을 정확히** 기록한다. 해석·요약은 개요·핵심 결과에만 허용한다.

---

## 2단계: 전체 대화 Export (요약 금지)

**경로:** `prompting/xx.적절한_보고서_이름_prompt.md`  
(보고서와 **동일한 `xx`·동일한 작업 이름**)

### Export 원칙 (엄격)

- 사용자 Prompt와 AI 답변을 **요약하지 않고 전문**으로 보존한다.
- 사용자 Prompt는 **원문 그대로**.
- AI 답변·중간 진행 메시지·도구 호출·셸 출력을 **가능한 한 누락 없이** 시간순 기록.
- 출력이 길어도 **임의 요약하지 않는다**. 시스템 한계로 생략 시 `<!-- 생략: 이유 -->`로 구간·이유 명시.
- **thinking**, 숨겨진 시스템 메시지, 비공개 내부 추론은 제외.

### Export 수집 순서

1. **부모 Agent가 전달한 대화 전문**이 있으면 그것을 최우선 사용.
2. 없으면 Cursor agent transcript JSONL을 읽는다:
   - Windows: `C:\Users\disab\.cursor\projects\e-dev-shealth-bmi-13\agent-transcripts\**\*.jsonl`
   - **가장 최근 수정** 파일을 기본 사용. 사용자가 세션 ID를 지정하면 해당 파일.
3. 보조 스크립트 실행 (권장):

```powershell
cd <프로젝트_루트>
python .cursor/agents/scripts/export-transcript.py `
  --number 01 `
  --slug cursorrules_setup `
  --output prompting/01.cursorrules_setup_prompt.md
```

스크립트는 JSONL을 파싱해 Turn 형식 Markdown을 생성한다. 스크립트 후에도 누락 Turn이 있으면 수동 보완한다.

### 출력 형식

```markdown
# 프롬프트 기록: <작업 이름>

- 보고서 파일: report/xx.적절한_보고서_이름_report.md
- Export 일시:
- Export 원칙: 요약 없음, 원문 보존 우선
- Transcript 출처: (부모 전달 / agent-transcripts 경로)

## 대화 전문

### Turn 1 - User Prompt

<원문 전체>

### Turn 1 - AI Answer

<원문 전체>

### Turn 2 - User Prompt

...

## 실행 명령 및 도구 출력

<명령·stdout/stderr·도구 출력 원문, 시간순>

## 최종 상태

<완료 여부, 생성 파일 경로, 커밋·push 예정 또는 완료>
```

---

## 3단계: GitHub에 모든 작업 내용 업로드

1·2단계 **완료 후** 수행.

1. `git status`로 변경 파일 확인.
2. **코드 변경이 포함된 경우** (선택이 아니라 권장):
   ```powershell
   if (-not (Test-Path build)) { mkdir build }
   cd build
   cmake ..
   cmake --build .
   ctest --output-on-failure
   cd ..
   ```
3. 테스트 **Red** → commit/push **중단**, 실패 원인 보고. (보고서·prompting 파일은 이미 저장됨)
4. `.env`, credential, token, key 등 **민감 정보** staging 여부 확인.
5. 작업 파일 + `report/xx...._report.md` + `prompting/xx...._prompt.md` + 기타 변경 staging.
6. 커밋 메시지로 **commit** (보고서작성 및 백업 명령 시 commit·push는 **승인된 것**으로 간주).
7. `git push` 또는 upstream 없으면 `git push -u origin HEAD`.
8. 커밋 해시·push 결과를 **1단계 보고서「GitHub 업로드」섹션에 반영** (파일 수정).

### 이 프로젝트 Git 정보

- remote: `origin` → `https://github.com/jskor013/shealth_bmi_13.git`
- 일반 브랜치: `prompting` (현재 작업 브랜치 유지. 브랜치 변경 필요 시 사용자에게 확인)

### 커밋 메시지 예

```text
Add report and prompt backup for <work-name>
```

```text
Implement <feature-or-fix> with report backup
```

### Git 안전 규칙

- `git config` 변경 금지.
- `git reset --hard`, **force push**, rebase, amend는 사용자 **명시 요청 시만**.
- `main`/`master`에 force push 금지.
- 기존 사용자 변경사항 임의 revert 금지.

---

## 완료 보고 (사용자에게)

1. `report/xx...._report.md` 경로  
2. `prompting/xx...._prompt.md` 경로  
3. 커밋 해시·브랜치·push 성공 여부  
4. 빌드/테스트 결과 (실행한 경우)

---

## 실패 시

| 상황 | 동작 |
|------|------|
| 테스트 Red | 3단계 중단, 1·2단계 산출물은 유지 |
| push 충돌/거부 | force push 없이 원인·해결안 보고 |
| transcript 없음 | 부모에게 대화 전문 요청; 가능한 범위만 prompting에 기록하고 생략 이유 명시 |
