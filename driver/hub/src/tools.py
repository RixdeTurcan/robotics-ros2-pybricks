from pybricks.parameters import Color, Button, Stop
from pybricks.iodevices import PUPDevice
from pybricks.tools import AppData, wait
from pybricks.pupdevices import ColorSensor, ForceSensor, Motor, ColorLightMatrix, UltrasonicSensor
from ustruct import unpack
from settings import globalVars, hub, AsyncLock, mtu
from constants import *


CRC8Table = bytearray([
	0x00, 0x97, 0xb9, 0x2e, 0xe5, 0x72, 0x5c, 0xcb,
	0x5d, 0xca, 0xe4, 0x73, 0xb8, 0x2f, 0x01, 0x96,
	0xba, 0x2d, 0x03, 0x94, 0x5f, 0xc8, 0xe6, 0x71,
	0xe7, 0x70, 0x5e, 0xc9, 0x02, 0x95, 0xbb, 0x2c,
	0xe3, 0x74, 0x5a, 0xcd, 0x06, 0x91, 0xbf, 0x28,
	0xbe, 0x29, 0x07, 0x90, 0x5b, 0xcc, 0xe2, 0x75,
	0x59, 0xce, 0xe0, 0x77, 0xbc, 0x2b, 0x05, 0x92,
	0x04, 0x93, 0xbd, 0x2a, 0xe1, 0x76, 0x58, 0xcf,
	0x51, 0xc6, 0xe8, 0x7f, 0xb4, 0x23, 0x0d, 0x9a,
	0x0c, 0x9b, 0xb5, 0x22, 0xe9, 0x7e, 0x50, 0xc7,
	0xeb, 0x7c, 0x52, 0xc5, 0x0e, 0x99, 0xb7, 0x20,
	0xb6, 0x21, 0x0f, 0x98, 0x53, 0xc4, 0xea, 0x7d,
	0xb2, 0x25, 0x0b, 0x9c, 0x57, 0xc0, 0xee, 0x79,
	0xef, 0x78, 0x56, 0xc1, 0x0a, 0x9d, 0xb3, 0x24,
	0x08, 0x9f, 0xb1, 0x26, 0xed, 0x7a, 0x54, 0xc3,
	0x55, 0xc2, 0xec, 0x7b, 0xb0, 0x27, 0x09, 0x9e,
	0xa2, 0x35, 0x1b, 0x8c, 0x47, 0xd0, 0xfe, 0x69,
	0xff, 0x68, 0x46, 0xd1, 0x1a, 0x8d, 0xa3, 0x34,
	0x18, 0x8f, 0xa1, 0x36, 0xfd, 0x6a, 0x44, 0xd3,
	0x45, 0xd2, 0xfc, 0x6b, 0xa0, 0x37, 0x19, 0x8e,
	0x41, 0xd6, 0xf8, 0x6f, 0xa4, 0x33, 0x1d, 0x8a,
	0x1c, 0x8b, 0xa5, 0x32, 0xf9, 0x6e, 0x40, 0xd7,
	0xfb, 0x6c, 0x42, 0xd5, 0x1e, 0x89, 0xa7, 0x30,
	0xa6, 0x31, 0x1f, 0x88, 0x43, 0xd4, 0xfa, 0x6d,
	0xf3, 0x64, 0x4a, 0xdd, 0x16, 0x81, 0xaf, 0x38,
	0xae, 0x39, 0x17, 0x80, 0x4b, 0xdc, 0xf2, 0x65,
	0x49, 0xde, 0xf0, 0x67, 0xac, 0x3b, 0x15, 0x82,
	0x14, 0x83, 0xad, 0x3a, 0xf1, 0x66, 0x48, 0xdf,
	0x10, 0x87, 0xa9, 0x3e, 0xf5, 0x62, 0x4c, 0xdb,
	0x4d, 0xda, 0xf4, 0x63, 0xa8, 0x3f, 0x11, 0x86,
	0xaa, 0x3d, 0x13, 0x84, 0x4f, 0xd8, 0xf6, 0x61,
	0xf7, 0x60, 0x4e, 0xd9, 0x12, 0x85, 0xab, 0x3c ])

