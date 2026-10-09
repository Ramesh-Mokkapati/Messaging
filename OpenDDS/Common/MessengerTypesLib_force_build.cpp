// This file ensures MessengerTypesLib.vcxproj is always scheduled for build,
// even before the IDL-generated source files (MessengerC.cpp, etc.) exist on disk.
// Visual Studio silently skips StaticLibrary projects that have no existing source
// files; this stub guarantees the project is never dropped from the build graph.
