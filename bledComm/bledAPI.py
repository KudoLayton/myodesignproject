#-*- coding: utf-8 -*-
import struct
import datetime
import time
import binascii

#error code를 원래 순서로 변환(에러코드가 자꾸 거꾸로 나오므로 해주어야 함)
def chn_errcode(origin_str, len_non_info):
	temp = origin_str[-(len(origin_str)-len_non_info):]
	temp = ''.join(reversed(temp))
	result = ''
	for b in temp:
		result = result + ("%02X" % (ord(b)))
	return result

#recieve test
def ble_recieve_test(p):
	expected_length = 0
	rx_buffer = []
	while(ser.inWaiting()):
			b = ord(ser.read())

			if expected_length == 0 and (b == 0x08 or b == 0x00):
				rx_buffer.append(b)

			elif expected_length == 1:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]

			else:
				rx_buffer.append(b)

			if len(rx_buffer) == expected_length:
				print "recieve: %s\n" % ''.join('%02X' % c for c in rx_buffer)
				rx_buffer = []


###############################################
#System class(0x00)

#Get Info(API reference p.191)
def ble_cmd_system_get_info(p):
	p.write(struct.pack('4B', 0x00, 0x00, 0x00, 0x08))

#response of Get Info
def ble_rsp_system_get_info(p):
	respond = p.read(16)
	isReal = False
	signature = []	
	for b in respond[:4]:
		signature.append(ord(b))
	if [0x00, 0x0C, 0x00, 0x08] == signature:
		isReal = True
	major, minor, patch, build, ll_version, protocol_version, hw = struct.unpack('<5HBB', respond[4:])
	return isReal, build, protocol_version

#Reset(API reference p.193)
#boot_in_dfu = 0:main program에서 부팅
#boot_in_dfu = 1:DFU(공장 초기화 같은 느낌?)에서 부팅
def ble_cmd_system_reset(p, boot_in_dfu):
	p.write(struct.pack('5B', 0, 1, 0, 0, boot_in_dfu))

#Hello(API reference p.192)
def ble_cmd_system_hello(p):
	p.write(struct.pack('4B', 0, 0, 0, 1))

#response of Hello
#result = "00000001": 정상 실행
#result != "00000001": 에러 발생
def ble_rsp_system_hello(p):
	response = p.read(4)
	temp = ''
	for b in response:
		temp = temp + ("%02X" % (ord(b)))
	return temp

###############################################
#Attribut Database class(0x02)

#Read(API reference p.70)
#handle: 읽고자하는 attribute handle
#offset: 읽고자하는 data의 offset 최대로 읽을 수 있는 data의 양은 32bit이다.
def ble_cmd_attributes_read(p, handle, offset):
	p.write(struct.pack('<4BHH', 0x00, 0x04, 0x02, 0x01, handle, offset))

#response of Read
#handle: 읽고자하는 attribute handle
#offset: 읽고자하는 data의 offset 최대로 읽을 수 있는 data의 양은 32bit이다.
#result = 0: 정상 실행
#result != 0: 에러 발생
#value: 내용
def ble_rsp_attributes_read(p):
	rx_buffer = []
	expected_length = 0
	handle = 0
	offset = 0
	result = []
	value = []
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x00:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
			rx_buffer.append(b)
			expected_length = 4 + rx_buffer[1]

		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length and expected_length > 0:
			packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
			rx_payload = rx_buffer[4:]
			handle, offset, _result, valueLen = struct.unpack('<3HB', ''.join(chr(b) for b in rx_payload[:7]))
			value = rx_payload[7:]
			result = '%04X' % _result

	return handle, offset, result, value

#Read Type(API reference p.72)
#handle: connect handle
def ble_cmd_attributes_read_type(p, handle):
	p.write(struct.pack('<4BH', 0x00, 0x02, 0x02, 0x02, handle))

#response of Read Type
#handle: connect handle
#result = 0: 정상적으로 읽었음
#result != 0: 에러 발생 (혹은 len(result) == 0 면 그냥 못받은 거)
#value: UUID
def ble_rsp_attributes_read_type(p):
	handle = 0
	result = []
	value = []
	expected_length = 0
	rx_buffer = []
	while(p.inWaiting()):
			b = ord(p.read())
			if expected_length == 0 and b == 0x00:
				rx_buffer.append(b)

			elif len(rx_buffer) == 1 and expected_length == 0:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]

			else:
				rx_buffer.append(b)

			if len(rx_buffer) == expected_length:

				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				handle, _result, value_len = struct.unpack('<HHB', ''.join(chr(b) for b in rx_payload[:5]))
				value = reversed(rx_payload[5:])
				result = ''.join('%04X' % _result)
				rx_buffer = []

	return handle, result, value

