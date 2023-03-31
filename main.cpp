#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <cstdint>
#include <iomanip>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "status.h"


std::ifstream file;
std::string content;
std::vector<std::string> lines;

struct CpuUsage
{
    uint64_t used;
    uint64_t total;
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

        uint64_t user, unice, usystem, idle, iowait, irq, softirq, guest;
        int nb_read = sscanf (line.c_str(), "%*s %llu %llu %llu %llu %llu %llu %llu %*u %llu",
                          &user, &unice, &usystem, &idle, &iowait, &irq, &softirq, &guest);
        if (nb_read <= 4) iowait = 0;
        if (nb_read <= 5) irq = 0;
        if (nb_read <= 6) softirq = 0;
        if (nb_read <= 7) guest = 0;

        uint64_t used = user + unice + usystem + irq + softirq + guest;
        uint64_t total = used + idle + iowait;
        values.push_back({used, total});
    }
    return values;
}

int main()
{
    int sock;
    struct sockaddr_in adr{AF_INET, htons(port), htonl(address)};

    if((sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == 0)
    {
        perror("Failed socket setup");
        return 1;
    }

    int broadcast=1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        &broadcast, sizeof broadcast);


    Status data;
    std::vector<CpuUsage> prevCpuUsage = getCoreUsage();
    data.CoreUsage.resize(prevCpuUsage.size());
    data.CoreCount = prevCpuUsage.size();

    // previous network usage
    uint64_t prevIn = 0, prevOut = 0;

    while(true)
    {
        sleep(1);

        //stat (CPU usage)

        data.CoreUsage.clear();
        std::vector<CpuUsage> currentCpuUsage = getCoreUsage();

        for(int i = 0; i < currentCpuUsage.size(); i++)
        {
            int deltaUsed = currentCpuUsage[i].used - prevCpuUsage[i].used;
            int deltaTotal = currentCpuUsage[i].total - prevCpuUsage[i].total;
            data.CoreUsage.push_back(((double)deltaUsed / deltaTotal) * 100000);            
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

        uint64_t inBytes = 0, outBytes = 0;
        for(std::string line : lines)
        {
            uint64_t in, out;
            sscanf(line.c_str(), "%*s %lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %lld %*lld", &in, &out);
            inBytes += in;
            outBytes += out;
        }

        data.NetworkIn = inBytes - prevIn;
        data.NetworkOut = outBytes - prevOut;
        prevIn = inBytes;
        prevOut = outBytes;

        // Drive usage

        content = "";
        char buffer[1024];
        FILE* stream = popen("df | grep \"^/dev/\"", "r");
        while (fgets(buffer, 1024, stream) != NULL)
        {
            content += buffer;
        }
        pclose(stream);
        lines = strToLines(content);
        data.DriveCount = 0;
        data.Drives.clear();
        for(std::string line : lines)
        {
            DriveUsage newPart;
            char location[2048], name[2048];
            sscanf(line.c_str(), "%s %lld %lld %*lld %*s %s", &location, &newPart.size, &newPart.used, &name);
            newPart.location += location;
            newPart.name += name;
            data.Drives.push_back(newPart);
            data.DriveCount++;
        }
        data.print();
        std::vector<uint8_t> serialData = data.serialise();

        if(sendto(sock, serialData.data(), serialData.size(), 0, (struct sockaddr *)&adr, sizeof(adr)) != serialData.size()) 
        {
            perror("Failed send");
            return 1;
        }
    }
}