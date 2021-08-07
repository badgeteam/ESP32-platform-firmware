import socket, time, _thread

class dnsserver:
    _dns_socket = None
    _ipaddr = None
    def __init__(self,ipaddr = "192.168.4.1"):
        self._ipaddr = ipaddr

    def start(self):
        #print("Starting DNS @ ",self._ipaddr)
        self._dns_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._dns_socket.bind((self._ipaddr, 53))
        self._server = _thread.start_new_thread("dnsserver",self.dnsserver_thread,[])

    def dnsserver_thread(self):
        while True:
            time.sleep(0.1)
            try:
                data, addr = self._dns_socket.recvfrom(4096)
                #print('[DNS] Client connected from', addr)

                if len(data) < 13:
                    print('[DNS] Ignored request 1')
                    return

                domain = ''
                tipo = (data[2] >> 3) & 15  # Opcode bits
                if tipo == 0:  # Standard query
                    ini = 12
                    lon = data[ini]
                    while lon != 0:
                        domain += data[ini + 1:ini + lon + 1].decode('utf-8') + '.'
                        ini += lon + 1
                        lon = data[ini]
                    packet = data[:2] + b'\x81\x80'
                    packet += data[4:6] + data[4:6] + b'\x00\x00\x00\x00'  # Questions and Answers Counts
                    packet += data[12:]  # Original Domain Name Question
                    packet += b'\xC0\x0C'  # Pointer to domain name
                    packet += b'\x00\x01\x00\x01\x00\x00\x00\x3C\x00\x04'  # Response type, ttl and resource data length -> 4 bytes
                    packet += bytes(map(int, self._ipaddr.split('.')))  # 4bytes of IP
                    self._dns_socket.sendto(packet, addr)
                else:
                    print('[DNS] Ignored request 2')
            except:
                pass