###############################################
#Connection class(0x03)

#Disconnect(API reference p.83)
#connection: 닫으려는 connection handle
def ble_cmd_connection_disconnect(p, connection):
	p.write(struct.pack('5B', 0, 1, 3, 0, connection))

#response of Disconnection
#result = 0: 정상 실행
#result != 0: 에러 발생(ex. 0x0186: 이미 해제되어있음)
def ble_rsp_connection_disconnect(p):
	response = p.read(7)
	return chn_errcode(response, 5)

###############################################
#Attribute Client class(0x04)

#Find Information(API reference p.47)
#connection: Connection Handle
#start: 첫번째 attribute handle
#end: 마지막 attribute handle
def ble_cmd_attclient_find_information(p, connection, start, end):
	p.write(struct.pack('<5BHH', 0x00, 0x05, 0x04, 0x03, connection, start, end))

#response of Find Information
#connection: Connection Handle
#result == 0: 정상 실행
#result != 0: 에러 발생
def ble_rsp_attclient_find_information(p):
	handle = 0
	result = []
	expected_length = 0
	rx_buffer = []
	found = False
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x00:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
			if b==0x03:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
			else:
				rx_buffer = []
		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length and expected_length > 0:
			#packet_type, payload_length, packet_class, packet_command = struct.unpack('4B', rx_buffer[:4])
			rx_payload = rx_buffer[4:]
			handle, result = struct.unpack('<BH', ''.join(chr(a) for a in rx_payload))
			result = ''.join('%04X' % result)
			rx_buffer = []
			break

	return handle, result

#Read By Group Type(API reference p.52)
#connection: Connection Handle
#start: 첫번째 handle number(처음 부터 찾으려면: 0x0001)
#end: 마지막 handle number(마지막까지 찾으려면: 0xFFFF)
#uuid: 찾으려는 Group uuid(Primary service: 0x2800, Secondary service: 0x2801)
def ble_cmd_attclient_read_by_group_type(p, connection, start, end, uuid):
	uuidLen = len(uuid)
	packetLen = 0x06 + len(uuid)
	packIndex = '<5BHHB%dB' % uuidLen
	p.write(struct.pack(packIndex, 0x00, packetLen, 0x04, 0x01, connection, start, end, uuidLen, *uuid))

#response of Read By Group Type
#connection: Connection Handle
#result = 0: 정상 실행
#result != 0: 오류 발생
def ble_rsp_attclient_read_by_group_type(p):
	handle = 0
	result = []
	expected_length = 0
	rx_buffer = []
	found = False
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x00:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
			if b==0x03:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
			else:
				rx_buffer = []
		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length and expected_length > 0:
			#packet_type, payload_length, packet_class, packet_command = struct.unpack('4B', rx_buffer[:4])
			rx_payload = rx_buffer[4:]
			handle, result = struct.unpack('<BH', ''.join(chr(a) for a in rx_payload))
			result = ''.join('%04X' % result)
			rx_buffer = []
			break

	return handle, result

#Read By Handle(API reference p.54)
#connection: Connection Handle
#chrhandle: Attribute Handle
def ble_cmd_attclient_read_by_handle(p, connection, chrhandle):
	p.write(struct.pack('<5BH', 0x00, 0x03, 0x04, 0x04, connection, chrhandle))

#response of Read By Handle
def ble_rsp_attclient_read_by_handle(p):
	handle = 0
	result = []
	expected_length = 0
	rx_buffer = []
	found = False
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x00:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
			if b==0x03:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
			else:
				rx_buffer = []
		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length and expected_length > 0:
			#packet_type, payload_length, packet_class, packet_command = struct.unpack('4B', rx_buffer[:4])
			rx_payload = rx_buffer[4:]
			handle, result = struct.unpack('<BH', ''.join(chr(a) for a in rx_payload))
			result = ''.join('%04X' % result)
			rx_buffer = []
			break

	return handle, result

