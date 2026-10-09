# OpenDDS Pub/Sub (C++11 mapping) — Visual Studio 2022

A minimal OpenDDS publisher/consumer pair using the **C++11 IDL
language mapping** (`-Lc++11`), split into three Visual Studio 2022
projects tied together by one solution:

```
OpenDDS_PubSub_Cxx11.sln
├── MessengerTypesLib   (static library)   Common\MessengerTypesLib.vcxproj
├── OpenDDS_Publisher   (console app)      Publisher\OpenDDS_Publisher.vcxproj
└── OpenDDS_Subscriber  (console app)      Subscriber\OpenDDS_Subscriber.vcxproj
```

`MessengerTypesLib` runs `opendds_idl`/`tao_idl` on `Messenger.idl`
**once** and compiles the generated type support code into a static
library. Both `OpenDDS_Publisher` and `OpenDDS_Subscriber` reference
that project (via `<ProjectReference>`) instead of regenerating or
recompiling the same code — build order and linking against
`MessengerTypesLib.lib` are both handled automatically by the
solution.

A CMake build of the same layout is also included
(`CMakeLists.txt`) if you'd rather use VS2022's Open-Folder/CMake
workflow instead of the `.sln` — see the bottom of this file.

## Layout

```
OpenDDS_PubSub_Cxx11/
├── OpenDDS_PubSub_Cxx11.sln
├── OpenDDS.props            <- shared include/lib paths, C++ std, OpenDDS/ACE/TAO libs
├── rtps.ini                 <- DDS discovery/transport config (shared)
├── CMakeLists.txt           <- alternate CMake build of the same layout
├── Common/
│   ├── Messenger.idl
│   └── MessengerTypesLib.vcxproj
├── Publisher/
│   ├── Publisher.cpp
│   └── OpenDDS_Publisher.vcxproj
└── Subscriber/
    ├── Subscriber.cpp
    ├── DataReaderListenerImpl.h / .cpp
    └── OpenDDS_Subscriber.vcxproj
```

Files like `MessengerC.h`, `MessengerTypeSupportImpl.cpp`, etc. are
**generated** by OpenDDS's IDL compiler into `Common\` at build
time — you won't see them until you build once. Solution Explorer
will show them as missing before that; that's expected.

## 1. Prerequisites

- **Visual Studio 2022** with the *Desktop development with C++* workload
- **Git for Windows** (on `PATH`)
- **CMake** — included with VS 2022 via the *C++ CMake tools for Windows*
  individual component, or install separately and put it on `PATH`
- **Perl** — the build script auto-downloads portable Strawberry Perl
  (~250 MB) to `perl\` if neither Perl on `PATH` nor Strawberry at
  `C:\Strawberry` is found; install Strawberry Perl yourself to skip that
  download

`DDS_ROOT`, `ACE_ROOT`, and `TAO_ROOT` are set automatically by
`OpenDDS.props` — no environment variables needed. The first build
triggers `setup-opendds.bat`, which clones OpenDDS, downloads ACE/TAO,
and runs a full CMake build (~30-90 minutes). Subsequent builds skip this
step instantly.

## 2. Open and build

1. Open `OpenDDS_PubSub_Cxx11.sln` in Visual Studio 2022 (Community
   edition is fine — nothing here needs Professional/Enterprise).
2. Select **x64** and **Debug** (or **Release**) from the toolbar.
3. **Build → Build Solution** (Ctrl+Shift+B). This builds
   `MessengerTypesLib` first (generating and compiling the DDS type
   support code), then `OpenDDS_Publisher` and `OpenDDS_Subscriber`,
   which link against it automatically.

> **First build only:** `setup-opendds.bat` fires as a pre-build event
> and compiles OpenDDS from source. The build window will appear to hang
> for 30-90 minutes — this is expected.

Output binaries land in `bin\x64\Debug\` (or `Release\`) at the
solution root. A post-build step copies all required OpenDDS and ACE/TAO
runtime DLLs and `rtps.ini` there automatically.

## 3. Run it

Each project already has its debugger command arguments set to
`-DCPSConfigFile rtps.ini` and its working directory set to its
output folder, so F5/Ctrl+F5 work out of the box. To run both at
once from Visual Studio:

1. Right-click the **solution** → **Properties → Common Properties →
   Startup Project**.
2. Choose **Multiple startup projects**, set both `OpenDDS_Publisher`
   and `OpenDDS_Subscriber` to **Start**, with `OpenDDS_Subscriber`
   listed above `OpenDDS_Publisher` (VS starts them top-to-bottom).
3. Press **Ctrl+F5** (Start Without Debugging) — the publisher waits
   for a matching subscriber before sending, so a small startup-order
   gap is fine either way.

Or from two terminals in `bin\x64\Debug\`:
```bat
OpenDDSSubscriber.exe -DCPSConfigFile rtps.ini
OpenDDSPublisher.exe  -DCPSConfigFile rtps.ini
```

The subscriber prints each sample it receives and exits once the
publisher finishes and disconnects.

## Notes / things you'll likely want to change

- **Domain ID** is hardcoded to `42` in both `Publisher.cpp` and
  `Subscriber.cpp` — must match on both sides.
- **Topic name** (`"Movie Discussion List"`) and type both come from
  the shared `Messenger.idl`/`MessengerTypesLib`, so they're
  guaranteed to match.
- This uses **RTPS discovery** (`rtps.ini`) — no separate
  `DCPSInfoRepo` process needed. Swap `rtps.ini` for an InfoRepo
  config (and run `DCPSInfoRepo`) if your deployment needs that
  instead.
- QoS is `RELIABLE` on both the writer and reader; drop that for
  best-effort delivery.
- **C++11 mapping**: struct fields use getter/setter methods —
  `message.subject_id(99)` to set, `message.subject_id()` to get —
  rather than the classic mapping's `message.subject_id = 99` /
  `CORBA::String_var`. To switch back to the classic mapping, remove
  `-Lc++11` from the `Command` lines in `MessengerTypesLib.vcxproj`
  (and `OPENDDS_IDL_OPTIONS` in `CMakeLists.txt`), then update field
  access in `Publisher.cpp` / `DataReaderListenerImpl.cpp`.
- MSVC has no dedicated "C++11" `/std` switch — its earliest
  `/std:c++NN` option is `c++14` (a superset of C++11); `OpenDDS.props`
  sets `stdcpp17`. This compiler flag is independent of, and doesn't
  affect, the `-Lc++11` **IDL** mapping.
- `OpenDDS.props` links the **Release** OpenDDS and ACE/TAO libs for
  both Debug and Release configurations. The cmake install only produces
  Release libs (no `d`-suffix variants), and mixing debug/release CRTs
  with the same DLLs causes runtime errors. `UseDebugLibraries` is still
  set to `true` for Debug so you get debug info for your own code.

## Alternative: CMake build

The included `CMakeLists.txt` builds the same three-target layout
(`MessengerTypesLib` static lib + `OpenDDSPublisher`/`OpenDDSSubscriber`
executables) via VS2022's **File → Open → Folder** (Open-Folder/CMake
workflow), or by generating a solution explicitly:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="path\to\opendds;path\to\opendds-src\build\ace_tao"
```

This is an alternative to the `.sln` — use one or the other, not both.
