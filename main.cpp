#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <fstream>

std::ifstream file;
std::string content;
std::vector<std::string> lines;

struct Status
{
    unsigned long MemTotal = 0; // kB
    unsigned long MemFree = 0; // kB
    unsigned long SwapTotal = 0; // kB
    unsigned long SwapFree = 0; // kB
    unsigned long Uptime = 0; // Seconds
    unsigned long NetworkIn = 0; // Bytes
    unsigned long NetworkOut = 0; // Bytes
    unsigned long CoreCount = 0;
    std::vector<unsigned long> CoreUsage; // Millipercent
};

struct CpuUsage
{
    unsigned long used;
    unsigned long total;
};

std::vector<std::string> strToLines(std::string content)
{
    std::vector<std::string> lines;
    for(std::size_t start = 0, i = content.find('\n', 0); i != std::string::npos; start = i+1, i = content.find('\n', i+1))
    {
        lines.push_back(content.substr(start, i-start+1));
    }
    return lines;
}

std::vector<CpuUsage> getCoreUsage()
{
    std::vector<CpuUsage> values;
    file.open("/proc/stat");
    content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    file.close();
    lines = strToLines(content);
    lines.erase(lines.begin());
    for( std::string line : lines)
    {
        if(!line.starts_with("cpu"))
            break;

        unsigned long long int user, unice, usystem, idle, iowait, irq, softirq, guest;
        int nb_read = sscanf (line.c_str(), "%*s %llu %llu %llu %llu %llu %llu %llu %*u %llu",
                          &user, &unice, &usystem, &idle, &iowait, &irq, &softirq, &guest);
        if (nb_read <= 4) iowait = 0;
        if (nb_read <= 5) irq = 0;
        if (nb_read <= 6) softirq = 0;
        if (nb_read <= 7) guest = 0;

        unsigned long used = user + unice + usystem + irq + softirq + guest;
        unsigned long total = used + idle + iowait;
        values.push_back({used, total});
    }
    return values;
}

int main()
{
    Status data;
    std::vector<CpuUsage> prevCpuUsage = getCoreUsage();
    data.CoreUsage.resize(prevCpuUsage.size());
    data.CoreCount = prevCpuUsage.size();

    // current network usage
    unsigned long prevIn = 0;
    unsigned long prevOut = 0;
    file.open("/proc/net/dev");
    content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    file.close();
    lines = strToLines(content);
    lines.erase(lines.begin());
    lines.erase(lines.begin());

    for(std::string line : lines)
    {
        unsigned long in, out;
        sscanf(line.c_str(), "%*s %lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %lld %*lld", &in, &out);
        prevIn += in;
        prevOut += out;
    }
    sleep(1);

    while(true)
    {
        //stat (CPU usage)

        data.CoreUsage.clear();
        std::vector<CpuUsage> currentCpuUsage = getCoreUsage();

        for(int i = 0; i < currentCpuUsage.size(); i++)
        {
            int deltaUsed = currentCpuUsage[i].used - prevCpuUsage[i].used;
            int deltaTotal = currentCpuUsage[i].total - prevCpuUsage[i].total;
            data.CoreUsage[i] = ((double)deltaUsed / deltaTotal) * 100000;            
        }

        prevCpuUsage = currentCpuUsage;

        //meminfo

        file.open("/proc/meminfo");
        content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        file.close();
        lines = strToLines(content);

        sscanf(lines[0].c_str(), "MemTotal: %d", &data.MemTotal);
        sscanf(lines[1].c_str(), "MemFree: %d", &data.MemFree);
        sscanf(lines[14].c_str(), "SwapTotal: %d", &data.SwapTotal);
        sscanf(lines[15].c_str(), "SwapFree: %d", &data.SwapFree);

        //uptime

        file.open("/proc/uptime");
        std::getline(file, content);
        file.close();
        sscanf(content.c_str(), "%d %*d", &data.Uptime);

        // network usage

        file.open("/proc/net/dev");
        content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        file.close();
        lines = strToLines(content);
        lines.erase(lines.begin());
        lines.erase(lines.begin());

        unsigned long inBytes = 0, outBytes = 0;
        for(std::string line : lines)
        {
            unsigned long in, out;
            sscanf(line.c_str(), "%*s %lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %lld %*lld", &in, &out);
            inBytes += in;
            outBytes += out;
        }

        data.NetworkIn = inBytes - prevIn;
        data.NetworkOut = outBytes - prevOut;
        prevIn = inBytes;
        prevOut = outBytes;

        std::cout << "networkout: " << data.NetworkOut << " network in: " << data.NetworkIn << "\r\n";

        // Sleep

        sleep(1);
    }
}