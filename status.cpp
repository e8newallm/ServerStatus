#include <iostream>

#include "status.h"

void pushLong(std::vector<uint8_t>& vector, uint64_t value)
{
    vector.push_back((value >> 56) & 0xFF);
    vector.push_back((value >> 48) & 0xFF);
    vector.push_back((value >> 40) & 0xFF);
    vector.push_back((value >> 32) & 0xFF);
    vector.push_back((value >> 24) & 0xFF);
    vector.push_back((value >> 16) & 0xFF);
    vector.push_back((value >> 8) & 0xFF);
    vector.push_back(value & 0xFF);
}

uint64_t popLong(std::vector<uint8_t>& vector)
{
    uint8_t* addr = (&vector.back()) - 7;
    uint64_t value = ((uint64_t)addr[0] << 56) + ((uint64_t)addr[1] << 48) + ((uint64_t)addr[2] << 40) + ((uint64_t)addr[3] << 32) + 
                     ((uint64_t)addr[4] << 24) + ((uint64_t)addr[5] << 16) + ((uint64_t)addr[6] <<  8) + (uint64_t)addr[7];
    vector.resize(vector.size() - 8);
    return value;
}

void Status::print()
{
    std::cout << "MemTotal: " << MemTotal << "\r\n";
    std::cout << "MemFree: " << MemFree << "\r\n";
    std::cout << "SwapTotal: " << SwapTotal << "\r\n";
    std::cout << "SwapFree: " << SwapFree << "\r\n";
    std::cout << "Uptime: " << Uptime << "\r\n";
    std::cout << "NetworkIn: " << NetworkIn << "\r\n";
    std::cout << "NetworkOut: " << NetworkOut << "\r\n";
    std::cout << "CoreCount: " << CoreCount << "\r\n";
    for(int i = 0; i < CoreUsage.size(); i++)
    {
        std::cout << "\tcore " << i << ": " << CoreUsage[i] / 1000.0f << "%\r\n";
    }
    std::cout << "DriveCount: " << DriveCount << "\r\n";
    for(int i = 0; i < Drives.size(); i++)
    {
        std::cout << "Parition " << Drives[i].location << " mounted at " << Drives[i].name << ": " 
            << Drives[i].used << " used of " << Drives[i].size << "\r\n";
    }
}

std::vector<uint8_t> Status::serialise()
{
    std::vector<uint8_t> data;
    data.reserve(16 + sizeof(Status) + sizeof(uint64_t)*CoreUsage.size() + sizeof(DriveUsage)*Drives.size()); //Rough estimate reservation
    pushLong(data, identifier);

    pushLong(data, MemTotal);
    pushLong(data, MemFree);
    pushLong(data, SwapTotal);
    pushLong(data, SwapFree);
    pushLong(data, Uptime);
    pushLong(data, NetworkIn);
    pushLong(data, NetworkOut);
    for(uint64_t core : CoreUsage)
    {
        pushLong(data, core);
    }
    pushLong(data, CoreCount);
    for(DriveUsage drive : Drives)
    {
        data.insert(data.end(), drive.name.begin(), drive.name.end());
        pushLong(data, drive.name.size());
        data.insert(data.end(), drive.location.begin(), drive.location.end());
        pushLong(data, drive.location.size());
        pushLong(data, drive.size);
        pushLong(data, drive.used);
    }
    pushLong(data, DriveCount);

    pushLong(data, identifierRev);
    return data;
}

void Status::deserialise(std::vector<uint8_t> data)
{
    uint64_t testData = popLong(data);
    if(testData != identifierRev)
    {
        std::cout << std::hex << "popLong(data): " << testData  << " identifierRev: " << identifierRev << std::dec << "\r\n";
        return;
    }

    DriveCount = popLong(data);
    Drives = std::vector<DriveUsage>(DriveCount, {"", "", 0, 0});
    for(int i = DriveCount-1; i >= 0; i--)
    {
        Drives[i].used = popLong(data);
        Drives[i].size = popLong(data);
        uint64_t size = popLong(data);
        Drives[i].location = std::string(data.end() - size, data.end());
        data.resize(data.size() - size);
        size = popLong(data);
        Drives[i].name = std::string(data.end() - size, data.end());
        data.resize(data.size() - size);
    }
    CoreCount = popLong(data);
    CoreUsage.resize(CoreCount);
    for(int i = CoreCount-1; i >= 0; i--)
    {
        CoreUsage[i] = popLong(data);
    }
    NetworkOut = popLong(data);
    NetworkIn = popLong(data);
    Uptime = popLong(data);
    SwapFree = popLong(data);
    SwapTotal = popLong(data);
    MemFree = popLong(data);
    MemTotal = popLong(data);
    
    if(popLong(data) != identifier)
        return;
}