// Library rows built by hand, for tests that need a library of charts without
// any chart files behind it.

#ifndef HYDRA_TESTS_LIBRARY_FIXTURES_H
#define HYDRA_TESTS_LIBRARY_FIXTURES_H

#include <cstdio>
#include <string>

#include "store/record_store.h"

// Library row `i`: md5 "hash<i>" (three digits), title "Song hash<i>", so the
// rows sort by title in the order of `i`. Its chart file does not exist, so a
// click on it lands on FileMissing and starts no job.
inline hydra::store::ChartLibraryEntry library_entry(int i) {
    char hash[32];
    std::snprintf(hash, sizeof(hash), "hash%03d", i);
    hydra::store::ChartLibraryEntry e;
    e.md5 = hash;
    e.title = std::string("Song ") + hash;
    e.artist = "Artist";
    e.charter = "Charter";
    e.notespath = std::string("C:\\charts\\") + hash + "\\notes.chart";
    e.rootfolder = "C:\\charts";
    e.sig = "sig";
    return e;
}

#endif  // HYDRA_TESTS_LIBRARY_FIXTURES_H
