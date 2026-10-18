# Cursor project agent config (shared)

This directory is the **in-repo** copy of rules, hooks, and skills that collaborators should use so everyone edits under the same constraints.

## Dual copy

Keep these in sync with the maintainer’s user-global copies when either side changes:

| Project path | Typical user-global mirror |
|--------------|----------------------------|
| `.cursor/rules/all-projects-hard.mdc` | `~/.cursor/rules/all-projects-questions.mdc` |
| `.cursor/rules/no-unilateral-behavior-change.mdc` | `~/.cursor/rules/no-unilateral-behavior-change.mdc` |
| `.cursor/hooks/block-master-develop-git.py` | `~/.cursor/hooks/block-master-develop-git.py` |
| `.cursor/hooks/inject-no-unilateral-behavior.py` | `~/.cursor/hooks/inject-no-unilateral-behavior.py` |
| `.cursor/skills/all-projects/SKILL.md` | `~/.cursor/skills/all-projects/SKILL.md` |
| `.cursor/hooks.json` | merge with `~/.cursor/hooks.json` (project paths use `.cursor/hooks/…`) |

Project-only (no user-global twin required): `esp32` skill/rules, `no-font-upscale`, `no-self-instance-ref`, `no-project-logic-in-universal`.

OpenSpec `/opsx-*` commands and `openspec-*` skills live in the user-global `~/.cursor/` (not duplicated here). Project specs live under `openspec/`.

## Hooks (project `.cursor/hooks.json`)

- `beforeShellExecution` — block git commit/push on `master` / `develop` / `main`
- `beforeSubmitPrompt` — inject no-unilateral-behavior policy
- `preToolUse` — prompt gate for behavior-changing edits

After clone, open the project in Cursor so project hooks/rules load. If hooks do not appear, check **Cursor Settings → Hooks** and reload the window.
