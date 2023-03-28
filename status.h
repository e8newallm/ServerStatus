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
    public:
        uint64_t MemTotal = 0; // kB
        uint64_t MemFree = 0; // kB
        uint64_t SwapTotal = 0; // kB
        uint64_t SwapFree = 0; // kB
        uint64_t Uptime = 0; // Seconds
        uint64_t NetworkIn = 0; // Bytes
        uint64_t NetworkOut = 0; // Bytes
        uint64_t CoreCount = 0;
        std::vector<uint64_t> CoreUsage; // Millipercent
        std::vector<DriveUsage> Drives;

        std::vector<char> serialise();
        void deserialise(std::vector<char> data);
};
