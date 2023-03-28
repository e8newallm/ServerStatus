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
    uint8_t* addr = &vector.back() - 7;
    uint64_t value = (addr[0] << 56) + (addr[1] << 48) + (addr[2] << 40) + (addr[3] << 32) + 
                     (addr[4] << 24) + (addr[5] << 16) + (addr[6] <<  8) + addr[7];
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
    pushLong(data, CoreCount);
    for(uint64_t core : CoreUsage)
    {
        pushLong(data, core);
    }
    pushLong(data, DriveCount);
    for(DriveUsage drive : Drives)
    {
        data.insert(data.end(), drive.name.begin(), drive.name.end());
        data.insert(data.end(), drive.location.begin(), drive.location.end());
        pushLong(data, drive.size);
        pushLong(data, drive.used);
    }

    pushLong(data, identifierRev);
    return data;
}

void Status::deserialise(std::vector<uint8_t> data)
{
    if(popLong(data) != identifierRev)
        return;

    
}