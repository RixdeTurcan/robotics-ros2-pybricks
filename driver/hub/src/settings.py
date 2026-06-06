from pybricks.hubs import PrimeHub
from pybricks.tools import StopWatch, wait

from uselect import poll, POLLIN
from usys import stdin


class AsyncLock:
	def __init__(self):
		self.locked = False

	async def __aenter__(self):
		while self.locked:
			await wait(2)
		self.locked = True

	async def __aexit__(self, exc_type, exc_val, exc_tb):
		self.locked = False

	def forceUnlock(self):
		self.locked = False


hub = PrimeHub()

timer = StopWatch()

writeLock = AsyncLock()
sensorResetLock = AsyncLock()

mtu = 19

globalVars = {
	'loop': [None, None, None, None],
	'minSamplingPeriod': 10,
	'pingReceived': False,
	'messageOutListening': False,
	'resetSensorsRequested': True,
	'ledAnimationStatus': -1,
	'sensor': [],
	'appData': None
}

keyboard = poll()
keyboard.register(stdin, POLLIN)
