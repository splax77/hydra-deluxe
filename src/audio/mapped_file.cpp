#include "audio/mapped_file.h"

#include <cstdint>
#include <optional>
#include <stdexcept>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "core/winstr.h"

namespace hydra::audio {

std::shared_ptr<const MappedFile> MappedFile::open(const std::string& utf8_path) {
    HANDLE file = CreateFileW(hydra::win32_path(utf8_path).c_str(), GENERIC_READ,
                              FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("cannot open file: " + utf8_path);

    const std::optional<uint64_t> size = hydra::open_handle_size_bytes(file);
    if (!size || *size > SIZE_MAX) {
        CloseHandle(file);
        throw std::runtime_error("cannot open file: " + utf8_path);
    }

    std::shared_ptr<MappedFile> out(new MappedFile());
    if (*size == 0) {
        // CreateFileMappingW refuses an empty file; an empty map needs no view.
        CloseHandle(file);
        return out;
    }

    HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    // The mapping keeps the file open; the view keeps the mapping alive.
    CloseHandle(file);
    if (mapping == nullptr) throw std::runtime_error("cannot open file: " + utf8_path);
    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(mapping);
    if (view == nullptr) throw std::runtime_error("cannot open file: " + utf8_path);

    out->data_ = static_cast<const uint8_t*>(view);
    out->size_ = static_cast<std::size_t>(*size);
    return out;
}

MappedFile::~MappedFile() {
    if (data_ != nullptr) UnmapViewOfFile(data_);
}

}  // namespace hydra::audio