#Read By Type(API reference p.56)
#connection: Connection Handle
#start: 첫번째 handle number(처음 부터 찾으려면: 0x0001)
#end: 마지막 handle number(마지막까지 찾으려면: 0xFFFF)
#uuid: 찾으려는 Attribute type uuid(ex 0x2803: characteristic declaration)
def ble_cmd_attclient_read_by_type(p, connection, start, end, uuid):
	uuidLen = len(uuid)
	packetLen = 0x06 + len(uuid)
	packIndex = '<5BHHB%dB' % uuidLen
	p.write(struct.pack(packIndex, 0x00, packetLen, 0x04, 0x02, connection, start, end, uuidLen, *uuid))

#response of Read By Type
#connection: Connection Handle
#result = 0: 정상 실행
#result != 0: 오류 발생
def ble_rsp_attclient_read_by_type(p):
	handle = 0
	result = []
	expected_length = 0
	rx_buffer = []
	found = False
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x00:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
			if b==0x03:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
			else:
				rx_buffer = []
		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length and expected_length > 0:
			#packet_type, payload_length, packet_class, packet_command = struct.unpack('4B', rx_buffer[:4])
			rx_payload = rx_buffer[4:]
			connection, result = struct.unpack('<BH', ''.join(chr(a) for a in rx_payload))
			result = ''.join('%04X' % result)
			rx_buffer = []
			break

	return handle, result

#Attribute Value(API reference p.64)
#connection: connection handle
#atthandle: attribute handle
#type: attribute type(API reference 63)
#value: Attribute Value
def ble_evt_attclient_attribute_value_evt_t(p):
	result = []
	expected_length = 0
	rx_buffer = []
	isItOver = False
	while(p.inWaiting()):

		b = ord(p.read())

		if expected_length == 0 and b == 0x80:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
		else:
			rx_buffer.append(b)
		if len(rx_buffer) == expected_length and expected_length > 0:
			if rx_buffer[3] == 0x05:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				connection, atthandle, Type, valueLen = struct.unpack('<BHBB', ''.join(chr(b) for b in rx_payload[:5]))
				value = rx_payload[5:]
				print "connection: %d" % connection
				print "atthandle: %04X" % atthandle
				if Type == 0x00:
					print "Attribute type: Value was read"
				elif Type == 0x01:
					print "Attribute type: Value was notified"
				elif Type == 0x02:
					print "Attribute type: Value was indicated"
				elif Type == 0x03:
					print "Attribute type: Value was read"
				elif Type == 0x04:
					print "Attribute type: Value was part of a long attribute"
				elif Type == 0x05:
					print "Attribute type: Value was indicated and the remote device is waiting for a confirmation"

				print "value : %s" % ''.join('%02X' % b for b in reversed(value))
				print "================================================="
				rx_buffer = []
				expected_length = 0
				
				#검색 시에는 주석 처리 할 것
				isItOver = True
				return isItOver
				#########################

			elif rx_buffer[3] == 0x01:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				connection, result, chrhandle = struct.unpack('<BHH', ''.join(chr(b) for b in rx_payload[:5]))
				print "connection: %d" % connection
				print "result: %04X" % result
				print "chrhandle: %04X" % chrhandle
				print "================================================="
				isItOver = True
				return isItOver
	return isItOver

#Information Found(API reference p.65)
#connection: connection handle
#chrhandle: characteristics handlle
#uuid: characteristics type
def ble_evt_attclient_find_information_found(p):
	result = []
	expected_length = 0
	rx_buffer = []
	isItOver = False
	while(p.inWaiting()):

		b = ord(p.read())

		if expected_length == 0 and b == 0x80:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
		else:
			rx_buffer.append(b)
		if len(rx_buffer) == expected_length and expected_length > 0:
			if rx_buffer[3] == 0x04:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				connection, chrhandle, valueLen = struct.unpack('<BHB', ''.join(chr(b) for b in rx_payload[:4]))
				value = rx_payload[4:]
				print "connection: %d" % connection
				print "chrhandle: %04X" % chrhandle
				print "value : %s" % ''.join('%02X' % b for b in value)
				print "================================================="
			elif rx_buffer[3] == 0x01:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				connection, result, chrhandle = struct.unpack('<BHH', ''.join(chr(b) for b in rx_payload[:5]))
				print "End Procedure"
				print "connection: %d" % connection
				print "result: %04X" % result
				print "chrhandle: %04X" % chrhandle
				print "================================================="
				isItOver = True
				return isItOver
	return isItOver

