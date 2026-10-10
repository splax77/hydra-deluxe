// Tests for ui/icons: the song-info icon PNGs. load_icons falls back to a
// plain marker when a PNG is missing or will not decode, so a renamed or
// corrupt file would ship as a blank icon with nothing failing. This test reads
// each file icon_files() names straight from the repo's resource/ folder and
// decodes it. No DX11 device is made; the GUI suite covers the upload.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "image/decode.h"
#include "source_tree.h"
#include "ui/icons.h"

using hydra::ui::IconFile;
using hydra::ui::icon_files;

TEST_CASE("icons: every icon PNG under resource/ decodes to 32 by 32 pixels") {
    REQUIRE_FALSE(icon_files().empty());
    for (const IconFile& icon : icon_files()) {
        CAPTURE(icon.file);
        const std::string path = (sourcetree::root() / "resource" / icon.file).u8string();
        std::vector<uint8_t> bytes;
        REQUIRE_NOTHROW(bytes = hydra::read_file_bytes(path));
        const hydra::image::DecodedImage img = hydra::image::decode_image(bytes);
        CHECK(img.width == 32);
        CHECK(img.height == 32);
    }
}
