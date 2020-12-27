// Copyright(c) 2020 Artonnet. All Rights Reserved.
#pragma once

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <opencv2/opencv.hpp> // Include OpenCV header file

#define SERVER_PORT 17889           // Socket server port
#define SERVER_ADDR "192.168.0.110" // Socket server port

#define IMG_WIDTH 640  // Default image width
#define IMG_HEIGHT 480 // Default image height

// Image size
#define IMG_SIZE_8UC1 IMG_WIDTH * IMG_HEIGHT
#define IMG_SIZE_16UC1 2 * IMG_SIZE_8UC1
#define IMG_SIZE_8UC3 3 * IMG_SIZE_8UC1

using namespace std;

#pragma pack(1)
struct Info
{
    unsigned int type; // 4 bytes
    double timestamp; // 16 bytes
};

struct Header
{
    const char delimiter[6] = "Arton";
    unsigned int len;
    Info info;
};
#pragma pack()

class SocketClient
{
private:
    int client;

public:
    SocketClient(void);
    ~SocketClient(void);

    void init(void);
    int transmit(unsigned int type, cv::Mat &img, double timestamp);
};
