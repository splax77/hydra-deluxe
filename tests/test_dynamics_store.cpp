// Tests for dynamics storage: encode/decode round-trip and RecordStore
// put_dynamics/get_dynamics persistence.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <sqlite3.h>

#include <cstdio>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "midi_util.h"
#include "parse/song.h"
#include "store/record_store.h"
#include "temp_util.h"

using namespace hydra::app;
using namespace hydra::store;

namespace {

// Build a breakdown with non-zero counts in every row (both dynamics_enabled
// values are tested separately).
DynamicsBreakdown make_full_breakdown(bool dynamics_enabled) {
    DynamicsBreakdown b;
    b.dynamics_enabled = dynamics_enabled;
    int v = 1;
    for (size_t i = 0; i < static_cast<size_t>(DynamicsRow::Count); ++i) {
        b.rows[i].ghost  = v++;
        b.rows[i].accent = v++;
        b.rows[i].normal = v++;
    }
    return b;
}

using TempFile = testtemp::ScopedFile;

}  // namespace

// ---------------------------------------------------------------------------
// Test 1: encode then decode round-trips with dynamics_enabled true and false.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics encode/decode round-trip") {
    for (bool enabled : {true, false}) {
        CAPTURE(enabled);
        const DynamicsBreakdown original = make_full_breakdown(enabled);
        const std::vector<uint8_t> blob = encode_dynamics(original);
        const auto decoded = decode_dynamics(blob);
        REQUIRE(decoded.has_value());
        CHECK(decoded->dynamics_enabled == enabled);
        for (size_t i = 0; i < static_cast<size_t>(DynamicsRow::Count); ++i) {
            CAPTURE(i);
            CHECK(decoded->rows[i].ghost  == original.rows[i].ghost);
            CHECK(decoded->rows[i].accent == original.rows[i].accent);
            CHECK(decoded->rows[i].normal == original.rows[i].normal);
        }
    }
}

// ---------------------------------------------------------------------------
// Test 2: decode rejects empty blob and unknown version.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics decode rejects bad blobs") {
    SUBCASE("empty blob") {
        CHECK_FALSE(decode_dynamics({}).has_value());
    }
    SUBCASE("a blob from a newer layout") {
        // A valid-length blob stamped with a layout this build doesn't know.
        const DynamicsBreakdown b = make_full_breakdown(true);
        std::vector<uint8_t> blob = encode_dynamics(b);
        blob[0] = static_cast<uint8_t>(kDynamicsBlobStamp.written + 1);
        CHECK_FALSE(decode_dynamics(blob).has_value());
    }
    SUBCASE("truncated blob") {
        const DynamicsBreakdown b = make_full_breakdown(true);
        std::vector<uint8_t> blob = encode_dynamics(b);
        blob.resize(10);  // too short
        CHECK_FALSE(decode_dynamics(blob).has_value());
    }
}

// ---------------------------------------------------------------------------
// The stored layout, byte for byte: these 118 bytes were copied from one run
// of encode_dynamics on the build before the blob moved onto serialize's
// BinaryWriter. If they still match, no stored Dynamics row moved.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics blob bytes are unchanged by the shared codec") {
    const std::vector<uint8_t> pinned = {
        0x02, 0x01,                                            // stamp, enabled
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0A, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
        0x0D, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
        0x13, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
        0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x19, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF,                                // no late tag
        0x00, 0x00, 0x00, 0x00,                                // marks before it
    };
    const std::vector<uint8_t> blob = encode_dynamics(make_full_breakdown(true));
    CHECK(blob.size() == 118);
    CHECK(blob == pinned);
}

// ---------------------------------------------------------------------------
// Test 3: RecordStore put/get dynamics — missing key, put+get, replace, and
//         keys differing only in pro or difficulty are separate rows.
// ---------------------------------------------------------------------------

TEST_CASE("RecordStore dynamics put/get") {
    TempFile tmp("dynamics_store", ".db");
    const DynamicsBreakdown bd = make_full_breakdown(true);
    const std::vector<uint8_t> blob = encode_dynamics(bd);

    {
        RecordStore store(tmp.path);

        // Missing key returns nullopt.
        DynamicsKey key{"abc123", "Expert", false};
        CHECK_FALSE(store.get_dynamics(key).has_value());

        // Put then get returns the same bytes.
        store.put_dynamics(key, blob, kDynamicsCountStamp.written);
        auto got = store.get_dynamics(key);
        REQUIRE(got.has_value());
        CHECK(*got == blob);

        // Put again replaces (different blob).
        const DynamicsBreakdown bd2 = make_full_breakdown(false);
        const std::vector<uint8_t> blob2 = encode_dynamics(bd2);
        store.put_dynamics(key, blob2, kDynamicsCountStamp.written);
        got = store.get_dynamics(key);
        REQUIRE(got.has_value());
        CHECK(*got == blob2);

        // A key differing only in pro is a separate row.
        DynamicsKey key_pro{"abc123", "Expert", true};
        CHECK_FALSE(store.get_dynamics(key_pro).has_value());
        store.put_dynamics(key_pro, blob, kDynamicsCountStamp.written);
        CHECK(store.get_dynamics(key_pro).has_value());
        // The non-pro row is still the replaced blob2.
        CHECK(*store.get_dynamics(key) == blob2);

        // A key differing only in difficulty is a separate row.
        DynamicsKey key_hard{"abc123", "Hard", false};
        CHECK_FALSE(store.get_dynamics(key_hard).has_value());
        store.put_dynamics(key_hard, blob, kDynamicsCountStamp.written);
        CHECK(store.get_dynamics(key_hard).has_value());
    }

    // Test 4: Reopen the same db file and the rows are still there.
    {
        RecordStore store2(tmp.path);
        DynamicsKey key{"abc123", "Expert", false};
        auto got = store2.get_dynamics(key);
        REQUIRE(got.has_value());
        // Should be blob2 (the replaced value).
        const DynamicsBreakdown bd2 = make_full_breakdown(false);
        CHECK(*got == encode_dynamics(bd2));

        DynamicsKey key_pro{"abc123", "Expert", true};
        CHECK(store2.get_dynamics(key_pro).has_value());

        DynamicsKey key_hard{"abc123", "Hard", false};
        CHECK(store2.get_dynamics(key_hard).has_value());
    }
}

