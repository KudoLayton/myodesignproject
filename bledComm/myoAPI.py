#-*- coding: utf-8 -*-

#enum 정의
def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)


###############################################
#UUID 관련
#myo기본 UUID
myoBaseUUID = [0x42, 0x48, 0x12, 0x4A, 0x7F, 0x2C, 0x48, 0x47,0xB9, 0xDE, 0x04, 0xA9, 0x00, 0x00, 0x06, 0xD5]

#myo의 자체 서비스별 UUID 주소
#myo의 UUID는 서비스에 따라 0x0000부분(12, 13 byte)가 달라진다.
#아래는 big endian으로 되어있지만 전송시에는 반드시 little endian으로 보내야 한다.  
myohw_services = enum(
    ControlService                = 0x0001, # Myo info service
    MyoInfoCharacteristic         = 0x0101, # Serial number for this Myo and various parameters which
                                            # are specific to this firmware. Read-only attribute. 
                                            # See myohw_fw_info_t.
    FirmwareVersionCharacteristic = 0x0201, # Current firmware version. Read-only characteristic.
                                            # See myohw_fw_version_t.
    CommandCharacteristic         = 0x0401, # Issue commands to the Myo. Write-only characteristic.
                                            # See myohw_command_t.

    ImuDataService                = 0x0002, # IMU service
    IMUDataCharacteristic         = 0x0402, # See myohw_imu_data_t. Notify-only characteristic.
    MotionEventCharacteristic     = 0x0502, # Motion event data. Indicate-only characteristic.

    ClassifierService             = 0x0003, # Classifier event service.
    ClassifierEventCharacteristic = 0x0103, # Classifier event data. Indicate-only characteristic. See myohw_pose_t.

    EmgDataService                = 0x0005, # Raw EMG data service.
    EmgData0Characteristic        = 0x0105, # Raw EMG data. Notify-only characteristic.
    EmgData1Characteristic        = 0x0205, # Raw EMG data. Notify-only characteristic.
    EmgData2Characteristic        = 0x0305, # Raw EMG data. Notify-only characteristic.
    EmgData3Characteristic        = 0x0405, # Raw EMG data. Notify-only characteristic.
)

#표준 Bluetooth 서비스별 UUID 주소
myohw_standard_services = enum(
    BatteryService                = 0x180f, #Battery service
    BatteryLevelCharacteristic    = 0x2a19, #Current battery level information. Read/notify characteristic.
    DeviceName                    = 0x2a00, #Device name data. Read/write characteristic.
)

#각 Attribute별 handle
myohw_chr_handle = enum(
    ControlService                = 0x0013, # Myo info service
    MyoInfoCharacteristic         = 0x0015, # Serial number for this Myo and various parameters which
                                            # are specific to this firmware. Read-only attribute. 
                                            # See myohw_fw_info_t.
    FirmwareVersionCharacteristic = 0x0017, # Current firmware version. Read-only characteristic.
                                            # See myohw_fw_version_t.
    CommandCharacteristic         = 0x0019, # Issue commands to the Myo. Write-only characteristic.
                                            # See myohw_command_t.

    ImuDataService                = 0x001A, # IMU service
    IMUDataCharacteristic         = 0x001C, # See myohw_imu_data_t. Notify-only characteristic.
    MotionEventCharacteristic     = 0x001F, # Motion event data. Indicate-only characteristic.

    ClassifierService             = 0x0021, # Classifier event service.
    ClassifierEventCharacteristic = 0x0023, # Classifier event data. Indicate-only characteristic. See myohw_pose_t.

    EmgDataService                = 0x0029, # Raw EMG data service.
    EmgData0Characteristic        = 0x002B, # Raw EMG data. Notify-only characteristic.
    EmgData1Characteristic        = 0x002E, # Raw EMG data. Notify-only characteristic.
    EmgData2Characteristic        = 0x0031, # Raw EMG data. Notify-only characteristic.
    EmgData3Characteristic        = 0x0034, # Raw EMG data. Notify-only characteristic.
)

#myo의 서비스에 맞는 UUID 주소로 바꾸어 준다.
def myo_uuid_trans(service):
    UUID = myoBaseUUID
    UUID[13] = service / 16
    UUID[12] = service % 16
    return UUID


###############################################
#pose 정보
myohw_pose_t = enum (
    myohw_pose_rest           = 0x0000,
    myohw_pose_fist           = 0x0001,
    myohw_pose_wave_in        = 0x0002,
    myohw_pose_wave_out       = 0x0003,
    myohw_pose_fingers_spread = 0x0004,
    myohw_pose_double_tap     = 0x0005,
    myohw_pose_unknown        = 0xffff
)

###############################################
#비례 상수
MYOHW_ORIENTATION_SCALE =  16384.0 # See myohw_imu_data_t::orientation
MYOHW_ACCELEROMETER_SCALE = 2048.0  # See myohw_imu_data_t::accelerometer
MYOHW_GYROSCOPE_SCALE =    16.0    # See myohw_imu_data_t::gyroscope