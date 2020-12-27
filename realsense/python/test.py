# First import the library
import socket
import time

import pyrealsense2 as rs
import numpy as np
import cv2

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#client.connect(("192.168.2.100", 17888))

# Create a context object. This object owns the handles to all connected realsense devices
config = rs.config()
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 60)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 60)
pipeline = rs.pipeline()
pipeline.start(config)

try:
    while True:
        # Create a pipeline object. This object configures the streaming camera and owns it's handle
        frames = pipeline.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_color_frame()
        if not depth_frame or not color_frame:
            continue

        # 获取内参
        dprofile = depth_frame.get_profile()
        cprofile = color_frame.get_profile()
        cvsprofile = rs.video_stream_profile(cprofile)
        dvsprofile = rs.video_stream_profile(dprofile)

        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())
        depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)
        images = np.hstack((color_image, depth_colormap))
        cv2.imwrite("/rs/out.png", images)
        #client.sendall(np_image)
        time.sleep(5)

finally:
    pipeline.stop()
    client.close()
