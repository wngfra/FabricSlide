#pragma once

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <thread>

#include <PCANBasic.h>

#define PCAN_DEVICE PCAN_USBBUS1

using namespace std;

struct TactileBuffer
{
    int timestamp;
    array<int, 2> proximity;
    array<int, 16> data;

    int max()
    {
        int max_ = 0;
        for (auto &e : data)
        {
            if (e > max_)
                max_ = e;
        }
        return max_;
    }

    int min()
    {
        int min_ = 65536;
        for (auto &e : data)
        {
            if (e < min_)
                min_ = e;
        }
        return min_;
    }

    int average()
    {
        float sum = 0.0;
        int max = 0;
        int min = 65535;
        for (auto &e : data)
        {
            if (e > max)
                max = e;
            else if (e < min)
                min = e;
            sum += e;
        }
        sum -= max + min;
        return (sum / (float)(data.size() - 2));
    }

    friend ostream &operator<<(ostream &os, const TactileBuffer &bf)
    {
        os << bf.timestamp;
        for (auto &p : bf.proximity)
            os << "," << p;
        for (auto &d : bf.data)
            os << "," << d;
        return os;
    };
};

class CANBus
{
public:
    CANBus(chrono::steady_clock::time_point &starttime) : pcan_device_(PCAN_DEVICE), starttime_(starttime)
    {
        // Attach the shared memory
        if ((shmid = shmget(key, sizeof(int) * 19, IPC_CREAT | 0666)) < 0)
        {
            cerr << "Error getting shared memory ID!" << endl;
            exit(1);
        }
        shm = (char *)shmat(shmid, NULL, 0);
        if (shm == (char *)-1)
        {
            cerr << "Error Attaching shared memory id!" << endl;
            exit(1);
        }
        if (loadCalibration())
        {
            cout << "INFO: Calibration file loaded." << endl;
        }
        else
        {
            cerr << "No calibration file loaded, using zero bias." << endl;
        }

        mlockall(MCL_CURRENT | MCL_FUTURE);
        Status = CAN_Initialize(pcan_device_, PCAN_BAUD_1M, 0, 0, 0);
        printf("INFO: CAN_Initialize(%xh): Status=0x%x\n", pcan_device_, (int)Status);
        printf("Starting CAN-BUS...\n");
        updater = thread(&CANBus::update, this);
    };
    ~CANBus()
    {
        bus_on = false;
        if (updater.joinable())
        {
            updater.join();
        }
        CAN_Uninitialize(pcan_device_);

        shmdt(&shm);
        shmctl(shmid, IPC_RMID, NULL);

        printf("INFO: CAN-Bus shut down.\n");
    };

    int status();
    TactileBuffer read();
    void logOn(const string &, const string &);
    void logOff();

private:
    void update();
    bool loadCalibration();
    array<int, 16> bias;
    array<int, 16> channel_order = {{11, 15, 14, 12, 9, 13, 8, 10, 6, 7, 4, 5, 2, 0, 3, 1}};

    unsigned int pcan_device_;
    int status_ = 0;
    bool recording = false;
    bool bus_on = true;
    ofstream writefile_;

    thread updater;

    int shmid;
    key_t key = 1233456;
    char *shm;

    TactileBuffer buffer_;
    TPCANMsg Message;
    TPCANStatus Status;

    chrono::steady_clock::time_point starttime_;
};