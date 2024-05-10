import argparse
import socket
import sys

BUF_SIZE = 1024
HOST_IP = ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((HOST_IP, int(args.port)))
        print(f"Server's listening on: {HOST_IP} and port: {args.port}")
        
        while(True):
            msg, adr = s.recvfrom(BUF_SIZE)
            if not msg:
                print("Error in message")
                break
            try:
                # print(f'message: {msg}')
                decoded_msg = msg
                # send response
                s.sendto(msg, adr)
            except UnicodeDecodeError:
                print(type(msg))
                decoded_msg = "[Cannot decode message]"

            byte = b''
            print(f'Client IP: {adr}')
            print(f'Client IP length: {sys.getsizeof(adr[0])} {sys.getsizeof(adr[1])}')
            print(f'Client msg length: {sys.getsizeof(msg)}')
            # print(f'Client msg: {msg}')

if __name__ == "__main__":
    main()
