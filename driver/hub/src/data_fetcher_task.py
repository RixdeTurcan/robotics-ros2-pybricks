from pybricks.tools import multitask, wait
from pybricks.parameters import Axis, Button

from ustruct import pack

from constants import deg_to_mrad, deg_to_2bytes, message_out_command_data
from constants import flag_hub_imu, flag_hub_buttons, flag_hub_system, flag_hub_speaker, flag_hub_light_55_matrix, flag_sensors
from constants import sensor_data_size_hub_imu, sensor_data_size_force_sensor, sensor_data_size_color_sensor, sensor_data_size_ultrasonic_sensor
from constants import sensor_data_size_light_33_matrix, sensor_data_size_medium_motor, sensor_data_size_large_motor, sensor_data_size_hub_buttons
from constants import sensor_data_size_hub_system, sensor_data_size_hub_speaker, sensor_data_size_hub_light_55_matrix
from constants import sensor_type_force_sensor, sensor_type_color_sensor, sensor_type_ultrasonic_sensor
from constants import sensor_type_medium_motor, sensor_type_large_motor, sensor_type_light_33_matrix

from messaging import sendMessage
from settings import timer, globalVars, sensorResetLock
from tools import clampToInt16, clampToUInt8, clampToUInt16


async def fillMotorData(sensor):
	state = sensor['module'].model.state()
	angle = state[0]
	speed = state[1]
	torque = state[2]
	stalled = state[3]
	async with sensor['writeLock']:
		sensor['data']['angleAbs'] = state[0]
		sensor['data']['angle'] = clampToUInt16((angle % 360)*100)
		sensor['data']['speed'] = clampToInt16(speed*100)
		sensor['data']['torque'] = clampToInt16(torque*100)
		sensor['data']['stalled'] = 0x01 if stalled else 0x00

async def motorFetcher():
	while True:
		if globalVars['resetSensorsRequested']:
			break

		fetchers = []
		for i in range(6):
			sensor = globalVars['sensor'][-6+i]
			if sensor['type'] in [sensor_type_medium_motor, sensor_type_large_motor]:
				fetchers.append(fillMotorData(sensor))

		if len(fetchers) > 0:
			await multitask(*fetchers)

		if globalVars['resetSensorsRequested']:
			break

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))

async def fillUltrasonicSensorDistanceData(sensor):
	distance = await (sensor['module'].distance())
	async with sensor['writeLock']:
		if distance >= 2000:
			distance = 0xFFFF
		sensor['data']['distance'] = clampToUInt16(distance)

async def fillUltrasonicSensorPresenceData(sensor):
	pass
	#async with sensor['writeLock']:
	#	presence = await (sensor['module'].presence())
	#	sensor['data']['presence'] = 0x01 if presence else 0x00



async def ultrasonicSensorFetcher():
	while True:
		if globalVars['resetSensorsRequested']:
			break

		fetchers = []
		for i in range(6):
			sensor = globalVars['sensor'][-6+i]
			if sensor['type'] == sensor_type_ultrasonic_sensor:
				fetchers.append(fillUltrasonicSensorDistanceData(sensor))
				fetchers.append(fillUltrasonicSensorPresenceData(sensor))

		if len(fetchers) > 0:
			await multitask(*fetchers)

		if globalVars['resetSensorsRequested']:
			break

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))


async def fillColorSensorColorData(sensor):
	color = await (sensor['module'].hsv(True))
	async with sensor['writeLock']:
		sensor['data']['colorH'] = clampToUInt16(color.h)
		sensor['data']['colorS'] = clampToUInt8(color.s)
		sensor['data']['colorV'] = clampToUInt8(color.v)

async def fillColorSensorReflectionData(sensor):
	reflection = await (sensor['module'].reflection())
	async with sensor['writeLock']:
		sensor['data']['reflection'] = clampToUInt8(reflection)

async def fillColorSensorAmbientData(sensor):
	pass
	#async with sensor['writeLock']:
	#	ambient = await (sensor['module'].ambient())
	#	sensor['data']['ambient'] = clampToUInt8(ambient)

async def colorSensorFetcher():
	while True:
		if globalVars['resetSensorsRequested']:
			break

		fetchers = []
		for i in range(6):
			sensor = globalVars['sensor'][-6+i]
			if sensor['type'] == sensor_type_color_sensor:
				fetchers.append(fillColorSensorColorData(sensor))
				fetchers.append(fillColorSensorReflectionData(sensor))
				fetchers.append(fillColorSensorAmbientData(sensor))

		if len(fetchers) > 0:
			await multitask(*fetchers)

		if globalVars['resetSensorsRequested']:
			break

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))

