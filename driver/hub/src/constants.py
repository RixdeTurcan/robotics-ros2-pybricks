from pybricks.parameters import Port

import math


ports = [Port.A, Port.B, Port.C, Port.D, Port.E, Port.F]

deg_to_rad: float = math.pi / 180
rad_to_deg: float = 180 / math.pi
rad_to_mrad: float = 1000
mm_to_m: float = 0.001
m_to_mm: float = 1000
deg_to_mrad: float = deg_to_rad * rad_to_mrad
deg_to_2bytes: float = 65536 / 360
rad_to_2bytes: float = 65536 / (2*math.pi)

# Structure of a message frame :
# message_boundary (1B) - message_[in/out]_command (1B) - message_size (1B -> max 255B of data) -  message_data (message_size B) - checksum (1B) - message_boundary (1B)
# checksum is the XOR of all bytes of [message_[in/out]_command + message_size + message_data]

message_boundary_start: int = 0xFF
message_boundary_escape: int = 0xFE
message_boundary_escape_start: int = 0xFD

message_out_command_response: int = 0x00
message_out_command_data: int = 0x01
message_out_command_hub_ready: int = 0x02

message_in_command_ping: int = 0x00
message_in_command_start_listening: int = 0x01
message_in_command_stop_listening: int = 0x02
message_in_command_stop_all_listening: int = 0x03
message_in_command_get_capabilities: int = 0x04
message_in_command_timesync: int = 0x05
message_in_command_actuator: int = 0x06

sensor_type_none: int = 0x0
sensor_type_hub_imu: int = 0x1
sensor_type_force_sensor: int = 0x2
sensor_type_color_sensor: int = 0x3
sensor_type_ultrasonic_sensor: int = 0x4
sensor_type_light_33_matrix: int = 0x5
sensor_type_medium_motor: int = 0x6
sensor_type_large_motor: int = 0x7
sensor_type_hub_buttons: int = 0x8
sensor_type_hub_system: int = 0x9
sensor_type_hub_speaker: int = 0xA
sensor_type_hub_light_55_matrix: int = 0xB

sensor_location_hub : int = 0x0
sensor_location_A : int = 0x1
sensor_location_B : int = 0x2
sensor_location_C : int = 0x3
sensor_location_D : int = 0x4
sensor_location_E : int = 0x5
sensor_location_F : int = 0x6
sensor_locations: list[int] = [sensor_location_A, sensor_location_B, sensor_location_C, sensor_location_D, sensor_location_E, sensor_location_F]

flag_hub_imu: int = 0x0004
flag_hub_buttons: int = 0x0008
flag_hub_system: int = 0x0010
flag_hub_speaker: int = 0x0020
flag_hub_light_55_matrix: int = 0x0040
flag_sensor_A: int = 0x0080
flag_sensor_B: int = 0x0100
flag_sensor_C: int = 0x0200
flag_sensor_D: int = 0x0400
flag_sensor_E: int = 0x0800
flag_sensor_F: int = 0x1000
flag_sensors: list[int] = [flag_sensor_A, flag_sensor_B, flag_sensor_C, flag_sensor_D, flag_sensor_E, flag_sensor_F]

sensor_data_size_hub_imu: str = 'hhhhhhHHH' #Acc xyz (signed mm/s²), gyr rpy (signed mrad/s), rot rpy (H = 360°)
sensor_data_size_force_sensor: str = 'HBB' #Force (H = 10N), distance (B=8mm), */*/*/*/*/*/pressed/touched
sensor_data_size_color_sensor: str = 'HBBBB' #Color h(360°), color s(%), color v(%), Reflection (%), Ambient (%)
sensor_data_size_ultrasonic_sensor: str = 'H' #Distance (mm) [max int = empty space]
sensor_data_size_light_33_matrix: str = ''
sensor_data_size_medium_motor: str = 'Hhh' #Rot (deg), vel (signed deg/s), torque (signed mNm)
sensor_data_size_large_motor: str = 'Hhh' #Rot (deg), vel (signed deg/s), torque (signed mNm)
sensor_data_size_hub_buttons: str = 'B' # */*/*/*/bluetooth/right/left/center
sensor_data_size_hub_system: str = 'BB' # capacity (B = 6.5->8.2V), current (B = 1A)
sensor_data_size_hub_speaker: str = ''
sensor_data_size_hub_light_55_matrix: str = ''

device_id_force_sensor: int = 63
device_id_color_sensor: int = 61
device_id_ultrasonic_sensor: int = 62
device_id_light_33_matrix: int = 64
device_id_medium_motor: int = 48
device_id_large_motor: int = 49

actuator_command_id_medium_motor_moveto_angle: int = 0x00


g = 9.80665 # m/s²