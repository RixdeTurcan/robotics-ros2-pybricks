from micropython import kbd_intr
from pybricks.tools import multitask, run_task

import sys
from uerrno import ENODEV, EAGAIN

from data_fetcher_task import fetchDataTask, motorFetcher, buttonsFetcher, ultrasonicSensorFetcher, colorSensorFetcher, forceSensorFetcher, systemFetcher, imuFetcher
from control_task import controlTask

from settings import globalVars
from tools import animateLedWaiting, animateLedPrepare, animateLedRun, prepareSensors, resetSensors, initHub, initSensors, releaseHub


async def mainLoop() -> None:
	await initSensors()
	await multitask(
		controlTask(),
		fetchDataTask(),
		motorFetcher(),
		buttonsFetcher(),
		ultrasonicSensorFetcher(),
		colorSensorFetcher(),
		forceSensorFetcher(),
		systemFetcher(),
		imuFetcher()
	)


def main() -> None:
	kbd_intr(-1)  # Disable REPL keyboard interrupt to prevent conflicts with message receiving

	print('Run program')

	globalVars['resetSensorsRequested'] = False

	try:
		animateLedPrepare()
		initHub()
		if resetSensors():
			return #Stop program requested
		prepareSensors()

		if globalVars['messageOutListening']:
			animateLedRun()
		else:
			animateLedWaiting()

		print('Run main loop')
		try:
			run_task(mainLoop())

		except OSError as e:
			if e.errno == ENODEV:
				print('Sensor error - ENODEV - Restarting')

			if e.errno == EAGAIN:
				print('Sensor error - EAGAIN - Restarting')

			else:
				print(sys.print_exception(e))

		except AttributeError as e:
			print(sys.print_exception(e))

		print('Stop main loop')


	except SystemExit as e:
		print('Exit main loop')

	except Exception as e:
		print(sys.print_exception(e))

	releaseHub()
	print('Stop program')


main()

print('Shutdown program')
