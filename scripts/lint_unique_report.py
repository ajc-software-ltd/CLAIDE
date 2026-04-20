#!/usr/bin/env python3
import collections
import pathlib
import re
import sys


def bucket_for(check: str, severity: str) -> str:
    if severity == "error" and check in {"", "clang-diagnostic-error"}:
        return "A"
    if check.startswith("clang-analyzer-") or check.startswith("bugprone-"):
        return "B"
    if check.startswith("readability-"):
        return "D"
    return "C"


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: lint_unique_report.py <raw_input> <unique_output> <summary_output>", file=sys.stderr)
        return 2

    raw_path = pathlib.Path(sys.argv[1])
    unique_path = pathlib.Path(sys.argv[2])
    summary_path = pathlib.Path(sys.argv[3])

    diag_pattern = re.compile(
        r"^(?P<path>.+?):(?P<line>\d+):(?P<col>\d+):\s+"
        r"(?P<severity>warning|error):\s+"
        r"(?P<message>.+?)"
        r"(?:\s+\[(?P<check>[-\w.]+)\])?\s*$"
    )

    unique = {}
    by_bucket = collections.Counter()
    by_check = collections.Counter()
    by_file = collections.Counter()
    by_scope = collections.Counter()

    for raw_line in raw_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if not line:
            continue

        match = diag_pattern.match(line)
        if match:
            path = match.group("path")
            line_no = match.group("line")
            severity = match.group("severity")
            check = match.group("check") or ("clang-diagnostic-error" if severity == "error" else "unknown")
            key = f"{path}:{line_no}:{check}"
            if key in unique:
                continue
            message = match.group("message")
            bucket = bucket_for(check, severity)
            unique[key] = (bucket, severity, path, line_no, check, message)
            by_bucket[bucket] += 1
            by_check[check] += 1
            by_file[path] += 1
            if "/src/" in path:
                by_scope["src"] += 1
            elif "/tests/" in path:
                by_scope["tests"] += 1
            else:
                by_scope["other"] += 1
            continue

        if line.startswith("Error while processing "):
            path = line.removeprefix("Error while processing ").rstrip(".")
            check = "clang-diagnostic-error"
            key = f"{path}:0:{check}"
            if key in unique:
                continue
            bucket = "A"
            unique[key] = (bucket, "error", path, "0", check, "Error while processing translation unit")
            by_bucket[bucket] += 1
            by_check[check] += 1
            by_file[path] += 1
            if "/src/" in path:
                by_scope["src"] += 1
            elif "/tests/" in path:
                by_scope["tests"] += 1
            else:
                by_scope["other"] += 1

    sorted_unique = sorted(unique.values(), key=lambda item: (item[0], item[2], int(item[3]), item[4]))
    unique_lines = [
        f"[{bucket}] {severity} {path}:{line_no} [{check}] {message}"
        for bucket, severity, path, line_no, check, message in sorted_unique
    ]
    unique_path.parent.mkdir(parents=True, exist_ok=True)
    unique_path.write_text("\n".join(unique_lines) + ("\n" if unique_lines else ""), encoding="utf-8")

    total = len(sorted_unique)
    summary_lines = [
        "# clang-tidy unique diagnostics summary",
        "",
        f"- Total unique diagnostics: {total}",
        f"- Bucket A (errors/parser/tool failures): {by_bucket['A']}",
        f"- Bucket B (clang-analyzer + bugprone): {by_bucket['B']}",
        f"- Bucket C (other non-readability warnings): {by_bucket['C']}",
        f"- Bucket D (readability warnings): {by_bucket['D']}",
        "",
        "## Scope split",
        f"- src/: {by_scope['src']}",
        f"- tests/: {by_scope['tests']}",
        f"- other: {by_scope['other']}",
        "",
        "## Top checks",
    ]

    for check, count in by_check.most_common(20):
        summary_lines.append(f"- {check}: {count}")

    summary_lines.append("")
    summary_lines.append("## Top files")
    for file_path, count in by_file.most_common(20):
        summary_lines.append(f"- {file_path}: {count}")

    summary_path.parent.mkdir(parents=True, exist_ok=True)
    summary_path.write_text("\n".join(summary_lines) + "\n", encoding="utf-8")

    print(f"[lint-report] Wrote {unique_path}")
    print(f"[lint-report] Wrote {summary_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
