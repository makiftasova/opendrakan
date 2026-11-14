# odCore — core engine library

Static library (`add_library(odCore STATIC)`, C++17). Headers live in `include/odCore/` (same relative paths).
**New `.cpp` files must be added to `src/odCore/CMakeLists.txt`.** odCore must stay independent of OSG/OpenAL;
anything backend-specific goes behind an abstract interface here and is implemented in `src/odOsg/`.

## Top level (`od` namespace)

| File | Purpose |
|---|---|
| `Engine.*` | Variant wrapper: either a `Client&` or a `Server&`; exposes shared subsystems (DbManager, RflManager, PhysicsSystem, EventQueue/StateManager). |
| `Server.*` | Authoritative simulation: loads level, ticks objects/physics, builds state snapshots, handles clients (`addClient`, downlink connectors), processes uplink messages. |
| `Client.*` | Presentation side: receives snapshots/events, interpolates, drives renderer/GUI/input, translates server DB indices to local ones (`translateGlobalDatabaseIndex`). |
| `Level.*` | Loads a `.lvl`: layers, object records, spawn logic (`initialSpawn`, `spawnAllObjects`), per-frame `update`, destruction queue. |
| `LevelObject.*` | One in-world object: transform, states (`ObjectStates`), RFL class instance, physics/render handles, attachments, spawn/despawn. |
| `ObjectRecord.*` | Raw object record parsed from the level (`ObjectRecordData`). `ObjectStates.h`: replicated per-object state (position, rotation, scale, visibility, ...). |
| `Layer.*` | Terrain layer (heightmap/cell grid, holes, texture/lighting bake). |
| `Light.*`, `LightCallback.h`, `ObjectLightReceiver.*` | Light data and the receiver that gets notified by the physics system which lights affect an object. |
| `SrscFile.*`, `SrscRecordTypes.h` | SRSC container reader (record table, type ids). Format: `doc/riot_database_format.txt`. |
| `DataStream.*`, `RiffReader.*`, `ZStream.*` | Binary readers/writers (`DataReader`/`DataWriter`), RIFF chunks (DirectMusic), zlib streams. |
| `FilePath.*`, `ConfigFile.*` | Case-insensitive-aware path helper (`adjustCase()`, Windows-origin data), config file parsing. |
| `Message.*`, `Guid.*`, `IdTypes.h` | Net message base types, GUIDs, strong typedef IDs. |
| `BoundingBox/Sphere.*`, `Units.h`, `GlmSerializers.cpp`, `CTypes.h`, `Bitmask.h`, `Downcast.h`, `StringUtils.*` | Math/utility helpers. |
| `Logger.h`, `NuLogger.*`, `Panic.*`, `ThreadUtils.*`, `UpdateCallback.cpp` | Logging (`Logger::info()` streams), `OD_PANIC()` fatal errors, thread helpers. |

## Subdirectories

| Dir | Namespace | Contents |
|---|---|---|
| `db/` | `odDb` | `DbManager` (loads/caches `Database`s, weak ownership), `Database` (one `.db`/`.lvl`/`.rrc`), `DependencyTable` (resolves db index → Database), `Asset`/`AssetFactory`/`AssetRef`, concrete assets: `Model` (+`ModelBounds`, `ModelFactory`), `Texture`, `Animation`, `Sequence`, `SkeletonDefinition`, `Sound`, `Segment`/`MusicContainer`, `Class`/`ClassFactory`. Each asset type has a `*Factory` that parses its SRSC record. |
| `rfl/` | `odRfl` | RFL plugin API: `Rfl` (hooks `onLoaded`, `onGameStartup`, `onLevelLoaded`), `RflManager`, `Class` (`ClassBase`/`FieldBundle`), `Field*` types, `AssetRefField`, visitor *Probes* (`FieldProbe`, `FieldLoaderProbe`, `PrefetchProbe`, `ClassBuilderProbe`, `ObjectBuilderProbe`), `DefaultObjectClass`/`DummyClass` fallbacks. |
| `state/` | `odState` | Replication: `StateManager` (snapshots per tick, delta/acks), `Timeline<T>`, `Tick`, `Event`/`EventQueue` (timed events + asset prefetch), `StateBundle(Detail)` (typed extra-state containers for RFL classes). |
| `net/` | `odNet` | `UplinkConnector`/`DownlinkConnector` interfaces, `Queued*` in-process impls, `LocalTunnel` (simulated lag/drop), `PacketBuilder`/`PacketParser`, `Protocol.h` (packet types), `MessageDispatcher`, `Socket`/`IpAddress` (real network, mostly unimplemented). Draft spec: `doc/net_protocol.md`. |
| `physics/` | `odPhysics` | Abstract `PhysicsSystem` (ray/contact/sphere tests, handle creation), `Handles` (Object/Layer/Light), `ModelShape`, `CharacterController`. `bullet/` (`odBulletPhysics`) is the Bullet implementation (`BulletPhysicsSystem`, `*HandleImpl`, `ManagedCompoundShape`, debug drawer). |
| `render/` | `odRender` | Renderer interface: `Renderer`, `Handle`, `Group`, `Geometry`, `Model`, `Rig`, `Camera`, `Texture`, `Image`, `Array`, GUI callbacks. Implemented by odOsg. |
| `anim/` | `odAnim` | `Skeleton`, `SkeletonAnimationPlayer`, `SequencePlayer`, `BoneAccumulator`, `Interpolator`, `AnimModes`. |
| `audio/` | `odAudio` | `SoundSystem`/`Source`/`Buffer` interfaces, EAX presets; `music/`: DirectMusic `SegmentPlayer`, `MidiSynth` interface, `DMusicDefines.h`. |
| `gui/` | `odGui` | Widget tree (`Gui`, `Widget`, `Quad`, `WidgetIntersector`); measure → layout → render passes; cursor/menu mode. |
| `input/` | `odInput` | `InputManager` (thread-safe; mouse in GUI space), `Action`/`AnalogAction`, `InputEvent`, `Keys`, listeners. Game code binds to *actions*, not raw keys. |

## Data flow cheat sheet

1. `odOsg/Main.cpp` builds `DbManager`, `RflManager` (loads `DragonRfl`), `Server`, `Client`, connects them, calls `loadLevel`.
2. `Server::loadLevel` → `Level::loadLevel` → `Database`s loaded through `DbManager` → object records → `LevelObject`s → `db::Class` → RFL class instance via `RflManager` (field values filled with a `FieldLoaderProbe`).
3. Each server tick: update objects + physics → `StateManager` collects changed states/events → sent via downlink connector.
4. Client applies snapshots/events (`Client`, `EventQueue`), renders through `odRender` handles.
