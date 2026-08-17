#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace CommBus { class XillyFile; }

namespace FFM {
class FfmDataSource {
public:
    FfmDataSource();
    ~FfmDataSource();
    bool Open();
    void Close();
    bool ReadFrame(std::vector<uint8_t>& bytes);
private:
    std::unique_ptr<CommBus::XillyFile> device_;
};
} // namespace FFM