#Group Found(API reference p.66)
#connection: connection handle
#start: 첫번째 handle number
#end: 마지막 handle number
#uuid: uuid
def ble_msg_attclient_group_found_evt_t(p):
	result = []
	expected_length = 0
	rx_buffer = []
	isItOver = False
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x80:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]
		else:
			rx_buffer.append(b)
		if len(rx_buffer) == expected_length and expected_length > 0:
			if rx_buffer[3] == 0x02:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				connection, start, end, uuidLen = struct.unpack('<BHHB', ''.join(chr(b) for b in rx_payload[:6]))
				uuid = rx_payload[6:]
				print "connection: %d" % connection
				print "start: %04X" % start
				print "end: %04X" % end
				print "uuid: %s" % ''.join('%02X' % b for b in uuid)
				print "================================================="
				rx_buffer = []
				expected_length = 0
			elif rx_buffer[3] == 0x01:
				packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
				rx_payload = rx_buffer[4:]
				print 'len: %d' % len(rx_payload[:5])
				connection, result, chrhandle = struct.unpack('<BHH', ''.join(chr(b) for b in rx_payload[:5]))
				print "connection: %d" % connection
				print "result: %04X" % result
				print "chrhandle: %04X" % chrhandle
				print "================================================="
				isItOver = True
				return isItOver
	return isItOver

#Status(API reference p.93)
#connection: Connection handle
#flags: Connection 상태. connstatus-enum을 참고(p.90)
#address: 해당 Bluetooth 장치 주소
#address_type: address type을 말한다 Bluetooth Address Types--gap 참고(p.116)
#conn_interval: 현재 connection interval (단위: 1.25ms)
#timeout: 현재 supervision timeout (단위: 1.25ms)
#latency: 현재 latency
#bonding: Bonding handle로 그 장치가 연결되어있었을 때 출력되며 그렇지 않다면 0xFF이다.
def ble_msg_connection_status_evt_t(p):
	getIt = False
	connection = 0
	flags = 0
	address = []
	address_type = 0
	conn_interval = 0
	timeout = 0
	latency = 0
	bonding = 0
	expected_length = 0
	rx_buffer = []
	while(p.inWaiting()):
		b = ord(p.read())
		if expected_length == 0 and b == 0x80:
			rx_buffer.append(b)

		elif len(rx_buffer) == 1 and expected_length == 0:
				rx_buffer.append(b)
				expected_length = 4 + rx_buffer[1]

		else:
			rx_buffer.append(b)

		if len(rx_buffer) == expected_length:
			getIt = True
			packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
			rx_payload = rx_buffer[4:]
			connection, flags = struct.unpack('2B', ''.join(chr(b) for b in rx_payload[0:2]))
			address = reversed(rx_payload[2:8])
			address_type, conn_interval, timeout, latency, bonding = struct.unpack('<B3HB', ''.join(chr(b) for b in rx_payload[8:16]))
			print "connection: %d" % connection
			if flags & 0x01:
				print "0 bit flag: connection_connected"
			if flags & 0x02:
				print "1 bit flag: connection_encrypted"
			if flags & 0x04:
				print "2 bit flag: connection_completed"
			if flags & 0x08:
				print "3 bit flag: connection_parameter_change"

			print "address: %s" % ':'.join('%02X' % b for b in address)
			print "conn_interval: %.2fms" % (1.25 * conn_interval)
			print "timeout: %dms" % (10 * timeout)
			print "latency: %d" % latency
			if bonding == 0xFF:
				print "No Bonding"
			else:
				print "bonding: %d", bonding
			print "================================================="
	return getIt, connection

###############################################
#Generic Access Profile class(0x06)

#ConnectDirect(API reference p.95)
#bd_addr: 목표 장치의 Bluetooth address

#addr_type = 1: random address
#addr_type = 0: public address

#conn_interval_min: 최소 연결 간격으로 6 - 3200사이에서 결정할 수 있다.(단위는 1.25ms)
#conn_interval_max: 최대 연결 간격으로 6 - 3200사이에서 결정할 수 있다.(단위는 1.25ms)
#timeout: timeout을 정하는 parameter로 10 - 3200사이에서 결정할 수 있다.(단위는 10ms)
#latency: slave latency를 결정하는 parameter이다. 주의!: (Slave_Laytency + 1) * Connection interval < timeout
def ble_cmd_gap_connect_direct(p, bd_addr, addr_type, conn_interval_min, conn_interval_max, timeout, latency):
	addr_0 = bd_addr[0]
	addr_1 = bd_addr[1]
	addr_2 = bd_addr[2]
	addr_3 = bd_addr[3]
	addr_4 = bd_addr[4]
	addr_5 = bd_addr[5]
 	p.write(struct.pack('<11BHHHH', 0x00, 0x0F, 0x06, 0x03, addr_0, addr_1, addr_2, addr_3, addr_4, addr_5, addr_type, conn_interval_min, conn_interval_max, timeout, latency))

