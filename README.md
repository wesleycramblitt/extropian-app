# extropian-app

**Minimal application framework.**

Provides a lightweight skeleton for Extropian desktop applications:
creating an SDL3 + OpenGL window, polling input events, and running
the main loop.

Depends on `extropian-core`. Does NOT depend on `extropian-render`.

## What It Provides

| Feature | Description |
|---------|-------------|
| `exd::app::Application` | Minimal lifecycle — inherit and override `on_startup` / `on_update` / `on_shutdown` |
| `exd::app::Window` | SDL3 + OpenGL window creation, event polling, buffer swap, input state |
| `exd::app::EventState` | Per-frame keyboard/mouse input snapshot |
| `exd::app::InputMode` | Re-export from core: `FPS` vs `UI` input modes |
| Platform services | Clipboard, file dialogs, notifications (thin SDL3 wrappers) |

## Building

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Requires: `extropian-core`, OpenGL, SDL3.

## License

Business Source License 1.1 — see [LICENSE](LICENSE).
Converts to Apache 2.0 on 2029-05-26.
