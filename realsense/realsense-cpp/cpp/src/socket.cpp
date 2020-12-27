// Copyright(c) 2020 Artonnet. All Rights Reserved.

#include "socket.h"

SocketClient::SocketClient(void)
{
    init();
}

SocketClient::~SocketClient(void)
{
    if (-1 != client)
    {
        close(client);
    }
}

void SocketClient::init(void)
{
    // socket
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == client)
    {
        cout << "Error: socket" << endl;
        return;
    }

    // connect
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "Error: connect" << endl;
        close(client);
        client = -1;
        return;
    }
}

int SocketClient::transmit(unsigned int type, cv::Mat &image, double timestamp)
{
    if (-1 != client)
    {
        size_t img_size;
        switch (type)
        {
        case 1:
            img_size = IMG_SIZE_8UC3;
            break;
        case 2:
            img_size = IMG_SIZE_8UC1;
            break;
        case 3:
            img_size = IMG_SIZE_16UC1;
            break;
        default:
            img_size = IMG_SIZE_8UC3;
        }

        Header header;
        // memset((char *)(&header.image), 0, IMG_SIZE_8UC3);
        // memcpy((char *)(&data.image), image.ptr<uchar>(0), img_size);
        header.len = img_size + sizeof(Info); // body includes header.info
        header.info.type = type;
        header.info.timestamp = timestamp;
        send(client, (char *)(&header), sizeof(header), 0);
        send(client, image.ptr<uchar>(0), img_size, 0);
    }
    else
    {
        init();
    }
}