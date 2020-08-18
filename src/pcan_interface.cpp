#include <iostream>
#include <chrono>
#include <experimental/filesystem>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <PCANBasic.h>

#include "pcan_interface.h"

using namespace std;

/**********************************************
 * Update function to run in a separate thread
 * Keep fetching data from the canbus
 * `buffer_` keeps the latest data
 * If `recording` flag set, writes data to file
 * Switch on and off with `bus_on` flag
 *********************************************/
void CANBus::update()
{
    size_t count, order;
    unsigned int sid;
    array<int, 2> proximity;
    array<int, 16> data;

    while (this->bus_on)
    {
        proximity.fill(0);
        data.fill(0);

        count = 0;
        order = 0;

        while (count < 5)
        {
            while ((Status = CAN_Read(pcan_device_, &Message, NULL)) == PCAN_ERROR_QRCVEMPTY)
                if (usleep(100))
                    break;
            if (Status != PCAN_ERROR_OK)
            {
                printf("CAN_Read(%xh) failure 0x%x\n", pcan_device_, (int)Status);
                throw runtime_error("CANBus stopped!");
            }

            sid = (int)Message.ID;

            if (sid == 0x405 && count == 0)
            {
                for (int i = 0; i < 4; ++i)
                {
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i + 1];
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i];
                    order += 1;
                }
                count = 1;
            }
            else if (sid == 0x407 && count == 1)
            {
                for (int i = 0; i < 4; ++i)
                {
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i + 1];
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i];
                    order += 1;
                }
                count = 2;
            }
            else if (sid == 0x409 && count == 2)
            {
                for (int i = 0; i < 4; ++i)
                {
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i + 1];
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i];
                    order += 1;
                }
                count = 3;
            }
            else if (sid == 0x40b && count == 3)
            {
                for (int i = 0; i < 4; ++i)
                {
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i + 1];
                    data[channel_order[order]] = (data[channel_order[order]] << 8) + Message.DATA[2 * i];
                    order += 1;
                }
                count = 4;
            }
            else if (sid == 0x601 && count == 4)
            {
                for (int i = 0; i < 2; ++i)
                {
                    proximity[i] = (proximity[i] << 8) + Message.DATA[2 * i + 1];
                    proximity[i] = (proximity[i] << 8) + Message.DATA[2 * i];
                }
                count = 5;
            }
        }

        auto now = chrono::steady_clock::now();
        chrono::duration<double> duration = chrono::duration_cast<chrono::duration<double>>(now - starttime_);

        for (int i = 0; i < data.size(); ++i)
        {
            data[i] -= bias[i];
        }

        buffer_.timestamp = int(duration.count() * 1000);
        buffer_.proximity = proximity;
        buffer_.data = data;

        float avg = buffer_.average();
        if (avg >= 3000 || avg <= -100)
        {
            this->logOff();
            this->bus_on = false;
        }
        int contents[19];
        int i = 1;
        contents[0] = buffer_.timestamp;
        for (auto const &e : proximity)
        {
            contents[i] = e;
            ++i;
        }
        for (auto const &e : data)
        {
            contents[i] = e;
            ++i;
        }

        memcpy(shm, &contents[0], sizeof(int) * 19);

        if (this->recording && writefile_.is_open())
        {
            writefile_ << buffer_ << endl;
        }
    }
}

bool CANBus::loadCalibration()
{
    namespace fs = std::experimental::filesystem;
    if (fs::exists("../calibration.txt"))
    {
        ifstream file("../calibration.txt");
        for (auto &b : bias)
        {
            file >> b;
        }
        return true;
    }
    else
    {
        bias.fill(0);
        return false;
    }
}

int CANBus::status()
{
    return status_;
}

// Read one data from buffer
TactileBuffer CANBus::read()
{
    return buffer_;
}

/**********************************
 * Start Logging to file
 * If `path` not exists, create one
 **********************************/
void CANBus::logOn(const string &path, const string &filename)
{
    namespace fs = std::experimental::filesystem;
    if (!fs::is_directory(path) || !fs::exists(path))
    {
        fs::create_directory(path);
    }
    writefile_.open(path + filename);
    writefile_ << "timestamp, proximity1, proximity2";
    for (int i = 0; i < 16; ++i)
    {
        writefile_ << ", channel" << i + 1;
    }
    writefile_ << endl;
    this->recording = true;
}

// Stop logging to files
void CANBus::logOff()
{
    this->recording = false;
    if (writefile_.is_open())
    {
        writefile_.close();
    }
}
