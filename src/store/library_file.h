// Which generation a library file (hydra.db) is, and what an open does with a
// file too old to convert. These two answers live here once (user,
// 2026-10-14: "cut off everything prior to 2.2.0"); the store's open asks
// the functions here and never probes the file's tables itself.

#ifndef HYDRA_STORE_LIBRARY_FILE_H
#define HYDRA_STORE_LIBRARY_FILE_H

#include <string>

struct sqlite3;

namespace hydra::store {

// The oldest Hydra release whose library file this build still converts. A
// file from any earlier release is set aside or refused (OldFilePolicy).
inline constexpr const char* kOldestConvertibleVersion = "2.2.0";

// What a library file is, read from its tables alone.
enum class LibraryFileKind {
    Fresh,        // no file, an empty file, or SQLite's in-memory name
    Current,      // this build's layout (profiles, records, record_paths)
    // the layout before the scan stored each chart's album: its charts table
    // lacks the album column, which the open adds in place
    LacksAlbumColumn,
    // 2.8.0's layout before failures were stored: its records table lacks the
    // failure columns, which the open adds in place
    LacksFailureColumns,
    Convertible,  // a 2.2.0 to 2.7.0 file: its results table becomes records on open
    TooOld,       // a file from before kOldestConvertibleVersion
    Unreadable,   // the file exists but SQLite cannot read its tables
};

// The one owner of "which generation is this library file". It opens the file
// only to read its table list and columns, and closes it before returning.
// TooOld needs positive evidence of an older layout; a file with none of the
// layouts' tables reads Current, and the store's set-up makes what it lacks.
LibraryFileKind classify_library_file(const std::string& dbpath);

// Whether the open file's charts table exists without the album column, the
// one test classify_library_file names LacksAlbumColumn by. The store's set-up
// asks it on its own handle for every kind older than LacksAlbumColumn too,
// since those files may or may not hold the charts table already. When SQLite
// cannot read the table's columns it answers true, so the add runs and
// SQLite's own failure reaches the open.
bool charts_table_lacks_album(sqlite3* db);

// What an open does with a TooOld file. Every caller names one; there is no
// default, because the app and the command-line tools want different things.
enum class OldFilePolicy {
    SetAside,  // rename it (set_aside_old_library) and start a fresh library
    Refuse,    // throw KindedError(OldLibraryRefused) and leave the file alone
};

}  // namespace hydra::store

#endif  // HYDRA_STORE_LIBRARY_FILE_H