async def fillForceSensorForceData(sensor):
	force = await (sensor['module'].force())
	async with sensor['writeLock']:
		sensor['data']['force'] = clampToUInt16((force/11.) * 0xFFFF)

async def fillForceSensorDistanceData(sensor):
	distance = await (sensor['module'].distance())
	async with sensor['writeLock']:
		sensor['data']['distance'] = clampToUInt8((distance/8.) * 0xFF)

async def fillForceSensorPressedData(sensor):
	pressed = await (sensor['module'].pressed(3))
	async with sensor['writeLock']:
		sensor['data']['pressed'] = 0x01 if pressed else 0x00

async def fillForceSensorTouchedData(sensor):
	touched = await (sensor['module'].touched())
	async with sensor['writeLock']:
		sensor['data']['touched'] = 0x01 if touched else 0x00

async def forceSensorFetcher():
	while True:
		if globalVars['resetSensorsRequested']:
			break

		fetchers = []
		for i in range(6):
			sensor = globalVars['sensor'][-6+i]
			if sensor['type'] == sensor_type_force_sensor:
				fetchers.append(fillForceSensorForceData(sensor))
				fetchers.append(fillForceSensorDistanceData(sensor))
				fetchers.append(fillForceSensorPressedData(sensor))
				fetchers.append(fillForceSensorTouchedData(sensor))

		if len(fetchers) > 0:
			await multitask(*fetchers)

		if globalVars['resetSensorsRequested']:
			break

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))


async def buttonsFetcher():
	period = 10 * globalVars['minSamplingPeriod']
	sensor = globalVars['sensor'][1]

	while True:
		if globalVars['resetSensorsRequested']:
			break

		pressed = sensor['module'].pressed()

		async with sensor['writeLock']:
			sensor['data']['center'] = 0x00
			sensor['data']['left'] = 0x00
			sensor['data']['right'] = 0x00
			sensor['data']['bluetooth'] = 0x00

			for button in pressed:
				if button == Button.CENTER:
					sensor['data']['center'] = 0x01
					globalVars['resetSensorsRequested'] = True
				elif button == Button.LEFT:
					sensor['data']['left'] = 0x01
				elif button == Button.RIGHT:
					sensor['data']['right'] = 0x01
				elif button == Button.BLUETOOTH:
					sensor['data']['bluetooth'] = 0x01

		t = timer.time()
		await wait(period - (t % period))

async def systemFetcher():
	period = 10 * globalVars['minSamplingPeriod']
	sensor = globalVars['sensor'][2]

	while True:
		if globalVars['resetSensorsRequested']:
			break

		voltage = sensor['module'].voltage()
		current = sensor['module'].current()

		async with sensor['writeLock']:
			capacity = clampToUInt8((voltage - 6500) * 255 / (8200 - 6500))
			sensor['data']['capacity'] = capacity

			current = clampToUInt8(current * 255 / 1000)
			sensor['data']['current'] = current

		t = timer.time()
		await wait(period - (t % period))

async def imuFetcher():
	sensor = globalVars['sensor'][0]

	while True:
		if globalVars['resetSensorsRequested']:
			break

		ax = sensor['module'].acceleration(Axis.X, calibrated=True) # mm/s²
		ay = sensor['module'].acceleration(Axis.Y, calibrated=True) # mm/s²
		az = sensor['module'].acceleration(Axis.Z, calibrated=True) # mm/s²

		wx = sensor['module'].angular_velocity(Axis.X, calibrated=True) # deg/s
		wy = sensor['module'].angular_velocity(Axis.Y, calibrated=True) # deg/s
		wz = sensor['module'].angular_velocity(Axis.Z, calibrated=True) # deg/s

		ry, rx = sensor['module'].tilt(calibrated=True)
		rz = -sensor['module'].heading()

		async with sensor['writeLock']:
			ax = clampToInt16(ax)
			ay = clampToInt16(ay)
			az = clampToInt16(az)
			sensor['data']['acceleration'] = [ax, ay, az]

			wx_krad = clampToInt16(wx * deg_to_mrad)
			wy_krad = clampToInt16(wy * deg_to_mrad)
			wz_krad = clampToInt16(wz * deg_to_mrad)
			sensor['data']['angularVelocity'] = [wx_krad, wy_krad, wz_krad]

			if sensor['data']['rotationOffset'] is None:
				sensor['data']['rotationOffset'] = [rx, ry, rz]

			rx -= sensor['data']['rotationOffset'][0]
			ry -= sensor['data']['rotationOffset'][1]
			rz -= sensor['data']['rotationOffset'][2]

			rx = round(rx * deg_to_2bytes) & 0xFFFF
			ry = round(ry * deg_to_2bytes) & 0xFFFF
			rz = round(rz * deg_to_2bytes) & 0xFFFF

			sensor['data']['rotation'] = [rx, ry, rz]

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))


