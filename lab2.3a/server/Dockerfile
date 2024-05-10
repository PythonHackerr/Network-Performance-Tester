FROM gcc:latest
RUN apt-get update && apt-get install -y iproute2
COPY . /Sockets
WORKDIR /Sockets/
RUN gcc -o server server.c -g
ENTRYPOINT ["./server"]
