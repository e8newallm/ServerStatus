#include <vector>
#include <string>
#include <cstdint>

struct DriveUsage
{
    std::string name;
    std::string location;
    uint64_t size;
    uint64_t used;
};

class Status
{
    const uint64_t identifier = 0xFFBB774499DD2200;
    const uint64_t identifierRev = 0x0022DD994477BBFF;
    public:
        uint64_t MemTotal = 0; // kB
        uint64_t MemFree = 0; // kB
        uint64_t SwapTotal = 0; // kB
        uint64_t SwapFree = 0; // kB
        uint64_t Uptime = 0; // Seconds
        uint64_t NetworkIn = 0; // Bytes
        uint64_t NetworkOut = 0; // Bytes
        std::vector<uint64_t> CoreUsage; // Millipercent
        uint64_t CoreCount = 0;
        std::vector<DriveUsage> Drives;
        uint64_t DriveCount;

        std::vector<uint8_t> serialise();
        void deserialise(std::vector<uint8_t> data);
        void print();
};
