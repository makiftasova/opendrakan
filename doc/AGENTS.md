# doc/ — reverse-engineering notes

These are research documents, not generated docs. Consult them *before* changing parsers or protocol code, and update
them when you learn something new about the original engine. Contributors add findings by editing these in place
(recent commits are mostly small additions to `riot_database_format.txt`).

| File | Content |
|---|---|
| `riot_database_format.txt` | **Primary format spec** for SRSC containers (`.db`, `.lvl`, `.rrc`): record types (e.g. 0x40 texture, 0x100 class RFL, 0x401 music), model/texture/animation/sequence layouts. Matches `odCore/SrscRecordTypes.h` and `db/*Factory.cpp`. |
| `lvl_format.txt` | Level (`.lvl`) file layout: layers, object records, etc. |
| `savegame_format.txt` | Original savegame structure. |
| `dragon_rfl_classtypes.txt` | All class types / editor fields of Drakan's `dragon.rfl`, grouped by category. Reference for `src/dragonRfl/classes`. |
| `drakan_network_protocol.md` | Research on the *original* Drakan network protocol (UDP/TCP, GameSpy-style discovery). |
| `net_protocol.md` | Draft of OpenDrakan's *own* protocol (ticks, asset refs, snapshots). Matches `odCore/net` + `state`. Initially incompatible with Drakan. |
| `animation_behaviour.md` | Clean-room behaviour description of the original engine's skeletal animation (root translation flags, playback, transitions, attach). Written by a separate examiner session; implement from this text only. |
| `clean_room_process.md` | Clean-room process for inspecting the original program: roles, workflow, implementer rules. Read before using original binaries. |
| `clean_room_examiner.md` | Rules for the examiner only. Give this file, and nothing else, to the examiner session. |
| `clean_room_reviewer.md` | Rules for the reviewer only. Give this file, and nothing else, to the reviewer session. |
| `commit_messages.md` | Commit message and history rules. |
| `collision_behaviour.md` | Clean-room description of the original character collision: ground height, snapping, falling, recovery. Implement from this text only. |
| `attachment_behaviour.md` | Clean-room description of how items attach to character channels (weapons). Implement from this text only. |
| `animation_channels_behaviour.md` | Clean-room description of animation channels, propagation, cutscene actions and transitions. Implement from this text only. |
| `water_behaviour.md` | Clean-room description of water: surface types, swimming, breath, rendering, currents. Implement from this text only. |
| `lighting.md` | How Riot lighting works (baked layer light, blending, dynamic lights). Used by `Layer`, `Light`, physics light dispatch. |
| `music.md` | DirectMusic data in `Music.rrc` (segments 0x1.., DLS 0x100.., volumes 0x1001..). |
| `volume.md` | Notes on sound volume (millibel) scaling. |
| `ubuntu_build.md` | Build instructions for Ubuntu. |
| `od_ownership.gv` | Graphviz diagram of shared/weak/non-owning pointer relations between engine objects. Update when changing ownership. |
