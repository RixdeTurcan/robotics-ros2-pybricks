from pybricks.tools import wait

from ustruct import pack
from usys import stdin

from constants import message_boundary_start, message_boundary_escape, message_boundary_escape_start, message_out_command_response
from settings import timer, keyboard, mtu, writeLock, globalVars
from tools import crc8


async def sendResponseMessage(command: int, data: bytes = b'', success: bool = True) -> None:
	statusByte = 0x00 if success else 0x01
	messageData = pack("<BB", command, statusByte) + data

	await sendMessage(message_out_command_response, messageData)


async def sendMessage(command: int, data: bytes = b'') -> None:
	data2 = bytearray()
	for b in data:
		if b == message_boundary_start:
			data2.append(message_boundary_escape)
			data2.append(message_boundary_escape_start)
		elif b == message_boundary_escape:
			data2.append(message_boundary_escape)
			data2.append(message_boundary_escape)
		else:
			data2.append(b)

	frame = bytearray()
	frame.append(message_boundary_start)
	frame.append(command)

	l = len(data2)
	if l > 0xFE:
		raise ValueError("Data too long to send in a single message")

	frame.append(l)

	if l > 0:
		frame.extend(data2)

	crc = crc8(frame)
	if crc == message_boundary_start:
		frame.append(message_boundary_escape)
		frame.append(message_boundary_escape_start)

	elif crc == message_boundary_escape:
		frame.append(message_boundary_escape)
		frame.append(message_boundary_escape)

	else:
		frame.append(crc)

	async with writeLock:
		for i in range(0, len(frame), mtu):
			await globalVars['appData'].write_bytes(bytes(frame[i:i+mtu]))


def decodeMessage(frame: bytearray) -> tuple[int, bytes] | tuple[None, None]:
	l = len(frame)
	if l < 4:
		return None, None  # Frame too short to be valid

	if frame[0] != message_boundary_start:
		return None, None  # Invalid frame boundaries

	command = frame[1]
	messageSize = frame[2]

	if messageSize > 0xFE:
		return None, None  # Invalid message size

	if l == 4 + messageSize:
		if frame[3 + messageSize] == message_boundary_escape:
			return None, None  # Escape sequence in crc, but no extra byte

		crc = frame[3 + messageSize]

	elif l == 5 + messageSize:
		if frame[3 + messageSize] != message_boundary_escape:
			return None, None  # Expected escape sequence in crc, but not found

		if frame[4 + messageSize] != message_boundary_escape_start and frame[4 + messageSize] != message_boundary_escape:
			return None, None  # Invalid escape sequence in crc

		if frame[4 + messageSize] == message_boundary_escape_start:
			crc = message_boundary_start
		else:
			crc = message_boundary_escape

	else:
		return None, None  # Frame size does not match expected size

	data2 = bytearray()
	i = 3
	while i < 3 + messageSize:
		b = frame[i]
		if b == message_boundary_escape:
			i += 1
			if i >= 3 + messageSize:
				return None, None  # Escape sequence at end of data, invalid

			nextByte = frame[i]
			if nextByte == message_boundary_escape_start:
				data2.append(message_boundary_start)
			elif nextByte == message_boundary_escape:
				data2.append(message_boundary_escape)
			else:
				return None, None  # Invalid escape sequence in data
		else:
			data2.append(b)

		i += 1

	computedCRC = crc8(frame[:3 + messageSize])  # From boundary to end of data

	if crc != computedCRC:
		return None, None  # Checksum mismatch

	return command, data2


async def constructInMessage(timeout: int = 1, timeoutIfEmpty: bool = False) -> tuple[int, bytes] | tuple[None, None]:
	buffer = bytearray()

	t0 = timer.time()

	while True:
		while not keyboard.poll(0):
			if globalVars['resetSensorsRequested']:
				break

			await wait(10)
			if (len(buffer) > 0 or timeoutIfEmpty) and timer.time() - t0 >= timeout:
				return None, None  # Timeout reached, no message received

		if globalVars['resetSensorsRequested']:
			break

		t0 = timer.time()  # Reset timeout on new data

		if len(buffer) == 0: #Reading the start of a new message
			data = stdin.buffer.read(1)
			if len(data) == 0: #No data yet
				continue

			if data[0] != message_boundary_start: #It's not the start of a message
				continue

			buffer.extend(data)

		if len(buffer) < 3: #Reading command and message size
			remainingSize = 3 - len(buffer)
			data = stdin.buffer.read(remainingSize)

			if len(data) == 0: #No data yet
				continue

			buffer.extend(data)
			if len(buffer) < 3: #Not enough data yet
				continue

		messageSize = buffer[2]

		if messageSize > 0xFE:
			return None, None  # Invalid message size

		remainingSize = messageSize + 4 - len(buffer)

		if remainingSize > 0:
			data = stdin.buffer.read(remainingSize)

			if len(data) == 0: #No data yet
				continue

			buffer.extend(data)

		if len(buffer) < messageSize + 4: #Not enough data yet
			continue

		if buffer[messageSize + 3] == message_boundary_escape: #Escape sequence in checksum, need to read one more byte
			data = stdin.buffer.read(1)

			if len(data) == 0: #No data yet
				continue

			buffer.extend(data)

			if buffer[-1] != message_boundary_escape_start and buffer[-1] != message_boundary_escape:
				return None, None  # Invalid escape sequence

		return decodeMessage(buffer)
	return None, None