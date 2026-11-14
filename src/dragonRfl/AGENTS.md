# dragonRfl — Drakan's game logic (`dragon.rfl` re-implementation)

Static library linked into executables with `--whole-archive`. Headers: `include/dragonRfl/`. Namespace `dragonRfl`.
Depends only on odCore interfaces (no OSG).

## Files

| File | Purpose |
|---|---|
| `RflDragon.cpp` | `DragonRfl : odRfl::Rfl`. Hooks: `onLoaded` (registers classes), `onGameStartup` (creates the main menu / loads intro), `onLevelLoaded` (server side), `spawnHumanControlForPlayer`. |
| `ClassRegistry.cpp` | `DragonRfl::_registerClasses()`: the list of `registerClass<T>()` calls. **Add new classes here.** Commented-out lines = not yet ported. |
| `Actions.cpp` | Input action definitions (`DragonRfl` key bindings: move, jump, attack...). |
| `Damage.cpp` | Damage types/helpers. |
| `MessageChannel.h` | Message channel for inter-object triggering (Detector → Timer → ...). |
| `LocalPlayer.h` | Holder for the local player's state/controls. |
| `classes/*.cpp` | One level-object class each (Building, Detector, Timer, CountDownToZero, Fader, DynamicLight, StaticLight, Sky, StompPlayer, TrackingCamera, HumanControl(+Fields), UserInterfaceLauncher, ...). Each is a `FieldBundle`/class with `probeFields()` for its editor fields plus lifecycle hooks (`onSpawned`, `onUpdate`, ...). |
| `classes/common/` | Shared bases/fields (`PlayerCommon`, `ItemCommon`, `Enums.h`). |
| `gui/*.cpp` | Drakan GUI: `DragonGui`, `MainMenu`, `CrystalRingButton`, `Cursor`, `HealthIndicator`, `GuiTextures`. |

Currently **registered/built** classes: Building, CountDownToZero, Detector, DynamicLight, Fader, HumanControl, Sky,
StaticLight, StompPlayer, Timer, TrackingCamera, UserInterfaceLauncher (see `CMakeLists.txt` / `ClassRegistry.cpp`; the
rest are commented out pending port).

## Adding a class

1. Look up the original field layout in `doc/dragon_rfl_classtypes.txt`; the numeric class id is the registration key.
2. Create `include/dragonRfl/classes/Foo.h` + `src/dragonRfl/classes/Foo.cpp`, mirroring an existing simple class (e.g. `Timer`, `CountDownToZero`).
3. Declare fields in `probeFields(odRfl::FieldProbe&)` with the original *field names* (they are matched against the level data).
4. Add the `.cpp` to `src/dragonRfl/CMakeLists.txt`, and `registerClass<Foo>()` + include in `ClassRegistry.cpp`.
5. Server vs client behaviour: logic belongs on the server; visuals/audio on the client. Replicated data goes through states
   (`odState::StateBundle`), see odCore `state/`.