#result = 0: 정상적으로 연결됨
#result != -: 에러 발생
#_connection_handle: 새로운 연결에 대한 connection handle값
def ble_rsp_gap_connect_direct(p):
	result = []
	rx_buffer = []
	rx_expected_length = 0
	_connection_handle = 0
	while(p.inWaiting()):
		b = ord(p.read())
		if b == 0x00 and rx_expected_length == 0:
			rx_buffer.append(b)
		elif len(rx_buffer) == 1:
			if b == 0x03:
				rx_buffer.append(b)
				rx_expected_length = 4 + rx_buffer[1]
			else:
				rx_buffer = []
				rx_expected_length = 0
		else:
			rx_buffer.append(b)

		if len(rx_buffer) == rx_expected_length and rx_expected_length > 0:
			_result = rx_buffer[4:6]
			_connection_handle = rx_buffer[6]
			result = ''.join('%02X' % c for c in reversed(_result))
			return result, _connection_handle

	return result, _connection_handle

#Discover(API reference p.101)
#mode: GAP Discover Mode 참조(API reference p.119)
def ble_cmd_gap_discover(p, mode):
	p.write(struct.pack('5B', 0, 1, 6, 2, mode))

#response of Discover
#result = 0: 정상 실행
#result != 0: 에러 발생
def ble_rsp_gap_discover(p):
	response = p.read(6)
	return chn_errcode(response, 4)

#End Procedure(API reference p.102)
def ble_cmd_gap_end_procedure(p):
	p.write(struct.pack('4B', 0, 0, 6, 4))

#response of End Procedure
#result = 0: 정상 실행
#result != 0: 에러 발생
def ble_rsp_gap_end_procedure(p):
	response = p.read(6)
	return chn_errcode(response, 4)

#Set Mode(API reference p.108)
#discover: GAP Discoverable Mode를 참조(API reference p.118)
#connect: GAP Connectable Mode를 참조(API reference p.117)
def ble_cmd_gap_set_mode(p, discover, connect):
	p.write(struct.pack('6B', 0, 2, 6, 1, discover, connect))

#response of Set Mode
#result = 0: 정상 실행
#result != 0: 에러 발생
def ble_rsp_gap_set_mode(p):
	response = p.read(6)
	return chn_errcode(response, 4)

#Set Scan Parameters(API reference p.111)
#scan_interval = 0x4 - 0x4000: 재스캔을 하는데 까지 걸리는 시간(unit: 625us, default: 0x4B(75ms))
#scan_window = 0x4 - 0x4000: 스캔 응답을 기다리는 시간(unit: 625us, default: 0x32(50ms))
#active = 1: Active Scanning / active = 0: Passive Scanning 
def ble_cmd_gap_set_scan_parameters(p, scan_interval, scan_window, active):
	p.write(struct.pack('<4BHHB', 0, 5, 6, 7, scan_interval, scan_window, active))

#response of Set Scan Parameters
#result = 0: 정상 실행
#result != 0: 에러 발생
def ble_rsp_gap_set_scan_parameters(p):
	response = p.read(6)
	return chn_errcode(response, 4)

#Scan Response(API reference p.122)
#rssi(Receiver Signal Strength Indication): dBm단위로 -103~ -38까지 표현할 수 있다.

#packet_type = 0: Connectable Advertisement packet
#packet_type = 2: Non Connectable Advertisement packet
#packet_type = 4: Scan response packet
#packet_type = 6: Discoverable advertisement packet

#sender: Advertisers Bluetooth address

#address_type = 1: random address
#address_type = 0: public address