TEST_CASE("dynamics keys come from one place") {
    DynamicsKey k = dynamics_store_key("abc123", hydra::Difficulty::Medium, true);
    CHECK(k.md5 == "abc123");
    CHECK(k.difficulty == "Medium");
    CHECK(k.pro);

    // The in-memory count is filed under the same key, so two keys compare
    // field by field: the same chart, difficulty and view are one count, and
    // a change to any one of them is another.
    CHECK(k == dynamics_store_key("abc123", hydra::Difficulty::Medium, true));
    CHECK(k != dynamics_store_key("abc124", hydra::Difficulty::Medium, true));
    CHECK(k != dynamics_store_key("abc123", hydra::Difficulty::Hard, true));
    CHECK(k != dynamics_store_key("abc123", hydra::Difficulty::Medium, false));

    // The background count always parses with 2x kicks kept.
    CHECK(kDynamicsParseBass2x);
}

TEST_CASE("dynamics_entry_from_analysis counts only when the parse kept 2x kicks") {
    hydra::Song song = hydra::load_songbytes_mid(
        testmidi::smf(testmidi::concat({testmidi::track_name("PART DRUMS"), testmidi::set_tempo(),
                                        testmidi::note_on(96, 100), testmidi::end_of_track()})),
        true, true);

    CHECK_FALSE(dynamics_entry_from_analysis("nokicks", song, /*bass2x=*/false,
                                             hydra::Difficulty::Expert, true)
                    .has_value());

    const std::optional<DynamicsEntry> entry = dynamics_entry_from_analysis(
        "withkicks", song, /*bass2x=*/true, hydra::Difficulty::Expert, true);
    REQUIRE(entry.has_value());
    CHECK(entry->key.md5 == "withkicks");
    CHECK(entry->key.difficulty == "Expert");
    CHECK(entry->key.pro);
    CHECK(entry->count_version == kDynamicsCountStamp.written);
    auto bd = decode_dynamics(entry->blob);
    REQUIRE(bd.has_value());
    CHECK(bd->row(DynamicsRow::Kick).all() == 1);
}

TEST_CASE("RecordStore dynamics rows from before the stamp read as missing") {
    TempFile tmp("dynamics_store", ".db");
    {  // A file from before the stamp: the dynamics table has no count_version column.
        sqlite3* db = nullptr;
        REQUIRE(sqlite3_open(tmp.path.c_str(), &db) == SQLITE_OK);
        REQUIRE(sqlite3_exec(db,
                             "CREATE TABLE dynamics (md5 TEXT NOT NULL, difficulty TEXT NOT NULL,"
                             " pro INTEGER NOT NULL, blob BLOB NOT NULL,"
                             " PRIMARY KEY (md5, difficulty, pro));"
                             "INSERT INTO dynamics VALUES ('old', 'Expert', 0, x'01');",
                             nullptr, nullptr, nullptr) == SQLITE_OK);
        sqlite3_close(db);
    }
    RecordStore store(tmp.path);
    DynamicsKey key{"old", "Expert", false};
    CHECK_FALSE(kDynamicsCountStamp.is_current(0));   // the migration stamps it 0
    CHECK_FALSE(store.get_dynamics(key).has_value());  // count again

    const std::vector<uint8_t> blob = encode_dynamics(make_full_breakdown(true));
    store.put_dynamics(key, blob, kDynamicsCountStamp.written);
    auto got = store.get_dynamics(key);
    REQUIRE(got.has_value());
    CHECK(*got == blob);
}

TEST_CASE("RecordStore dynamics rows with another count stamp read as missing") {
    TempFile tmp("dynamics_store", ".db");
    RecordStore store(tmp.path);
    DynamicsKey key{"abc123", "Expert", false};
    const std::vector<uint8_t> blob = encode_dynamics(make_full_breakdown(true));

    // A stamp this build doesn't accept, as a build with another counting
    // would have written it.
    const int other = kDynamicsCountStamp.written + 1;
    REQUIRE_FALSE(kDynamicsCountStamp.is_current(other));

    store.put_dynamics(key, blob, other);
    CHECK_FALSE(store.get_dynamics(key).has_value());

    // The recount under this build's stamp replaces the row.
    store.put_dynamics(key, blob, kDynamicsCountStamp.written);
    CHECK(store.get_dynamics(key).has_value());
}
