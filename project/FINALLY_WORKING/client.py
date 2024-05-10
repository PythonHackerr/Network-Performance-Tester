import argparse
import socket
import struct
import random
import time

BUF_SIZE = 1024
#BUF_SIZE = 1024
HOST_IP = ''

speed_statistic = False
calculate_delay = False

def measure_one_packet_speed(one_packet_start_time, one_packet_end_time, message, i):
    #measure speed for upload one packet
    one_packet_end_time = time.time()
    one_packet_duration = one_packet_end_time - one_packet_start_time
    one_packet_speed_upload_bytes = len(message) / one_packet_duration
    one_packet_speed_upload_KBps = one_packet_speed_upload_bytes / 1024
    print(f"Speed {round(one_packet_speed_upload_bytes,2)} Bytes/sec - {round(one_packet_speed_upload_KBps,2)} KB/s - {round(one_packet_speed_upload_KBps/1024,2)} MB/s - (Packet number: {i})!")


def measure_total_speed(total_start_time, total_bytes_sent):
    # check upload duration and speed
    total_end_time = time.time()
    total_duration = total_end_time - total_start_time
    #print(f'\nStopped timer - end_time {total_end_time}')
    total_speed_upload_bytes = total_bytes_sent / total_duration
    total_speed_upload_KBps = total_speed_upload_bytes / 1024

    print(f'\nTotal bytes sent: {total_bytes_sent}')
    print(f"Total duration: {round(total_duration,2)} seconds")
    print(f"Total upload speed: {round(total_speed_upload_bytes,2)} Bytes/sec - {round(total_speed_upload_KBps,2)} KB/s - {round(total_speed_upload_KBps/1024,2)} MB/s")
    

def process_ack(s, message, server_address, is_last, send_time):
    while True:
        try:
            # receive ack from server
            packet, addr = s.recvfrom(BUF_SIZE)
            if packet:
                print("Received ACK for message. Data was correct")

                if calculate_delay:
                    receive_time = time.time()
                    rtt = receive_time - send_time
                    print(f"RTT for packet: {rtt} seconds")

                if is_last:
                    print(f"Ending communication with server on {addr[0]}:{addr[1]}")
                else:
                    print(f"Starting communication with server on {addr[0]}:{addr[1]}")
                break
        except socket.timeout:
            # in case of timeout, send first packet again
            print("ACK not received...\nSending packet again")
            time.sleep(0.1)
            s.sendto(message, server_address)
            print(f"Sent packet again!")


def main():
    global speed_statistic, calculate_delay

    # parse arguments - server ip, server port, and number of packets
    parser = argparse.ArgumentParser()
    parser.add_argument("server_ip")
    parser.add_argument("server_port")
    parser.add_argument("packets")
    parser.add_argument("--speed_statistic", action="store_true", help="Enable speed statistics")
    parser.add_argument("--delay", action="store_true", help="Calculate packet delay")
    args = parser.parse_args()

    # set flags based on command line arguments
    speed_statistic = args.speed_statistic
    calculate_delay = args.delay

    server_address = (args.server_ip, int(args.server_port))

    # open socket for communication
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        # set timeout for resending packet if ack was not received
        s.settimeout(2)

        s.bind((HOST_IP, int(args.server_port)))
        print(f"Client's listening on port: {s.getsockname()[1]}")

        # variables for checking speed upload
        total_bytes_sent = 0
        total_start_time = time.time()

        # send first packet with info about number of packets
        packets = struct.pack('!I', int(args.packets))
        s.sendto(packets, server_address)

        # repeat until ack for first msg received
        process_ack(s, packets, server_address, 0, time.time() if calculate_delay else None)

        # Is last packet flag
        is_last = 0

        for i in range(int(args.packets)):
            # change flag if packet is last
            if i == int(args.packets) - 1:
                is_last = 1

            # create random bytes
            random_bytes = bytes([random.randint(0, 255) for _ in range(BUF_SIZE - 8 - struct.calcsize('!Bd'))])

            # construct message
            if calculate_delay:
                timestamp = time.time()
                message = struct.pack('!Bd{0}s'.format(len(random_bytes)), is_last, timestamp, random_bytes)
            else:
                message = struct.pack('!B{0}s'.format(len(random_bytes) + 1), is_last, random_bytes)
            
            one_packet_start_time = time.time() 
            s.sendto(message, server_address)
            one_packet_end_time = time.time()

            print(f"Sent {len(message)} Bytes to server - (Packet number: {i})!")
            
            # speed statistic for one packet
            if speed_statistic:
                measure_one_packet_speed(one_packet_start_time, one_packet_end_time, message, i)
                
            # update the total bytes sent
            total_bytes_sent += len(message)

            # process ack and calculate delay
            if is_last:
                if calculate_delay:
                    process_ack(s, message, server_address, is_last, one_packet_start_time)
                else:
                    process_ack(s, message, server_address, is_last, None)

        # speed statistic for all packets
        if speed_statistic:
            measure_total_speed(total_start_time, total_bytes_sent)


if __name__ == "__main__":
    main()
