FROM ubuntu:18.04

WORKDIR /

ENV DEBIAN_FRONTEND noninteractive

RUN \
  echo 'APT::Acquire::Retries "3";' > /etc/apt/apt.conf.d/80-retries && \
  sed -i 's#ports.ubuntu.com#mirrors.aliyun.com#g' /etc/apt/sources.list

RUN \
  apt-get -y update && \
  apt-get -y upgrade && \
  apt-get -y install sudo curl wget git cmake && \
  apt-get -y install --install-recommends libssl-dev libusb-1.0-0-dev pkg-config libgtk-3-dev libopencv-dev

# git clone https://github.com/IntelRealSense/librealsense.git
ADD ./librealsense.tar.gz /
RUN \
  cd /librealsense && \
  mkdir build && cd build && \
  cmake ../ -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=false -DBUILD_GRAPHICAL_EXAMPLES=false -DFORCE_RSUSB_BACKEND=ON -DBUILD_WITH_TM2=false -DIMPORT_DEPTH_CAM_FW=false && \
  sudo make uninstall && \
  make clean && \
  make -j4 && \
  sudo make install && \
  cd /librealsense && \
  mkdir -p /etc/udev/rules.d && \
  ./scripts/setup_udev_rules.sh

# cd librealsense/examples/hello-realsense
# g++ -std=c++11 rs-hello-realsense.cpp -lrealsense2 -lopencv_core -lopencv_highgui -o hello
