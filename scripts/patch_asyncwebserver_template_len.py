from pathlib import Path
import re

Import("env")

TARGET_DEFINE = "TEMPLATE_PARAM_NAME_LENGTH"
TARGET_VALUE = "64"


def patch_asyncwebserver_template_limit() -> None:
    project_dir = Path(env.subst("$PROJECT_DIR"))
    libdeps_dir = Path(env.subst("$PROJECT_LIBDEPS_DIR"))
    pio_env = env.subst("$PIOENV")
    header_path = libdeps_dir / pio_env / "ESPAsyncWebServer" / "src" / "WebResponseImpl.h"

    if not header_path.exists():
        print(f"[patch_asyncwebserver_template_len] skip: {header_path} not found")
        return

    content = header_path.read_text(encoding="utf-8")
    pattern = re.compile(rf"^#define\s+{TARGET_DEFINE}\s+(\d+)\s*$", re.MULTILINE)
    match = pattern.search(content)

    if not match:
        raise RuntimeError(
            "[patch_asyncwebserver_template_len] "
            f"{TARGET_DEFINE} define not found in {header_path.relative_to(project_dir)}"
        )

    current_value = match.group(1)
    if current_value == TARGET_VALUE:
        print(
            "[patch_asyncwebserver_template_len] "
            f"{TARGET_DEFINE} already set to {TARGET_VALUE}"
        )
        return

    updated_content = pattern.sub(
        f"#define {TARGET_DEFINE} {TARGET_VALUE}",
        content,
        count=1,
    )
    header_path.write_text(updated_content, encoding="utf-8")
    print(
        "[patch_asyncwebserver_template_len] "
        f"updated {TARGET_DEFINE}: {current_value} -> {TARGET_VALUE}"
    )


patch_asyncwebserver_template_limit()
