// Library rows built by hand, for tests that need a library of charts without
// any chart files behind it.

#ifndef HYDRA_TESTS_LIBRARY_FIXTURES_H
#define HYDRA_TESTS_LIBRARY_FIXTURES_H

#include <cstdio>
#include <string>

#include "store/record_store.h"

// The one library row built from names alone. Its chart file does not exist,
// so a click on it lands on FileMissing and starts no job.
inline hydra::store::ChartLibraryEntry library_row(const std::string& md5,
                                                   const std::string& title,
                                                   const std::string& artist = "Artist",
                                                   const std::string& charter = "Charter") {
    hydra::store::ChartLibraryEntry e;
    e.md5 = md5;
    e.title = title;
    e.artist = artist;
    e.charter = charter;
    e.notespath = "C:\\charts\\" + md5 + "\\notes.chart";
    e.rootfolder = "C:\\charts";
    e.sig = "sig-" + md5;
    return e;
}

// Library row `i`: md5 "hash<i>" (three digits), title "Song hash<i>", so the
// rows sort by title in the order of `i`.
inline hydra::store::ChartLibraryEntry library_entry(int i) {
    char hash[32];
    std::snprintf(hash, sizeof(hash), "hash%03d", i);
    return library_row(hash, std::string("Song ") + hash);
}

#endif  // HYDRA_TESTS_LIBRARY_FIXTURES_H
