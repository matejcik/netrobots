import sys
import os.path
import argparse
import socket
import re

START	= 'g'
CYCLE	= 'c'
CANNON	= 'f'
SCAN	= 's'
DRIVE	= 'd'
LOC_X	= 'x'
LOC_Y	= 'y'
DAMAGE	= 'l'
SPEED	= 'v'
ELAPSED	= 'e'
NAME	= 'n'
IMAGE	= 'i'

ERROR	= '!'
OK	= 'k'
DEAD	= 'd'

STD_RESP_LEN = 64


class RobotException(Exception):
	pass

class RobotError(RobotException):
	pass

class RobotGameOver(RobotException):
	pass


class Robot(object):
	"""Class that represents a single robot connected to robotserver.
	
	Properties:
		x	current x coordinate
		y	current y coordinate
		speed	current speed
		damage	current damage
		elapsed	elapsed cycles"""

	def __init__(self, host=None, port=None, img_path=None, init=True):
		"""__init__([host], [port], [img_path])

		Create new robot connected to given host:port (if not
		specified, command line will be parsed with fallback to
		localhost:4300). Optionally, upload given image to the
		server; img_path must point to a PNG file, file size
		restrictions may apply."""

		self.__parse_options(host, port)
		self.__connect()
		if init:
			self.__set_default_name()
			if img_path:
				self.image(img_path)
			self.start()

	def __parse_options(self, host, port):
		parser = argparse.ArgumentParser()
		parser.add_argument('-H', '--host', help='remote host', default='localhost')
		parser.add_argument('-P', '--port', type=int, help='remote port', default=4300)
		args = parser.parse_args()
		self.host = host or args.host
		self.port = int(port or args.port)
	
	def __connect(self):
		try:
			self.sk = socket.create_connection((self.host, self.port))
		except socket.error as msg:
			raise RobotError(str(msg))
	
	def __set_default_name(self):
		if hasattr(self, 'name'):
			name = self.name
		else:
			name = self.__class__.__name__
			if name == 'Robot':
				name = ''
		if not name:
			name = os.path.basename(sys.argv[0])
			if not name:
				name = 'PyRobot'
		else:
			name = re.sub('([A-Z]+)', r' \1', name).lstrip()
		self.set_name(name)
	
	def __get_reply(self):
		try:
			data = self.sk.recv(STD_RESP_LEN)
		except socket.error as msg:
			raise RobotError(str(msg))
		if not data:
			raise RobotGameOver('Server terminated')
		data = data.split(' ')
		result = data[0]
		data = data[1:]
		if result == DEAD:
			raise RobotGameOver('You are dead: %s' % ' '.join(data).strip())
		if result != OK:
			raise RobotError('Server returned error: %s' % ' '.join(data).strip())
		try:
			data = [int(d) for d in data]
		except ValueError:
			raise RobotError('Invalid response received')
		return data


	def custom_command(self, command, *args):
		"""custom_command(command, [arg...])

		Sends a custom command to the server. Use only when you know
		what you are doing."""
		
		data = "%s %s" % (command, ' '.join([str(a) for a in args]))
		try:
			self.sk.send(data)
		except socket.error as msg:
			raise RobotError(str(msg))
		return self.__get_reply()
	
	def __custom_command_check(self, values, command, *args):
		result = self.custom_command(command, *args)
		if len(result) < values:
			raise RobotError('Invalid response received')
		if values == 1:
			return result[0]
		return result

	def scan(self, degree, resolution):
		"""scan(degree, resolution)

		Scans the battlefield in the given direction with the given
		resolution. Resolution has to be <= 10. Returns distance to
		the nearest opponent."""

		return self.__custom_command_check(1, SCAN, degree, resolution)
	
	def cannon(self, degree, distance):
		"""cannon(degree, distance)

		Fires cannon in the given direction to the given distance.
		Returns False if currently reloading."""

		return bool(self.__custom_command_check(1, CANNON, degree, distance))
	
	def drive(self, degree, speed):
		"""drive(degree, speed)

		Drives with the given speed in the given direction. Note
		that changing of direction is possible only when current
		speed < 50 and decceleration does not happen immediately. If
		the currect speed >= 50, the direction is ignored."""

		self.custom_command(DRIVE, degree, speed)
	
	def cycle(self):
		"""cycle()

		Waits for the next cycle, doing nothing."""

		return self.__custom_command_check(1, CYCLE)
	
	def set_name(self, name):
		"""set_name(name)

		Sets name. Note that you don't have to set name, it is done
		automatically for you."""

		self.custom_command(NAME, name)
	
	def image(self, path):
		"""image(path)

		Sets image. Use the img_path parameter of the class
		constructor instead of this."""

		try:
			f = open(path, 'rb')
			data = f.read()
			f.close()
		except IOError as msg:
			raise RobotError(str(msg))
		self.custom_command(IMAGE, len(data))
		try:
			self.sk.send(data)
		except socket.error as msg:
			raise RobotError(str(msg))
		result = self.__get_reply()
		try:
			return bool(result[0])
		except IndexError:
			raise RobotError('Invalid response received')
	
	def start(self):
		"""start()

		Waits for the game start. You don't generally need that."""

		self.custom_command(START)
	
	def __getattr__(self, name):
		cmds = { 'x': LOC_X, 'y': LOC_Y, 'speed': SPEED, 'damage': DAMAGE,
			 'elapsed': ELAPSED }
		if not name in cmds:
			raise AttributeError()
		return self.__custom_command_check(1, cmds[name])