async def sendData(flags):
	packing = '<HHBBB' # Timestamp (ms) + dataFlag + (F/E, D/C, B/A sensor types)
	args = []
	sensorIds = []

	if flags & flag_hub_imu:
		packing += sensor_data_size_hub_imu
		sensor = globalVars['sensor'][0]
		args.extend(sensor['data']['acceleration'])
		args.extend(sensor['data']['angularVelocity'])
		args.extend(sensor['data']['rotation'])

	if flags & flag_hub_buttons:
		packing += sensor_data_size_hub_buttons
		sensor = globalVars['sensor'][1]
		buttons_state = 0
		buttons_state |= sensor['data']['center'] << 0
		buttons_state |= sensor['data']['left'] << 1
		buttons_state |= sensor['data']['right'] << 2
		buttons_state |= sensor['data']['bluetooth'] << 3
		args.append(buttons_state)

	if flags & flag_hub_system:
		packing += sensor_data_size_hub_system
		sensor = globalVars['sensor'][2]
		args.append(sensor['data']['capacity'])
		args.append(sensor['data']['current'])

	if flags & flag_hub_speaker:
		packing += sensor_data_size_hub_speaker

	if flags & flag_hub_light_55_matrix:
		packing += sensor_data_size_hub_light_55_matrix

	for i in range(6):
		sensor = globalVars['sensor'][-6+i]
		sensorIds.append(sensor['type'])
		if flags & flag_sensors[i]:
			if sensor['type'] == sensor_type_force_sensor:
				packing += sensor_data_size_force_sensor
				args.append(sensor['data']['force'])
				args.append(sensor['data']['distance'])
				state = 0
				state |= sensor['data']['touched'] << 0
				state |= sensor['data']['pressed'] << 1
				args.append(state)

			elif sensor['type'] == sensor_type_color_sensor:
				packing += sensor_data_size_color_sensor
				args.append(sensor['data']['colorH'])
				args.append(sensor['data']['colorS'])
				args.append(sensor['data']['colorV'])
				args.append(sensor['data']['reflection'])
				args.append(sensor['data']['ambient'])

			elif sensor['type'] == sensor_type_ultrasonic_sensor:
				packing += sensor_data_size_ultrasonic_sensor
				args.append(sensor['data']['distance'])

			elif sensor['type'] == sensor_type_light_33_matrix:
				packing += sensor_data_size_light_33_matrix

			elif sensor['type'] == sensor_type_medium_motor:
				packing += sensor_data_size_medium_motor
				args.append(sensor['data']['angle'])
				args.append(sensor['data']['speed'])
				args.append(sensor['data']['torque'])

			elif sensor['type'] == sensor_type_large_motor:
				packing += sensor_data_size_large_motor
				args.append(sensor['data']['angle'])
				args.append(sensor['data']['speed'])
				args.append(sensor['data']['torque'])

	t = timer.time()
	ts = t & 0xFFFF  # ms % 65536

	stateAB = 0
	for i in range(2):
		stateAB |= sensorIds[i] << (i*4)

	stateCD = 0
	for i in range(2):
		stateCD |= sensorIds[i+2] << (i*4)

	stateEF = 0
	for i in range(2):
		stateEF |= sensorIds[i+4] << (i*4)

	frame = pack(packing, ts, flags, stateEF, stateCD, stateAB, *args)
	await sendMessage(message_out_command_data, frame)


async def fetchDataTask():
	while True:
		if globalVars['resetSensorsRequested']:
			break

		for loop in globalVars['loop']: # [flags, period count, current count]
			if loop is not None:
				loop[2] += 1
				if loop[2] >= loop[1]:
					loop[2] = 0
					flags = loop[0]
					if (flags & 0xFFFC) != 0x0000:
						async with sensorResetLock:
							await sendData(flags)

		t = timer.time()
		await wait(globalVars['minSamplingPeriod'] - (t % globalVars['minSamplingPeriod']))