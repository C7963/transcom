#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace PSCAN
{
    class IPscanDataSource
    {
    public:
        virtual ~IPscanDataSource() = default;
        virtual uint32_t ReadData(uint32_t detectorBitmask, uint8_t* buffer, uint32_t pointCount) = 0;
    };

    class PciePscanDataSource : public IPscanDataSource
    {
    public:
        uint32_t ReadData(uint32_t detectorBitmask, uint8_t* buffer, uint32_t pointCount) override;
    };

    class SocketPscanDataSource : public IPscanDataSource
    {
    public:
        uint32_t ReadData(uint32_t detectorBitmask, uint8_t* buffer, uint32_t pointCount) override;
        void SetSocketEndpoint(const std::string& ip, uint16_t port);

    private:
        std::string ip_ = "127.0.0.1";
        uint16_t port_ = 8080;
    };
}
