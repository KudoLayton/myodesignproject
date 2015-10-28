#-*- coding: utf-8 -*-

import serial, struct, time, bledAPI, myoAPI

#BLED112 serial Communication 설정
baud = 115200
serialTimeOut = 1
port = "/dev/ttyACM0"

myoUUID = [0x42, 0x48, 0x12, 0x4A, 0x7F, 0x2C, 0x48, 0x47,0xB9, 0xDE, 0x04, 0xA9, 0x01, 0x00, 0x06, 0xD5 ]

#Myo BLE Communication 설정
Conn_Interval_Min = 7.5
Conn_Interval_Max = 500
Conn_Latency = 100
Supervision_Timeout = 32000

#실행 시작
print "\n================================================="
print "Welcome to POSTECH Design Project Group 2 BLED112 Console for Myo"
print "Port: %s" % (port)
print "Baud Rate: %d" % (baud)
print "================================================="
print "BLED112 통신을 위한 Serial Port를 열고 있습니다."
try:
	#포트 개설 요청
	ser = serial.Serial(port=port, baudrate=baud, timeout=serialTimeOut)

except serial.SerialException as e:
	#포트 개설 에러
	print "\n================================================="
	print "Port Error"
	print "=================================================\n"
	exit(2)

ser.flushInput()
ser.flushOutput()

#기존 0번째 handle 연결 해제
#bledAPI.ble_cmd_system_hello(ser)
#result = bledAPI.ble_rsp_system_hello(ser)
#00000001: 연결이 정상적으로 해제됨
#if (result == "00000001"):
#	print "BLED112 통신 준비가 모두 완료되었습니다."
#else:
#	print "\n================================================="
#	print "BLED112 통신 준비를 실패했습니다."
#	print "코드: %s" % result
#	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
#	print "================================================="
#	exit(2)

print "BLED112 scan준비를 시작합니다."

#기존 0번째 handle 연결 해제
bledAPI.ble_cmd_connection_disconnect(ser, 0)
result = bledAPI.ble_rsp_connection_disconnect(ser)
#0000: 연결이 정상적으로 해제됨
#0186: 이미 연결이 끊겨있음
if (result == "0000") or (result == "0186"):
	print "BLED112의 기존 연결을 끊었습니다."
else:
	print "\n================================================="
	print "기존 연결을 끊지 못했습니다."
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)

#GAP 연결 불가 모드로 전환
#GAP 발견 불가 모드로 전환
bledAPI.ble_cmd_gap_set_mode(ser, 0, 0)
result = bledAPI.ble_rsp_gap_set_mode(ser)
#0000: 실행이 정상적으로 됨
if (result =="0000"):
	print "BLED112를 gap_non_connectable mode 및 gap_non_discoverable mode로 전환하였습니다."
else:
	print "\n================================================="
	print "BLED112를 gap_non_connectable mode 및 gap_non_discoverable mode로 전환하지 못했습니다."
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)

#Advertising중인 device scan 중지
bledAPI.ble_cmd_gap_end_procedure(ser)
result = bledAPI.ble_rsp_gap_end_procedure(ser)

#0000: 실행이 정상적으로 됨
if (result =="0000") or (result =="0181"):
	print "BLED112의 device scan을 중지합니다."
else:
	print "\n================================================="
	print "BLED112의 device scan을 중지하지 못했습니다."
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)

bledAPI.ble_cmd_system_get_info(ser)
isReal, build, version = bledAPI.ble_rsp_system_get_info(ser)
if isReal:
	print 'Build version: %d' % build 
	print 'API version: %d' % version

#Scan 정보 설정(default로 설정)
bledAPI.ble_cmd_gap_set_scan_parameters(ser, 0xC8, 0xC8, 1)
result = bledAPI.ble_rsp_gap_set_scan_parameters(ser)

#0000: 실행이 정상적으로 됨
if (result =="0000"):
	print "BLED112의 Scan 정보를 설정합니다."
else:
	print "\n================================================="
	print "BLED112의 Scan 정보를 설정하지 못했습니다."
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)

#Scan 시작
bledAPI.ble_cmd_gap_discover(ser, 1)
result = bledAPI.ble_rsp_gap_discover(ser)

