import argparse
import socket
import struct
import random
import time

BUF_SIZE = 1024
#BUF_SIZE = 32768
HOST_IP = '0.0.0.0'
            
def packet_loss(received_packets, packets_quantity):
    lost_packets = packets_quantity - received_packets
    print(f"Received packets: {received_packets}, expected: {packets_quantity}, lost packets: {lost_packets}" )

def measure_one_packet_speed(one_packet_start_time, one_packet_end_time, random_bytes):
     #measure speed for download one packet
    one_packet_duration = one_packet_end_time - one_packet_start_time
    one_packet_speed_upload_bytes = len(random_bytes) / one_packet_duration
    one_packet_speed_upload_KBps = one_packet_speed_upload_bytes / 1024

    print(f"Speed {round(one_packet_speed_upload_bytes,2)} Bytes/sec - {round(one_packet_speed_upload_KBps,2)} KB/s - {round(one_packet_speed_upload_KBps/1024,2)} MB/s")


def measure_total_speed(total_start_time, total_bytes_received):
     # check download duration and speed
    total_end_time = time.time()
    total_duration = total_end_time - total_start_time
    #print(f'\nStopped timer - end_time {total_end_time}')
    total_speed_download_bytes = total_bytes_received / total_duration
    total_speed_download_KBps = total_speed_download_bytes / 1024

    print(f'\nTotal bytes received: {total_bytes_received}')
    print(f"Total Duration: {round(total_duration,2)} seconds")
    print(f"Total download speed: {round(total_speed_download_bytes,2)} Bytes/sec - {round(total_speed_download_KBps,2)} KB/s - {round(total_speed_download_KBps/1024,2)} MB/s")


def measure_total_delay():
    # global total_packet_delay
    average_delay = total_packet_delay / total_packets_received if total_packets_received > 0 else 0
    print(f"\nTotal delay: {total_packet_delay} seconds")
    print(f"Average packet delay: {average_delay} seconds")


def start_communication(s):
    while True:
        # receive packet from client
        info_packet, addr = s.recvfrom(BUF_SIZE)
        # simulate loosing packets in 30% of cases
        if random.random() < 0.0:
            continue
        # unpack info about expected number of packets
        packets_quantity = struct.unpack('!I', info_packet)[0]
        # send ACK to client
        print("Sending ACK to client...")
        s.sendto(bytes(packets_quantity), addr)
        print(f'Communication was started. Expecting to get {packets_quantity} packets...')
        return packets_quantity

def handle_communication(s, packets_quantity):
    # variables for checking speed download
    global total_packets_received
    global total_packet_delay
    total_bytes_received = 0
    total_packets_received = 0 
    total_packet_delay = 0 

    total_start_time = time.time()
    #print(f'Started timer - start_time {total_start_time}')

    #variables for checking number of packet lost and length of packet
    received_packets = 0


    while(True):
        # receive packet from client
        one_packet_start_time = time.time()
        msg, adr = s.recvfrom(BUF_SIZE)
        one_packet_end_time = time.time()

        # simulate loosing packets in 30% of cases
        # if random.random() < 0.3:
        #     msg = 0
        # if msg is empty print info and continue
        if not msg:
            print("Error in message")
            continue

        received_packets += 1
        total_packet_delay += (time.time() - one_packet_start_time)
        # unpack received message
        try:
            is_last, random_bytes = struct.unpack('!B{0}s'.format(len(msg) - 1), msg)

        except UnicodeDecodeError:
            print(type(msg))
            decoded_msg = "[Cannot decode message]"
        # print info about message and client
        print(f'Client IP: {adr}')
        print(f'Is it last packet: {is_last}')
        print(f'Client msg: {random_bytes[:10]}')

         
        # speed statistic for one packet
        if (speed_statistic):
            measure_one_packet_speed(one_packet_start_time, one_packet_end_time, random_bytes)      

        if calculate_delay:
            measure_total_delay()
        # update the total bytes received
        total_bytes_received += len(random_bytes)

        # if is_last flag is 1 end communication
        if is_last:
            # send ACK
            print("Sending ACK to client...")
            s.sendto(random_bytes, adr)
            break
    
    # speed statistic for all packets
    if (speed_statistic):
        measure_total_speed(total_start_time, total_bytes_received)
    # calculate packet loss
    if (packet_loss_statistic):
        packet_loss(received_packets, packets_quantity)

          
   


def main():
    global speed_statistic
    global packet_loss_statistic
    global calculate_delay
    # parse argument - port of server
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("--speed_statistic", action="store_true", help="Enable speed statistics")
    parser.add_argument("--packet_loss_statistic", action="store_true", help="Enable packet loss statistics")
    parser.add_argument("--delay", action="store_true", help="Calculate packet delay")

    args = parser.parse_args()

    # set speed_statistic based on command line argument
    speed_statistic = args.speed_statistic

    #set packet_loss_statistic based on command line argument
    packet_loss_statistic = args.packet_loss_statistic

    #set calculate_delay based on command line argument
    calculate_delay = args.delay


    # open socket for communication
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((HOST_IP, int(args.port)))
        print(f"Server's listening on: {HOST_IP} and port: {args.port}")
        
        # repeat communication process
        while True:
            packets_quantity = start_communication(s)
            handle_communication(s, packets_quantity)



if __name__ == "__main__":
    main()
