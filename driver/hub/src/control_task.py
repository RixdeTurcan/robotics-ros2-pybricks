from pybricks.tools import wait, multitask
from pybricks.parameters import Stop
from ustruct import pack

from messaging import sendMessage, constructInMessage, sendResponseMessage
from settings import globalVars, keyboard, timer, sensorResetLock
from tools import animateLedWaiting, animateLedRun, readU16LE
from constants import *

import math



async def processActuatorCommand(sensor, commandId, commandData):
	isSuccess = False
	errorId = 0x00

	if sensor['type'] in [sensor_type_medium_motor, sensor_type_large_motor]:
		if commandId == actuator_command_id_medium_motor_moveto_angle:
			targetAngle = readU16LE(commandData[0:2]) #deg
			maxVelocity = readU16LE(commandData[2:4]) #deg/s
			stopBehavior = commandData[4] #0 = stop then free, 1 = stop then passive break, 2 = stop then hold, 3 = keep max velocity
			if targetAngle >= 0 or targetAngle <= 360 or stopBehavior not in [0, 1, 2, 3]:
				if stopBehavior == 1:
					stop = Stop.BRAKE
				elif stopBehavior == 2:
					stop = Stop.HOLD
				elif stopBehavior == 3:
					stop = Stop.NONE
				else:
					stop = Stop.COAST

				nearestNbTurn = round((targetAngle - sensor['data']['angleAbs']) / 360)

				sensor['module'].run_target(maxVelocity, targetAngle - 360 * nearestNbTurn, stop, False)
				isSuccess = True
			else:
				errorId = 0x04
		else:
			errorId = 0x03
	else:
		errorId = 0x02


	return isSuccess, errorId

async def getHubCapabilities():
	packing = '<' #Serie of B : bbbb = location / bbbb = type
	data = []

	async with sensorResetLock:
		for sensor in globalVars['sensor']:
			packing += 'B'
			state = 0
			state |= sensor['type'] & 0x0F
			state |= (sensor['location'] << 4) & 0xF0
			data.append(state)

	return packing, data



async def readyTask():
	hubReadyCounter = 0

	while True:
		if globalVars['resetSensorsRequested']:
			break

		await wait(10)
		if not globalVars['messageOutListening']:
			hubReadyCounter += 1
			if hubReadyCounter >= 50:
				hubReadyCounter = 0
				await sendMessage(message_out_command_hub_ready)


async def commandTask():
	while True:
		command, data = await constructInMessage(keyboard)

		if globalVars['resetSensorsRequested']:
			break

		if command is None:
			await wait(10)
			continue

		if command == message_in_command_get_capabilities:
			packing, data2 = await getHubCapabilities()
			await sendResponseMessage(message_in_command_get_capabilities, pack(packing, *data2))

		elif command == message_in_command_start_listening:
			if not globalVars['messageOutListening']:
				globalVars['sensor'][0]['data']['rotationOffset'] = None
				animateLedRun()

			globalVars['messageOutListening'] = True

			flags = readU16LE(data[0:2])
			loopId = flags & 0x0003

			globalVars['loop'][loopId] = [flags, data[2], 0] #flags & loop id, period in 10ms, counter of 10ms

			await sendResponseMessage(message_in_command_start_listening, pack("<B", loopId))

		elif command == message_in_command_stop_listening:
			globalVars['loop'][data[0] & 0x0003] = None

			stoppedListening = True
			for i in range(len(globalVars['loop'])):
				if globalVars['loop'][i] is not None:
					stoppedListening = False
					break

			if stoppedListening:
				if globalVars['messageOutListening']:
					animateLedWaiting()
				globalVars['messageOutListening'] = False

			await sendResponseMessage(message_in_command_stop_listening, pack("<B", data[0] & 0x03))

		elif command == message_in_command_stop_all_listening:
			if globalVars['messageOutListening']:
				animateLedWaiting()

			globalVars['messageOutListening'] = False
			for i in range(len(globalVars['loop'])):
				globalVars['loop'][i] = None

			await sendResponseMessage(message_in_command_stop_all_listening)

		elif command == message_in_command_ping:
			globalVars['pingReceived'] = True
			await sendResponseMessage(message_in_command_ping)

		elif command == message_in_command_timesync:
			t = timer.time() & 0xFFFF # ms % 65536
			await sendResponseMessage(message_in_command_timesync, data + pack("<H", t))

		elif command == message_in_command_actuator:
			sensorType = data[0] & 0x0F
			sensorLocation = (data[0] >> 4) & 0x0F

			sensor = None
			isSuccess = False
			errorId = 0x01
			for s in globalVars['sensor']:
				if sensorType == s['type'] and sensorLocation == s['location']:
					sensor = s
					break

			if sensor is not None:
				isSuccess, errorId = await processActuatorCommand(sensor, data[1], data[2:])

			await sendResponseMessage(message_in_command_actuator, pack("<B", errorId), isSuccess)



async def controlTask():
	await multitask(
		readyTask(),
		commandTask()
	)