#0000: 실행이 정상적으로 됨
if (result =="0000"):
	print "BLED112의 Scan을 시작합니다."
	print "================================================="
else:
	print "\n================================================="
	print "BLED112의 Scan을 시작하지 못했습니다"
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)

result = False
UUID = []
sender = []
address_type = 0
try:
	while (not result):
		result, UUID, sender, address_type = bledAPI.ble_msg_gap_set_scan_response_evt_t(ser)
		time.sleep(0.01)
except KeyboardInterrupt:
	bledAPI.ble_cmd_gap_end_procedure(ser)

try:
	if UUID == myoAPI.myo_uuid_trans(myoAPI.myohw_services.ControlService):
		print 'Myo를 찾았습니다.'
		bledAPI.ble_cmd_gap_end_procedure(ser)
		result = bledAPI.ble_rsp_gap_end_procedure(ser)

		#0000: 실행이 정상적으로 됨
		if (result =="0000") or (result =="0181"):
			print "BLED112의 device scan을 중지합니다."
		else:
			print "\n================================================="
			print "BLED112의 device scan을 중지하지 못했습니다."
			print "에러코드: %s" % result
			print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
			print "================================================="
			exit(2)
	else:
		print 'Myo를 찾지 못했습니다.'
		print 'Scan을 중지합니다.'
		bledAPI.ble_cmd_gap_end_procedure(ser)
		exit(1)
except:
	bledAPI.ble_cmd_gap_end_procedure(ser)
	exit(2)

ser.flushInput()
ser.flushOutput()

connected = False
for i in range(1, 10):
	print 'Myo (%s) 연결을 요청합니다.' % ':'.join('%02X' % b for b in sender[::-1])

	bledAPI.ble_cmd_gap_connect_direct(ser, sender, address_type, 0x0064, 0x006A, 0x012C, 0x0010)
	result = []
	

	while len(result)==0:
		result, connection_handle = bledAPI.ble_rsp_gap_connect_direct(ser)

	if result == "0000":
		connected = True
		print 'Myo와의 연결을 요청하였습니다'
		print 'preserved connection_handle: %d' % connection_handle
		print "================================================="
		break
	else:
		print "\n================================================="
		print "Myo와의 연결을 요청하지 못했습니다."
		print 'connection_handle: %d' % connection_handle
		print "에러코드: %s" % result
		print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
		print "================================================="

if not connected:
	exit(2)

getIt = False
try:
	while not getIt:
		getIt, connection_handle = bledAPI.ble_msg_connection_status_evt_t(ser)
except KeyboardInterrupt:
	exit(2)

#ser.flushInput()
#ser.flushOutput()

uuid = [0x00, 0x28]
#revMyoUUID = []
#for b in reversed(myoUUID):
#	revMyoUUID.append(b)
print "Myo의 정보를 요청합니다."
bledAPI.ble_cmd_attclient_read_by_handle(ser, connection_handle, 0x0017)

result = []
while len(result) == 0:
	handle, result = bledAPI.ble_rsp_attclient_read_by_handle(ser)

isItOver = False
while not isItOver:
	isItOver = bledAPI.ble_evt_attclient_attribute_value_evt_t(ser)

if result == "0000":
	connected = True
	print 'Myo의 정보를 성공적으로 받아왔습니다.'
	print 'connection_handle: %d' % handle
	print "================================================="
else:
	print "\n================================================="
	print "Myod의 Attribute를 받아오지 못했습니다."
	print 'connection_handle: %d' % handle
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="

ser.flushInput()
ser.flushOutput()

bledAPI.ble_cmd_connection_disconnect(ser, connection_handle)
result = bledAPI.ble_rsp_connection_disconnect(ser)
#0000: 연결이 정상적으로 해제됨
#0186: 이미 연결이 끊겨있음
if (result == "0000") or (result == "0186"):
	print "BLED112의 기존 연결을 끊었습니다."
else:
	print "\n================================================="
	print "기존 연결을 끊지 못했습니다."
	print "에러코드: %s" % result
	print "에러코드는 Bluegiga Blutooth Smart Software API reference를 참조하십시오"
	print "================================================="
	exit(2)
