docker stop realsense_2.0-resp4b
docker start realsense_2.0-resp4b
docker exec -d realsense_2.0-resp4b /bin/bash /app/RSCPP
#docker container prune -f
#docker image prune -f