#bond: 이 장비에 bond가 이미 있을 경우 bond handle 출력(없다면 0xff)
#data: Scan response data
def ble_msg_gap_set_scan_response_evt_t(p):
	rx_buffer = []
	rx_expected_length = 0
	UUID = []
	sender = []
	address_type = 0
	find = False
	while(p.inWaiting()):
		b = ord(p.read())
		if len(rx_buffer) == 0 and b == 0x80:
			rx_buffer.append(b)
		elif len(rx_buffer) == 1:
			rx_buffer.append(b)
			rx_expected_length = 4 + (rx_buffer[0] & 0x07) + rx_buffer[1]
		elif len(rx_buffer) > 1:
			rx_buffer.append(b)

		if rx_expected_length > 0 and len(rx_buffer) == rx_expected_length:
			packet_type, payload_length, packet_class, packet_command = rx_buffer[:4]
			rx_payload = b''.join(chr(i) for i in rx_buffer[4:])
			if packet_type & 0x80 == 0x00:
				bgapi_filler = 0
			else:
				if packet_class == 0x06:
					rssi, packet_type, sender, address_type, bond, data_len = struct.unpack('<bB6sBBB', rx_payload[:11])
					sender = [ord(b) for b in sender]
					data_data = [ord(b) for b in rx_payload[11:]]

					print "RSSI: %ddBm" % (rssi)

					if packet_type == 0:
						print "packet type: Connectable Advertisement packet"

					elif packet_type == 2:
						print "packet type: Non Connectable Advertisement packet"

					elif packet_type == 4:
						print "packet type: Scan response packet"

					elif packet_type == 6:
						print "packet type: Discoverable advertisement packet"

					print "sender: %s" % ':'.join(['%02X' % b for b in sender[::-1]])

					if address_type == 1:
						print "address type: random address"
					elif address_type == 0:
						print "address type: public address"

					if bond == 0xFF:
						print "bond: no bond"
					else:
						print "bond: %d" % bond

					ad_fields = []
					this_field = []
					ad_flags = 0
					ad_services = []
					ad_local_name = []
					ad_tx_power_level = 0
					ad_manufacturer = []

					bytes_left = 0

					for b in data_data:
						if bytes_left == 0:
							bytes_left = b
							this_field = []
						else:
							this_field.append(b)
							bytes_left = bytes_left - 1
							if bytes_left == 0:
								ad_fields.append(this_field)

								#flags
								if this_field[0] == 0x01:
									ad_flags = this_field[1]

								#partial or complete list of 16-bit UUIDs
								if this_field[0] == 0x02 or this_field[0] == 0x03:
									UUID_str = []
									for i in xrange((len(this_field)-1)/2):
										ad_services.append(this_field[-1 - i * 2 : -3 - i * 2 : -1])
									for i in ad_services:
										UUID = i[::-1]
										for c in i:
											UUID_str.append('%02X' % c)
									if len(UUID_str) > 0:
										print "16 bit UUID: %s" % ''.join(UUID_str)
									else:
										print "No ID"

								#partial or complete list of 32-bit UUIDs
								if this_field[0] == 0x04 or this_field[0] == 0x05:
									UUID_str = []
									for i in xrange((len(this_field)-1)/4):
										ad_services.append(this_field[-1 - i * 4 : -5 - i * 4 : -1])
									for i in ad_services:
										UUID = i[::-1]
										for c in i:
											UUID_str.append('%02X' % c)
									if len(UUID_str) > 0:
										print "32 bit UUID: %s" % ''.join(UUID_str)
									else:
										print "No ID"

								#partial or complete list of 128-bit UUIDs
								if this_field[0] == 0x06 or this_field[0] == 0x07:
									UUID_str = []
									for i in xrange((len(this_field)-1)/16):
										ad_services.append(this_field[-1 - i * 16 : -17 - i * 16 : -1])
									for i in ad_services:
										UUID = i[::-1]
										for c in i:
											UUID_str.append('%02X' % c)
									if len(UUID_str) > 0:
										print "128 bit UUID: %s" % ''.join(UUID_str)
									else:
										print "No ID"

								#shortened or complete local name
								if this_field[0] == 0x08 or this_field[0] == 0x09:
									ad_local_name = this_field[1:]
									print "local name: %s" % ''.join([chr(c) for c in ad_local_name])

								#TX power level
								if this_field[0] == 0x0A:
									ad_tx_power_level = this_field[1]
									print "TX power level: %d" % ad_tx_power_level

								#manufactuerer
								if this_field[0] == 0xFF:
									ad_manufacturer.append(this_field[1:])
									print "manufactuerer: %s" % ''.join(chr(c) for c in ad_manufacturer)
					
					

					if len(data_data) == 0:
						print "No Data"

					print "================================================="
				find = True
			rx_buffer = []
	return find, UUID, sender, address_type

