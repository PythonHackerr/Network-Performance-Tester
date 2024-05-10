docker rm z27_server2_c
docker build -t z27_server2_c .
docker run -it --network z27_network --name z27_server2_c z27_server2_c
