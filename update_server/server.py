#-*-coding:utf-8-*-
import os
import sys
import json
import socket
import threading
import queue
import ctypes
import struct

PATCH_DIR = "patch_bin"
PATCH_CONF = "patch.json"

def log(msg, *args):
	pass

class UpdateProtocol:
	CMD_LOAD = 1
	CMD_HARTBEAT = 2

class PatchServer:

	def send_data(self):
		pass

	def check_client_alive(self, conn):
		pass

	def close_client(self, addr):
		pass

class PatchUartServer(PatchServer):
	COM = "com6" # "/dev/tty6"

	def __init__(self):
		pass

	def __start_server(self):
		pass

class PatchTCPServer(PatchServer):
	SERVER_ADDR = "192.0.2.1"
	PORT = 4242

	def __init__(self):
		# self.th = threading.
		self.conns = {}
		self.running = True
		self.server_thread = None
		self.__start_server()

	def __start_server(self):
		self.running = True
		th = threading.Thread(target=self.__wait_for_client, args=())
		self.server_thread = th
		th.setDaemon(True)
		th.start()

	def wait_for_exit(self):
		self.running = False
		self.server_thread.join()

	def __wait_for_client(self):
		with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
			s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
			s.bind(('0.0.0.0', self.PORT))
			print("Server Listen: {}:{}".format(self.SERVER_ADDR, self.PORT))
			s.listen()
			while self.running:
				print("runnin")
				conn, addr = s.accept()
				print('Connected by', addr)
				self.conns[addr] = conn
				# self.client_conn = conn
			# with conn:
			# 	print('Connected by', addr)
			# 	while True:
			# 		data = conn.recv(1024)
			# 		if not data:
			# 			break
			# 		conn.sendall(data)

	def send_data(self, cmd, buf=None):
		bys = struct.pack('B', cmd)
		if buf:
			sz = len(buf)
			print("buf len", sz, buf)
			bys += struct.pack('B', int(sz / 256))
			bys += struct.pack('B', sz % 256)
			bys += buf

		should_close = []
		for addr, client in self.conns.items():
			alive = self.check_client_alive(client)
			print("check_client_alive", addr, alive)
			if not alive:
				should_close.append(addr)
				continue
			try:
				client.sendall(bys)
				print("send {} bytes to {}".format(len(bys), addr))
			except Exception as ex:
				print(ex.args)
				should_close.append(addr)

		for addr in should_close:
			self.close_client(addr)

	def check_client_alive(self, conn):
		bys = struct.pack('B', UpdateProtocol.CMD_HARTBEAT)
		try:
			conn.sendall(bys)
			data = conn.recv(128)
			if data:
				data = struct.unpack('B', data)
				print("alive recv", data)
				return data[0] == UpdateProtocol.CMD_HARTBEAT
		except Exception as ex:
			print(ex.args)
		return False

	def close_client(self, addr):
		print("Close ", addr)
		self.conns[addr].close()
		del self.conns[addr]


class PatchPacket(ctypes.Structure):
	pack = 2
	_fields_ = [
		('patch_type', ctypes.c_ushort),
		('code_len', ctypes.c_ushort),
		('patch_point', ctypes.c_uint), # addr or patch id
		# ('code', ctypes) # ebpf bytecode
	]

	@staticmethod
	def from_conf(conf):
		pkt = PatchPacket(conf["type"], len(conf["bin"]), conf["loc"])
		return bytearray(pkt) + conf["bin"]

class Cli:

	def __init__(self):
		self.server = None
		self.server_dir = None
		self.patch_map = {}
		self.__load_patch_conf()
		self.__register_cmds()
		self.__init_server()

	def __init_server(self):
		self.server = PatchTCPServer()

	def __register_cmds(self):
		self.cmd = [
			# ('start')
		]

	def __load_patch_conf(self):
		self.server_dir = os.path.dirname(os.path.realpath(__file__))
		conf = os.path.join(self.server_dir, PATCH_CONF)
		with open(conf, "r") as fp:
			data = json.load(fp)
		for item in data:
			self.patch_map[item["alias"]] = item

	def __send_patch(self, cmd, patch_tag):
		pkt = self.__load_patch(patch_tag)
		if not pkt:
			return
		self.server.send_data(cmd, pkt)

	def __load_patch(self, patch_tag):
		# little endian
		patch = self.patch_map.get(patch_tag)
		if not patch:
			print("Not find patch: ", patch_tag)
			print("Usage: load [{}]".format(" | ".join(self.patch_map.keys())))
			return
		
		fi = os.path.join(self.server_dir, "patch_bin", patch["code"])
		if not os.path.exists(fi):
			print("Path {} not exist!".format(fi))
			return
		with open(fi, "rb") as fp:
			byte_code = fp.read()
		patch["bin"] = byte_code
		print("send patch", patch)
		data = PatchPacket.from_conf(patch)
		# pkt = PatchPacket(2, len(byte_code), 0x08005432)
		# data = bytearray(pkt) + byte_code
		print(data, len(data), "bin len:", len(byte_code))
		signature = self.__patch_sign(data)
		# print(int(data[4]), int(data[5]), int(data[6]), int(data[7])) 
		return signature + data

	def __patch_sign(self, data):
		return b's' * 16
	
	def run(self):
		tips = "load [{}]".format(" | ".join(self.patch_map.keys()))
		print("Please enter cmd: " + tips)
		while True:
			cmd = sys.stdin.readline()
			cmd = cmd.strip().split(' ')
			# print("cmd:", cmd)
			if cmd[0] == 'q':
				# self.on_exit()
				exit(0)
			elif cmd[0] == 'load':
				self.__send_patch(UpdateProtocol.CMD_LOAD, cmd[1])
			elif cmd[0] == 'alive':
				self.check_alive()

	def check_alive(self):
		should_close = []
		for addr, client in self.server.conns.items():
			if not self.server.check_client_alive(client):
				should_close.append(addr)

		for addr in should_close:
			self.server.close_client(addr)


	def on_exit(self):
		self.server.wait_for_exit()


def dump_patch_data():
	pass

def main():
	Cli().run()

if __name__ == "__main__":
	main()
