FROM gcc:latest
COPY . /Sockets
WORKDIR /Sockets/
RUN gcc -o client client.c
ENTRYPOINT ["./client"]
