// A read-only memory map of one file.
//
// A loose audio stem can be hundreds of megabytes. Mapping it lets a stem
// reader pull compressed bytes straight from the file on demand: Windows pages
// the parts that are touched into memory and can drop them again, so opening a
// long stem costs no big allocation and no up-front read.

#ifndef HYDRA_AUDIO_MAPPED_FILE_H
#define HYDRA_AUDIO_MAPPED_FILE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace hydra::audio {

class MappedFile {
public:
    // Maps the whole file read-only. Other programs may still read it while it
    // is mapped. A zero-length file maps as empty (data() is null, size() 0).
    // Throws std::runtime_error ("cannot open file: <path>") when the file
    // can't be opened or mapped.
    static std::shared_ptr<const MappedFile> open(const std::string& utf8_path);

    ~MappedFile();
    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    const uint8_t* data() const { return data_; }
    std::size_t size() const { return size_; }

private:
    MappedFile() = default;

    const uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
};

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_MAPPED_FILE_H
