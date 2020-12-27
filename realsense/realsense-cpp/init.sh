REALSENSE_PATH=/data/realsense/realsense-cpp/cpp
DOCKER_IMAGE=registry.cn-shenzhen.aliyuncs.com/artonnet/realsense:2.0-resp4b

docker container prune -f
docker run -it --privileged -v $REALSENSE_PATH:/app --name=realsense_2.0-resp4b $DOCKER_IMAGE /bin/bash /app/buildrun.sh
