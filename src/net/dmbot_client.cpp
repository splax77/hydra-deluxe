#include "net/dmbot_client.h"

#include <stdexcept>

#include "json.hpp"

#include "app/analysis.h"  // normalize_chart_hash
#include "core/error_kind.h"
#include "net/http.h"

namespace hydra::net {

namespace {

using json = nlohmann::json;

// --- defensive JSON field readers (the API sends nulls for missing values) ---

std::string jstr(const json& j, const char* key) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null()) return {};
    return it->is_string() ? it->get<std::string>() : it->dump();
}

int64_t jint(const json& j, const char* key, int64_t def = 0) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null() || !it->is_number()) return def;
    return it->get<int64_t>();
}

std::optional<int> joptint(const json& j, const char* key) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null() || !it->is_number()) return std::nullopt;
    return it->get<int>();
}

std::string join_charters(const json& entry) {
    auto it = entry.find("charter_refs");
    if (it == entry.end() || !it->is_array()) return {};
    std::string out;
    for (const json& c : *it) {
        if (!c.is_string()) continue;
        if (!out.empty()) out += ", ";
        out += c.get<std::string>();
    }
    return out;
}

DmScore parse_score(const json& entry, bool known) {
    DmScore s;
    s.identifier = app::normalize_chart_hash(jstr(entry, "identifier"));
    s.song_name = jstr(entry, "song_name");
    s.artist = jstr(entry, "artist");
    s.charter = join_charters(entry);
    s.score = jint(entry, "score");
    s.is_fc = jint(entry, "is_fc") != 0;
    s.percent = (int)jint(entry, "percent");
    s.speed = (int)jint(entry, "speed", kBaseSpeedPercent);
    s.rank = joptint(entry, "rank");
    s.posted = jstr(entry, "posted");
    s.known = known;
    return s;
}

json parse_json(const std::string& body) {
    try {
        return json::parse(body);
    } catch (const std::exception&) {
        throw KindedError(ErrorKind::NetBadReply,
                          "the leaderboard sent a response Hydra couldn't read");
    }
}

}  // namespace

std::vector<DmUser> parse_users_json(const std::string& body) {
    json root = parse_json(body);
    if (!root.is_array()) throw KindedError(ErrorKind::NetBadReply, "unexpected user-list format");

    std::vector<DmUser> users;
    users.reserve(root.size());
    for (const json& u : root) {
        DmUser du;
        du.id = jstr(u, "id");
        du.username = jstr(u, "username");
        du.elo = joptint(u, "elo");
        auto stats = u.find("stats");
        if (stats != u.end() && stats->is_object()) {
            du.total_scores = (int)jint(*stats, "total_scores");
            du.total_score = jint(*stats, "total_score");
        }
        if (!du.id.empty()) users.push_back(std::move(du));
    }
    return users;
}

std::vector<DmScore> parse_scores_json(const std::string& body) {
    json root = parse_json(body);

    std::vector<DmScore> scores;
    auto add_all = [&](const char* key, bool known) {
        auto arr = root.find(key);
        if (arr == root.end() || !arr->is_array()) return;
        scores.reserve(scores.size() + arr->size());
        for (const json& entry : *arr) {
            DmScore s = parse_score(entry, known);
            if (!s.identifier.empty()) scores.push_back(std::move(s));
        }
    };
    add_all("scores", /*known=*/true);
    add_all("unknown_scores", /*known=*/false);
    return scores;
}

namespace {
// The name the transport gives this server in its raw detail text.
constexpr const char* kHostName = "leaderboard";

std::string get(const std::string& url, const std::atomic<bool>* cancel) {
    return http_get(url, cancel, kHostName);
}
}  // namespace

std::vector<DmUser> fetch_users(const std::string& api_base, const std::atomic<bool>* cancel) {
    return parse_users_json(get(api_base + "/all-users", cancel));
}

std::vector<DmScore> fetch_scores(const std::string& discord_id, const std::string& api_base,
                                  const std::atomic<bool>* cancel) {
    return parse_scores_json(get(api_base + "/user/" + discord_id + "/scores", cancel));
}

}  // namespace hydra::net
