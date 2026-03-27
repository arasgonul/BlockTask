# BlockTask

Unreal **5.4**. Project file: `BlockTask/BlockTask/BlockTask.uproject` — open it in the editor and build when asked. If you only want to run it without compiling, use the packaged build from the link in the submission (this repo is source-only).

**Controls:** WASD, Space to jump, hold **left click** to mine, **right click** to place. Mouse wheel or **1 / 2 / 3** for the hotbar slots.

**Implemented features**

- Procedural world each run (Perlin noise terrain).
- Three block types by height: gray (deep, slow to mine), green (mid), white (high, fast to mine).
- Chunks load and unload as you move.
- Floor and ceiling — you can’t dig or build past the limits.
- Mining needs a hold; placing is one click and snaps to the grid.
- Three-slot inventory filled from broken blocks; hotbar shows what you’re holding.