CRC8POLY = 0x97

def buildCRC8Table(poly):
	global CRC8POLY
	global CRC8Table

	CRC8POLY = poly

	for i in range (0,256):
		c = i
		for j in range (0,8):
			c = c<<1 if ((c & 0x80) == 0) else (c<<1) ^ poly
			c &= 0xff
		CRC8Table[i] = c

	return

def getCRC8Poly():
	return CRC8POLY

def clampToUInt8(value):
	if value < 0:
		return 0
	elif value > 255:
		return 255
	else:
		return int(value)

def clampToInt16(value):
	if value < -32767:
		return -32767
	elif value > 32767:
		return 32767
	else:
		return int(value)

def clampToUInt16(value):
	if value < 0:
		return 0
	elif value > 65535:
		return 65535
	else:
		return int(value)

def crc8(msg, init=0x00):
	rem = init
	l = len(msg)
	for i in range(l):
		rem = CRC8Table[(rem ^ msg[i])] & 0xff
	return rem

def animateLedWaiting():
	if globalVars['ledAnimationStatus'] != 0:
		globalVars['ledAnimationStatus'] = 0
		hub.light.animate(
			[Color(180, 100, i) for i in range(100, 30, -1)] +
			[Color(180, 100, i) for i in range(30, 100, 1)],
			30
		)

def animateLedPrepare():
	if globalVars['ledAnimationStatus'] != 1:
		globalVars['ledAnimationStatus'] = 1
		hub.light.animate(
			[Color(260, 100, i) for i in range(100, 30, -1)] +
			[Color(260, 100, i) for i in range(30, 100, 1)],
			30
		)

def animateLedRun():
	if globalVars['ledAnimationStatus'] != 2:
		globalVars['ledAnimationStatus'] = 2
		hub.light.animate(
			[Color(120, 100, i) for i in range(100, 30, -1)] +
			[Color(120, 100, i) for i in range(30, 100, 1)],
			30
		)

def readU16LE(data: bytes) -> int:
	return unpack('<H', data)[0]


def releaseHub():
	hub.light.off()
	if globalVars['appData'] is not None:
		globalVars['appData'].close()


def initHub():
	globalVars['appData'] = AppData([(0,mtu)])
	globalVars['sensor'].append({
		'type': sensor_type_hub_imu,
		'location': sensor_location_hub,
		'data': {
			'acceleration': [0, 0, 0], # mm/s²
			'angularVelocity': [0, 0, 0], # mrad/s
			'rotation': [0, 0, 0], # 2b = 360°
			'rotationOffset': None # deg
		},
		'module': hub.imu,
		'writeLock': AsyncLock()
	})

	globalVars['sensor'].append({
		'type': sensor_type_hub_buttons,
		'location': sensor_location_hub,
		'data': {
			'center': 0x00, # 0 = off / 1 = on
			'left': 0x00, # 0 = off / 1 = on
			'right': 0x00, # 0 = off / 1 = on
			'bluetooth': 0x00, # 0 = off / 1 = on
		},
		'module': hub.buttons,
		'writeLock': AsyncLock()
	})

	globalVars['sensor'].append({
		'type': sensor_type_hub_system,
		'location': sensor_location_hub,
		'data': {
			'capacity': 0, # 2b = 6.5->8.2V
			'current': 0, # 2b = 1A
		},
		'module': hub.battery,
		'writeLock': AsyncLock()
	})

	globalVars['sensor'].append({
		'type': sensor_type_hub_speaker,
		'location': sensor_location_hub,
		'data': {},
		'module': hub.speaker,
		'writeLock': AsyncLock()
	})

	globalVars['sensor'].append({
		'type': sensor_type_hub_light_55_matrix,
		'location': sensor_location_hub,
		'data': {},
		'module': hub.display,
		'writeLock': AsyncLock()
	})


	for i in range(6):
		globalVars['sensor'].append({
			'type': sensor_type_none,
			'location': sensor_locations[i],
			'data': {},
			'module': None,
			'writeLock': AsyncLock()
		})

	hub.imu.settings(
		angular_velocity_threshold=0.5,
		acceleration_threshold=100,
		angular_velocity_bias=(0.7, 1.6, 0.35),
		acceleration_correction=(10000, -9850, 9880, -9930, 9730, -9880)
	)

	hub.speaker.volume(100)
	hub.display.off()
	hub.system.set_stop_button(None)



