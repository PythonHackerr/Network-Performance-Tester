docker rm z27_server2_py
docker build -t z27_server2_py .
docker run -it --network z27_network --name z27_server2_py z27_server2_py
