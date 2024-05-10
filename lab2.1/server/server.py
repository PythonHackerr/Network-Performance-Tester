import argparse
import socket
import struct

BUF_SIZE = 1024
HOST_IP = ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as serv_s:
        serv_s.bind((HOST_IP, int(args.port)))
        serv_s.listen(5)
        print(f"Server's listening on: {HOST_IP} and port: {args.port}")
        while True:
            client_s, addr = serv_s.accept()
            with client_s:
                print("Connect from: ", addr)
                while True:
                    msg = client_s.recv(BUF_SIZE)
                    if not msg:
                        print("Message ended")
                        break
                    try:
                        packet_number, packet_length = struct.unpack("!I H", msg[:6])
                        data = msg[6:6+packet_length]
                        print(f'Packet Number: {packet_number}, Packet Length: {packet_length}, Data: {data[:10]}')
                    except UnicodeDecodeError:
                        print("Error decoding message")
                        break
                    # client_s.sendall(decoded_msg)




if __name__ == "__main__":
    main()
