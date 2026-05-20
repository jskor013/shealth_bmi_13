#!/usr/bin/env python3
"""
Export Cursor agent-transcript JSONL to prompting markdown (no summarization).
Usage:
  python .cursor/agents/scripts/export-transcript.py --number 01 --slug work_name --output prompting/01.work_name_prompt.md
  python .cursor/agents/scripts/export-transcript.py --transcript path/to/file.jsonl --number 01 --slug work_name
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from datetime import datetime, timezone
from pathlib import Path


DEFAULT_TRANSCRIPT_ROOT = Path(
    r"C:\Users\disab\.cursor\projects\e-dev-shealth-bmi-13\agent-transcripts"
)


def find_latest_jsonl(root: Path) -> Path | None:
    files = list(root.glob("**/*.jsonl"))
    if not files:
        return None
    return max(files, key=lambda p: p.stat().st_mtime)


def strip_user_query_wrapper(text: str) -> str:
    m = re.search(r"<user_query>\s*(.*?)\s*</user_query>", text, re.DOTALL)
    return m.group(1).strip() if m else text


def content_to_text(content) -> str:
    if isinstance(content, str):
        return content
    if not isinstance(content, list):
        return str(content)
    parts: list[str] = []
    for item in content:
        if not isinstance(item, dict):
            parts.append(str(item))
            continue
        t = item.get("type")
        if t == "text":
            parts.append(item.get("text", ""))
        elif t == "tool_use":
            name = item.get("name", "unknown")
            inp = json.dumps(item.get("input", {}), ensure_ascii=False, indent=2)
            parts.append(f"\n[Tool: {name}]\n```json\n{inp}\n```\n")
        else:
            parts.append(json.dumps(item, ensure_ascii=False, indent=2))
    return "\n".join(parts)


def parse_jsonl(path: Path) -> list[dict]:
    turns: list[dict] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            turns.append(json.loads(line))
        except json.JSONDecodeError:
            turns.append({"role": "raw", "raw_line": line})
    return turns


def build_markdown(
    turns: list[dict],
    work_name: str,
    report_path: str,
    transcript_path: Path,
) -> str:
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    lines = [
        f"# 프롬프트 기록: {work_name}",
        "",
        f"- 보고서 파일: {report_path}",
        f"- Export 일시: {now}",
        "- Export 원칙: 요약 없음, 원문 보존 우선",
        f"- Transcript 출처: {transcript_path}",
        "",
        "## 대화 전문",
        "",
    ]

    turn_idx = 0
    last_role: str | None = None

    for entry in turns:
        role = entry.get("role", "unknown")
        if role == "raw":
            lines.append("### Raw line")
            lines.append("")
            lines.append(f"```\n{entry.get('raw_line', '')}\n```")
            lines.append("")
            continue

        msg = entry.get("message", {})
        content = msg.get("content") if isinstance(msg, dict) else msg
        text = content_to_text(content)
        if role == "user":
            text = strip_user_query_wrapper(text)
            turn_idx += 1
            lines.append(f"### Turn {turn_idx} - User Prompt")
        elif role == "assistant":
            label = "AI Answer" if last_role != "assistant" else "AI Answer (continued)"
            if last_role != "user":
                lines.append(f"### Turn {turn_idx} - {label}")
            else:
                lines.append(f"### Turn {turn_idx} - AI Answer")
        else:
            lines.append(f"### {role}")
        lines.append("")
        lines.append(text)
        lines.append("")
        last_role = role

    lines.extend(
        [
            "## 실행 명령 및 도구 출력",
            "",
            "(JSONL 내 Tool 블록 및 셸 출력은 위 Turn에 포함됨)",
            "",
            "## 최종 상태",
            "",
            "- Export: 스크립트 자동 생성 (수동 검토 권장)",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Export Cursor transcript to prompting markdown")
    parser.add_argument("--number", required=True, help="Report number, e.g. 01")
    parser.add_argument("--slug", required=True, help="Work name slug, e.g. cursorrules_setup")
    parser.add_argument("--output", help="Output .md path (default: prompting/NN.slug_prompt.md)")
    parser.add_argument("--transcript", help="Path to .jsonl (default: latest under agent-transcripts)")
    parser.add_argument("--transcript-root", default=str(DEFAULT_TRANSCRIPT_ROOT))
    args = parser.parse_args()

    num = args.number.zfill(2) if args.number.isdigit() else args.number
    slug = args.slug
    report_path = f"report/{num}.{slug}_report.md"
    output = Path(args.output or f"prompting/{num}.{slug}_prompt.md")

    if args.transcript:
        transcript_path = Path(args.transcript)
    else:
        root = Path(args.transcript_root)
        transcript_path = find_latest_jsonl(root)
        if transcript_path is None:
            print(f"No .jsonl found under {root}", file=sys.stderr)
            return 1

    if not transcript_path.is_file():
        print(f"Transcript not found: {transcript_path}", file=sys.stderr)
        return 1

    turns = parse_jsonl(transcript_path)
    md = build_markdown(turns, slug.replace("_", " "), report_path, transcript_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(md, encoding="utf-8")
    print(f"Wrote {output} ({len(turns)} JSONL records from {transcript_path})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