def resetSensors():
	for i in range(len(ports)):
		sensor = globalVars['sensor'][-6+i]
		sensor['type'] = sensor_type_none
		sensor['module'] = None
		sensor['data'] = {}

		port = ports[i]
		try:
			device = PUPDevice(port)
			device.reset()
		except OSError as ex:
			pass

	print('Wait for sensors reset')
	loopSize = 6
	loopMs = 250
	for i in range(loopSize):
		wait(loopMs)
		pressed = hub.buttons.pressed()
		if Button.CENTER in pressed:
			print('Stop program requested')
			return True

	return False


def prepareSensors():
	for i in range(len(ports)):
		sensor = globalVars['sensor'][-6+i]
		port = ports[i]
		try:
			device = PUPDevice(port)
		except OSError as ex:
			if sensor['type'] != sensor_type_none:
				sensor['type'] = sensor_type_none
				sensor['module'] = None
				sensor['data'] = {}
			continue

		deviceId = device.info()['id']

		if deviceId == device_id_color_sensor:
			if sensor['type'] != sensor_type_color_sensor:
				sensor['type'] = sensor_type_color_sensor
				sensor['module'] = ColorSensor(port)
				sensor['data'] = {
					'colorH': 0, # °
					'colorS': 0, # %
					'colorV': 0, # %
					'reflection': 0, # %
					'ambient': 0 # %
				}

		elif deviceId == device_id_force_sensor:
			if sensor['type'] != sensor_type_force_sensor:
				sensor['type'] = sensor_type_force_sensor
				sensor['module'] = ForceSensor(port)
				sensor['data'] = {
					'force': 0., # N
					'distance': 0., # B=8mm
					'pressed': 0x00, # 0 = off / 1 = on
					'touched': 0x00, # 0 = off / 1 = on
				}

		elif deviceId == device_id_large_motor:
			if sensor['type'] != sensor_type_large_motor:
				sensor['type'] = sensor_type_large_motor
				sensor['module'] = Motor(port)
				sensor['data'] = {
					'angle': 0, # deg
					'angleAbs': 0, # deg (count turns)
					'speed': 0, # deg/s
					'torque': 0, # mNm
					'stalled': 0x00, # 0 = off / 1 = on
				}

		elif deviceId == device_id_medium_motor:
			if sensor['type'] != sensor_type_medium_motor:
				sensor['type'] = sensor_type_medium_motor
				sensor['module'] = Motor(port)
				sensor['data'] = {
					'angle': 0, # deg
					'angleAbs': 0, # deg (count turns)
					'speed': 0, # deg/s
					'torque': 0, # mNm
					'stalled': 0x00, # 0 = off / 1 = on
				}

		elif deviceId == device_id_light_33_matrix:
			if sensor['type'] != sensor_type_light_33_matrix:
				sensor['type'] = sensor_type_light_33_matrix
				sensor['module'] = ColorLightMatrix(port)
				sensor['data'] = {}

		elif deviceId == device_id_ultrasonic_sensor:
			if sensor['type'] != sensor_type_ultrasonic_sensor:
				sensor['type'] = sensor_type_ultrasonic_sensor
				sensor['module'] = UltrasonicSensor(port)
				sensor['data'] = {
					'distance': 0xFFFF, # mm (0xFFFF = infinity)
					'presence': 0x00, # 0 = off / 1 = on
				}

		else:
			print('Unknown device {} Id: {}'.format(i, deviceId))


async def initSensors():
	for i in range(6):
		sensor = globalVars['sensor'][-6+i]

		if sensor['type'] in [sensor_type_medium_motor, sensor_type_large_motor]:
			sensor['module'].settings(9000) #Max voltage 5000->9000mV
			sensor['module'].control.target_tolerances(10, 2) #velocity (deg/s), position (deg) tolerance in default control
			sensor['module'].reset_angle()
			sensor['module'].run_target(180, 0, Stop.COAST, False)

