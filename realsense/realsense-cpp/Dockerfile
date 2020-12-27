FROM registry.cn-shenzhen.aliyuncs.com/artonnet/realsense:2.0-resp4b

ADD ./cpp /app

WORKDIR /app
RUN cmake . && \
  make -j4

