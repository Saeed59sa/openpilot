#!/usr/bin/env python3
"""
fix_qt_connects.py
- Auto-fix Qt QObject::connect calls that pass only 3 args with a lambda as the 3rd.
- Makes them 4-arg connects by inserting a context object before the lambda.
- Safe to re-run (idempotent). Creates .bak backups.
- Defaults to scanning selfdrive/ui/qt and using 'this' as context.

Usage:
  python3 tools/fix_qt_connects.py --apply
  python3 tools/fix_qt_connects.py --dry-run
Options:
  --root <dir>       Root folder to scan (default: selfdrive/ui/qt)
  --context <name>   Context object to insert (default: this)
"""

import argparse, re, pathlib, sys, shutil

# Files to scan
DEFAULT_ROOT = "selfdrive/ui/qt"
FILE_EXTS = {".h", ".hh", ".hpp", ".cc", ".cpp", ".cxx"}

# Regex that matches:
# QObject::connect( <sender> , &Class::signal , [lambda...])
# connect( <sender> , &Class::signal , [lambda...])
# and variants with spaces/newlines. It ignores already-correct forms having ", this," (or any context) before '['.
CONNECT_LAMBDA_3ARGS = re.compile(
    r"""
    (?P<prefix>\b(?:QObject::)?connect\s*\(\s*         # connect(
        [^,()]+?                                       #   sender
        \s*,\s*
        (?:&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*\s*::\s*[A-Za-z_]\w*  #   &Class::signal
         |QOverload<[^>]+>::of\(\s*&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*\s*::\s*[A-Za-z_]\w*\s*\)
        )
        \s*,\s*
    )
    (?![A-Za-z_]\w*\s*,)                                #   NOT already having a context like 'this,' or 'panel,' etc.
    (?P<lambda_intro>\[[^\]]*\])                        #   start of lambda capture [=], [&], [this], etc.
    (?P<rest>\s*\()                                     #   the '(' starting the lambda params
    """,
    re.VERBOSE | re.MULTILINE | re.DOTALL,
)

# A lighter regex to avoid touching already-correct calls:
# ... , <context> , [ ...  -> we skip those
ALREADY_4ARGS_NEARBY = re.compile(r",\s*[A-Za-z_]\w*\s*,\s*\[")

def patch_text(text: str, context_name: str):
    # Quick-out if no 'connect(' present
    if "connect(" not in text and "QObject::connect(" not in text:
        return text, 0

    # Replace only the 3-arg lambda forms, avoid altering 4-arg connects
    def _repl(m):
        prefix = m.group("prefix")
        # If the preceding characters already show ", something , [", don't replace
        # (defensive; though the main regex tries to avoid this).
        start = m.start()
        probe_start = max(0, start - 80)
        if ALREADY_4ARGS_NEARBY.search(text[probe_start:start]):
            return m.group(0)
        return f"{prefix}{context_name}, {m.group('lambda_intro')}{m.group('rest')}"

    new_text, count = CONNECT_LAMBDA_3ARGS.subn(_repl, text)
    return new_text, count

def should_edit(path: pathlib.Path):
    return path.suffix in FILE_EXTS

def main():
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("--root", default=DEFAULT_ROOT)
    ap.add_argument("--context", default="this")
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--apply", action="store_true", help="write changes to files")
    mode.add_argument("--dry-run", action="store_true", help="print changes summary only")
    args = ap.parse_args()

    root = pathlib.Path(args.root)
    if not root.exists():
        print(f"[error] root not found: {root}", file=sys.stderr)
        sys.exit(2)

    total_fixed = 0
    files_fixed = 0

    for p in root.rglob("*"):
        if not p.is_file() or not should_edit(p):
            continue
        try:
            src = p.read_text(encoding="utf-8", errors="ignore")
        except Exception as e:
            print(f"[skip] {p}: {e}")
            continue

        new_src, n = patch_text(src, args.context)
        if n > 0:
            files_fixed += 1
            total_fixed += n
            if args.apply:
                # backup once
                bak = p.with_suffix(p.suffix + ".bak")
                if not bak.exists():
                    try:
                        shutil.copyfile(p, bak)
                    except Exception as e:
                        print(f"[warn] backup failed for {p}: {e}")
                try:
                    p.write_text(new_src, encoding="utf-8")
                except Exception as e:
                    print(f"[error] write failed for {p}: {e}")
                    continue
                print(f"[fixed] {p} (+{n} connect)")
            else:
                print(f"[would-fix] {p} (+{n} connect)")

    print(f"\nSummary: files={'applied' if args.apply else 'detected'}={files_fixed}, connects_fixed={total_fixed}, context='{args.context}'")

if __name__ == "__main__":
    main()
