import argparse
import socket
import struct

BUF_SIZE = 1024
HOST_IP = ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((HOST_IP, int(args.port)))
        print(f"Server's listening on: {HOST_IP} and port: {args.port}")
        
        while True:
            msg, adr = s.recvfrom(BUF_SIZE)
            if not msg:
                print("Error in message")
                break
            try:
                print(f'message: {msg}')
                decoded_msg = msg
            except UnicodeDecodeError:
                print(type(msg))
                decoded_msg = "[Cannot decode message]"

            print(f'Client IP: {adr}')
            print(f'Client msg: {decoded_msg}')

            # Pobranie numeru otrzymanego pakietu z wiadomości
            packet_number = struct.unpack("!I", msg[:4])[0]

            # Przygotowanie potwierdzenia z numerem otrzymanego pakietu
            print(f'Received packet with number: {packet_number}')
            # Send ACK back to the client
            ack_msg = struct.pack("!I", packet_number)  # ACK message with the same packet number
            s.sendto(ack_msg, adr)
            print(f'Sent ACK for packet number: {packet_number}')

if __name__ == "__main__":
    main()
