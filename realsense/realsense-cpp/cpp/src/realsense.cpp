// Copyright(c) 2020 Artonnet. All Rights Reserved.

#include <iostream>
#include <chrono>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // include OpenCV header file

#include "socket.h"

using namespace std;

int main(int argc, char *argv[])
{
    // Flag
    bool flag_depth = false;

    // Socket client
    SocketClient client;

    // Contruct a pipeline which abstracts the device
    rs2::pipeline pipe;

    // Create a configuration for configuring the pipeline with a non default profile
    rs2::config cfg;

    // Add desired streams to configuration
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
    cfg.enable_stream(RS2_STREAM_INFRARED, 640, 480, RS2_FORMAT_Y8, 30);
    if (flag_depth) cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);

    int retry = 0;
    while (true)
    {
        if (retry > 30)
        {
            return EXIT_FAILURE;
        }

        // Instruct pipeline to start streaming with the requested configuration
        try
        {
            pipe.start(cfg);

            // Camera warmup - dropping several first frames to let auto-exposure stabilize
            rs2::frameset frames;
            for (int i = 0; i < 30; i++)
            {
                // Wait for all configured streams to produce a frame
                frames = pipe.wait_for_frames();
            }

            break;
        }
        catch (const rs2::error &e)
        {
            cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
            retry--;
            sleep(5);
            continue;
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
            retry--;
            sleep(5);
            continue;
        }
    }

    // Time
    const auto starttime = std::chrono::steady_clock::now();
    while (true)
    {
        // Delay
        // usleep(500);

        try
        {
            rs2::frameset frames;

            // Wait for all configured streams to produce a frame
            frames = pipe.wait_for_frames();

            // Calc time shift
            const auto currtime = std::chrono::steady_clock::now();
            const auto timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(currtime - starttime).count();

            // Get each frame
            rs2::frame color_frame = frames.get_color_frame();
            rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED);
            rs2::frame depth_frame;

            if (flag_depth) depth_frame = frames.get_depth_frame();

            // Creating OpenCV Matrix from a color image
            cv::Mat color_image(cv::Size(640, 480), CV_8UC3, (void *)color_frame.get_data(), cv::Mat::AUTO_STEP);

            // IR image
            cv::Mat ir_image(cv::Size(640, 480), CV_8UC1, (void*)ir_frame.get_data(), cv::Mat::AUTO_STEP);

            // Depth image
            cv::Mat* depth_image;
            if (flag_depth) depth_image = new cv::Mat(cv::Size(640, 480), CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

            // OpenCV save pic
            // cv::imwrite("./out.png", color_image);

            // Send to server
            client.transmit(1, color_image, timestamp);
            client.transmit(2, ir_image, timestamp);
            if (flag_depth) client.transmit(3, *depth_image, timestamp);
        }
        catch (const rs2::error &e)
        {
            cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
            continue;
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
            continue;
        }
    }
}