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
    unsigned long CoreCount = 0;
    std::vector<unsigned long> CoreUsage; // Millipercent
};

struct CpuUsage
{
    unsigned long idle;
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

        std::vector<int> vect;
        int value;
        std::string cpuID;
        std::stringstream ss(line); 
        ss >> cpuID;
        while(ss >> value)
            vect.push_back (value); 

        unsigned long idle = vect[3];
        unsigned long total = 0;
        for(unsigned long val : vect)
            total += val;
        values.push_back({idle, total});
    }
    return values;
}

int main()
{
    Status data;
    std::vector<CpuUsage> prevCpuUsage = getCoreUsage();
    data.CoreUsage.resize(prevCpuUsage.size());
    data.CoreCount = prevCpuUsage.size();
    sleep(2);

    while(true)
    {
        //stat (CPU usage)

        data.CoreUsage.clear();
        std::vector<CpuUsage> currentCpuUsage = getCoreUsage();

        for(int i = 0; i < currentCpuUsage.size(); i++)
        {
            int deltaIdle = currentCpuUsage[i].idle - prevCpuUsage[i].idle;
            int deltaTotal = currentCpuUsage[i].total - prevCpuUsage[i].total;
            data.CoreUsage[i] = (1.0f - ((double)deltaIdle / deltaTotal)) * 100000;
            std::cout << "Core " << i << " idle: " << deltaIdle << " total: " << deltaTotal << " percent: " << data.CoreUsage[i] / 1000.0f << "\r\n";
            
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

        sleep(2);
    }
}