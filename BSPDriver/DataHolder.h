#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>

// C++14兼容的延迟加载持有器
template<typename T>
class LazyDataHolder {
private:
    mutable std::mutex mutex_;
    mutable std::unique_ptr<std::vector<T>> data_ptr_;
    mutable std::atomic<bool> is_loaded_{ false };
    size_t expected_size_;
    std::function<bool(unsigned char*, unsigned int)> loader_;

public:
    LazyDataHolder(std::function<bool(unsigned char*, unsigned int)> loader, size_t expected_size)
        : loader_(loader), expected_size_(expected_size) {
    }

    LazyDataHolder(const LazyDataHolder&) = delete;
    LazyDataHolder& operator=(const LazyDataHolder&) = delete;

    const std::vector<T>& get() const {
        if (!is_loaded_.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!is_loaded_.load(std::memory_order_relaxed)) {
                auto temp_ptr = std::make_unique<std::vector<T>>();
                temp_ptr->resize(expected_size_);

                if (loader_(temp_ptr->data(), static_cast<unsigned int>(expected_size_))) {
                    data_ptr_ = std::move(temp_ptr);
                    is_loaded_.store(true, std::memory_order_release);
                }
            }
        }
        return *data_ptr_;
    }

    bool is_loaded() const { return is_loaded_.load(); }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        data_ptr_.reset();
        is_loaded_.store(false);
    }

    size_t size() const {
        return is_loaded_ ? data_ptr_->size() : 0;
    }
};

// 具体数据类型
class SpectrumData : public LazyDataHolder<unsigned char> {
public:
    SpectrumData(std::function<bool(unsigned char*, unsigned int)> loader)
        : LazyDataHolder(loader, 1024 * 4) {
    }
};

class IQData : public LazyDataHolder<unsigned char> {
public:
    IQData(std::function<bool(unsigned char*, unsigned int)> loader)
        : LazyDataHolder(loader, 2400 * 1024) {
    }
};

class PersistenceData : public LazyDataHolder<unsigned char> {
public:
    PersistenceData(std::function<bool(unsigned char*, unsigned int)> loader)
        : LazyDataHolder(loader, 1024 * 512 * 4) {
    }
};