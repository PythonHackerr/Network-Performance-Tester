FROM gcc:latest
RUN apt-get update && apt-get install -y iproute2
COPY . /Sockets
WORKDIR /Sockets/
RUN gcc -o client client.c -g
ENTRYPOINT ["./client", "172.21.27.2", "8827", "100"]
