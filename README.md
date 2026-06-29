# extropian-app

**Application framework with RmlUi integration.**

Provides the skeleton for all Extropian desktop applications: mode management, system graph ordering, undo/redo, UI document loading, and hot-reload.

Depends on `extropian-core` and `extropian-render`. Does NOT depend on `extropian-physics`.

## What It Provides

| Feature | Description |
|---------|-------------|
| `ext::app::Application` | Lifecycle skeleton — inherit and override `on_*` hooks |
| `ext::app::ModeManager` | Mode state machine (Edit/Simulate/Postprocess or MainMenu/Play/PvP) |
| `ext::app::SystemGraph` | Register systems with mode affinity + ordering |
| `ext::app::CommandStack` | Generic undo/redo with `ICommand` |
| `ext::app::IUIHost` | RmlUi document loading, data binding, event wiring |
| Reusable components | PropertyEditor, TreeView, Viewport3D, Console, CurveEditor |

## RmlUi Integration

UIs are defined in RML (HTML-like markup) and RCSS (CSS-like stylesheets). C++ wires data bindings and event handlers. RmlUi provides hot-reload — edit `.rml`/`.rcss` files and see changes instantly without recompiling.

```cpp
auto& ui = app.ui();
ui.load_document("panels/solver_config.rml");
ui.bind("domain.nx", [&] { return domain_.nx; });
ui.on("onSolve", [&] { solver.run(); });
```

## Building

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Requires: `extropian-core`, `extropian-render`, RmlUi, Freetype.

## License

Business Source License 1.1 — see [LICENSE](LICENSE).
Converts to Apache 2.0 on 2029-05-26.
