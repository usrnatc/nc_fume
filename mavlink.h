#if !defined(__MAVLINK_H__)
#define __MAVLINK_H__

#include "nc_types.h"
#include "nc_string.h"

// @defines____________________________________________________________________
#define MAVLINK_MAGIC_VERSION_1       0xFE
#define MAVLINK_MAGIC_VERSION_2       0xFD
#define MAVLINK_HEADER_SIZE_VERSION_1 6
#define MAVLINK_HEADER_SIZE_VERSION_2 10
#define MAVLINK_CRC_SIZE              2
#define MAVLINK_SIGNATURE_SIZE        13
#define MAVLINK_FRAME_SIZE_MIN        (MAVLINK_HEADER_SIZE_VERSION_1 + MAVLINK_CRC_SIZE)
#define MAVLINK_IFLAG_SIGNED          0x01
#define MAVLINK_CRC_SEED              U16_MAX
#define MAVLINK_CRC_POLYNOMIAL        0x8408
#define MAVLINK_CRC_SLICE_COUNT       8
#define MAVLINK_MSG_SLOTS_COUNT       (U16_MAX + 1)
#define MAVLINK_MSG_SLOT_NONE         U16_MAX
#define MAVLINK_EXTRA_NONE            0x0100

#define MAVLINK_IS_MAGIC(X) ((u8) ((X) - MAVLINK_MAGIC_VERSION_2) <= 1)

#define MAVLINK_MSG_XLIST                                     \
    X(    0,  50, HEARTBEAT)                                  \
    X(    1, 124, SYS_STATUS)                                 \
    X(    2, 137, SYSTEM_TIME)                                \
    X(    4, 237, PING)                                       \
    X(    5, 217, CHANGE_OPERATOR_CONTROL)                    \
    X(    6, 104, CHANGE_OPERATOR_CONTROL_ACK)                \
    X(    7, 119, AUTH_KEY)                                   \
    X(   11,  89, SET_MODE)                                   \
    X(   20, 214, PARAM_REQUEST_READ)                         \
    X(   21, 159, PARAM_REQUEST_LIST)                         \
    X(   22, 220, PARAM_VALUE)                                \
    X(   23, 168, PARAM_SET)                                  \
    X(   24,  24, GPS_RAW_INT)                                \
    X(   25,  23, GPS_STATUS)                                 \
    X(   26, 170, SCALED_IMU)                                 \
    X(   27, 144, RAW_IMU)                                    \
    X(   28,  67, RAW_PRESSURE)                               \
    X(   29, 115, SCALED_PRESSURE)                            \
    X(   30,  39, ATTITUDE)                                   \
    X(   31, 246, ATTITUDE_QUATERNION)                        \
    X(   32, 185, LOCAL_POSITION_NED)                         \
    X(   33, 104, GLOBAL_POSITION_INT)                        \
    X(   34, 237, RC_CHANNELS_SCALED)                         \
    X(   35, 244, RC_CHANNELS_RAW)                            \
    X(   36, 222, SERVO_OUTPUT_RAW)                           \
    X(   37, 212, MISSION_REQUEST_PARTIAL_LIST)               \
    X(   38,   9, MISSION_WRITE_PARTIAL_LIST)                 \
    X(   39, 254, MISSION_ITEM)                               \
    X(   40, 230, MISSION_REQUEST)                            \
    X(   41,  28, MISSION_SET_CURRENT)                        \
    X(   42,  28, MISSION_CURRENT)                            \
    X(   43, 132, MISSION_REQUEST_LIST)                       \
    X(   44, 221, MISSION_COUNT)                              \
    X(   45, 232, MISSION_CLEAR_ALL)                          \
    X(   46,  11, MISSION_ITEM_REACHED)                       \
    X(   47, 153, MISSION_ACK)                                \
    X(   48,  41, SET_GPS_GLOBAL_ORIGIN)                      \
    X(   49,  39, GPS_GLOBAL_ORIGIN)                          \
    X(   50,  78, PARAM_MAP_RC)                               \
    X(   51, 196, MISSION_REQUEST_INT)                        \
    X(   54,  15, SAFETY_SET_ALLOWED_AREA)                    \
    X(   55,   3, SAFETY_ALLOWED_AREA)                        \
    X(   61, 167, ATTITUDE_QUATERNION_COV)                    \
    X(   62, 183, NAV_CONTROLLER_OUTPUT)                      \
    X(   63, 119, GLOBAL_POSITION_INT_COV)                    \
    X(   64, 191, LOCAL_POSITION_NED_COV)                     \
    X(   65, 118, RC_CHANNELS)                                \
    X(   66, 148, REQUEST_DATA_STREAM)                        \
    X(   67,  21, DATA_STREAM)                                \
    X(   69, 243, MANUAL_CONTROL)                             \
    X(   70, 124, RC_CHANNELS_OVERRIDE)                       \
    X(   73,  38, MISSION_ITEM_INT)                           \
    X(   74,  20, VFR_HUD)                                    \
    X(   75, 158, COMMAND_INT)                                \
    X(   76, 152, COMMAND_LONG)                               \
    X(   77, 143, COMMAND_ACK)                                \
    X(   81, 106, MANUAL_SETPOINT)                            \
    X(   82,  49, SET_ATTITUDE_TARGET)                        \
    X(   83,  22, ATTITUDE_TARGET)                            \
    X(   84, 143, SET_POSITION_TARGET_LOCAL_NED)              \
    X(   85, 140, POSITION_TARGET_LOCAL_NED)                  \
    X(   86,   5, SET_POSITION_TARGET_GLOBAL_INT)             \
    X(   87, 150, POSITION_TARGET_GLOBAL_INT)                 \
    X(   89, 231, LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET)    \
    X(   90, 183, HIL_STATE)                                  \
    X(   91,  63, HIL_CONTROLS)                               \
    X(   92,  54, HIL_RC_INPUTS_RAW)                          \
    X(   93,  47, HIL_ACTUATOR_CONTROLS)                      \
    X(  100, 175, OPTICAL_FLOW)                               \
    X(  101, 102, GLOBAL_VISION_POSITION_ESTIMATE)            \
    X(  102, 158, VISION_POSITION_ESTIMATE)                   \
    X(  103, 208, VISION_SPEED_ESTIMATE)                      \
    X(  104,  56, VICON_POSITION_ESTIMATE)                    \
    X(  105,  93, HIGHRES_IMU)                                \
    X(  106, 138, OPTICAL_FLOW_RAD)                           \
    X(  107, 108, HIL_SENSOR)                                 \
    X(  108,  32, SIM_STATE)                                  \
    X(  109, 185, RADIO_STATUS)                               \
    X(  110,  84, FILE_TRANSFER_PROTOCOL)                     \
    X(  111,  34, TIMESYNC)                                   \
    X(  112, 174, CAMERA_TRIGGER)                             \
    X(  113, 124, HIL_GPS)                                    \
    X(  114, 237, HIL_OPTICAL_FLOW)                           \
    X(  115,   4, HIL_STATE_QUATERNION)                       \
    X(  116,  76, SCALED_IMU2)                                \
    X(  117, 128, LOG_REQUEST_LIST)                           \
    X(  118,  56, LOG_ENTRY)                                  \
    X(  119, 116, LOG_REQUEST_DATA)                           \
    X(  120, 134, LOG_DATA)                                   \
    X(  121, 237, LOG_ERASE)                                  \
    X(  122, 203, LOG_REQUEST_END)                            \
    X(  123, 250, GPS_INJECT_DATA)                            \
    X(  124,  87, GPS2_RAW)                                   \
    X(  125, 203, POWER_STATUS)                               \
    X(  126, 220, SERIAL_CONTROL)                             \
    X(  127,  25, GPS_RTK)                                    \
    X(  128, 226, GPS2_RTK)                                   \
    X(  129,  46, SCALED_IMU3)                                \
    X(  130,  29, DATA_TRANSMISSION_HANDSHAKE)                \
    X(  131, 223, ENCAPSULATED_DATA)                          \
    X(  132,  85, DISTANCE_SENSOR)                            \
    X(  133,   6, TERRAIN_REQUEST)                            \
    X(  134, 229, TERRAIN_DATA)                               \
    X(  135, 203, TERRAIN_CHECK)                              \
    X(  136,   1, TERRAIN_REPORT)                             \
    X(  137, 195, SCALED_PRESSURE2)                           \
    X(  138, 109, ATT_POS_MOCAP)                              \
    X(  139, 168, SET_ACTUATOR_CONTROL_TARGET)                \
    X(  140, 181, ACTUATOR_CONTROL_TARGET)                    \
    X(  141,  47, ALTITUDE)                                   \
    X(  142,  72, RESOURCE_REQUEST)                           \
    X(  143, 131, SCALED_PRESSURE3)                           \
    X(  144, 127, FOLLOW_TARGET)                              \
    X(  146, 103, CONTROL_SYSTEM_STATE)                       \
    X(  147, 154, BATTERY_STATUS)                             \
    X(  148, 178, AUTOPILOT_VERSION)                          \
    X(  149, 200, LANDING_TARGET)                             \
    X(  150, 134, SENSOR_OFFSETS)                             \
    X(  151, 219, SET_MAG_OFFSETS)                            \
    X(  152, 208, MEMINFO)                                    \
    X(  153, 188, AP_ADC)                                     \
    X(  154,  84, DIGICAM_CONFIGURE)                          \
    X(  155,  22, DIGICAM_CONTROL)                            \
    X(  156,  19, MOUNT_CONFIGURE)                            \
    X(  157,  21, MOUNT_CONTROL)                              \
    X(  158, 134, MOUNT_STATUS)                               \
    X(  160,  78, FENCE_POINT)                                \
    X(  161,  68, FENCE_FETCH_POINT)                          \
    X(  162, 189, FENCE_STATUS)                               \
    X(  163, 127, AHRS)                                       \
    X(  164, 154, SIMSTATE)                                   \
    X(  165,  21, HWSTATUS)                                   \
    X(  166,  21, RADIO)                                      \
    X(  167, 144, LIMITS_STATUS)                              \
    X(  168,   1, WIND)                                       \
    X(  169, 234, DATA16)                                     \
    X(  170,  73, DATA32)                                     \
    X(  171, 181, DATA64)                                     \
    X(  172,  22, DATA96)                                     \
    X(  173,  83, RANGEFINDER)                                \
    X(  174, 167, AIRSPEED_AUTOCAL)                           \
    X(  175, 138, RALLY_POINT)                                \
    X(  176, 234, RALLY_FETCH_POINT)                          \
    X(  177, 240, COMPASSMOT_STATUS)                          \
    X(  178,  47, AHRS2)                                      \
    X(  179, 189, CAMERA_STATUS)                              \
    X(  180,  52, CAMERA_FEEDBACK)                            \
    X(  181, 174, BATTERY2)                                   \
    X(  182, 229, AHRS3)                                      \
    X(  183,  85, AUTOPILOT_VERSION_REQUEST)                  \
    X(  184, 159, REMOTE_LOG_DATA_BLOCK)                      \
    X(  185, 186, REMOTE_LOG_BLOCK_STATUS)                    \
    X(  186,  72, LED_CONTROL)                                \
    X(  191,  92, MAG_CAL_PROGRESS)                           \
    X(  192,  36, MAG_CAL_REPORT)                             \
    X(  193,  71, EKF_STATUS_REPORT)                          \
    X(  194,  98, PID_TUNING)                                 \
    X(  195, 120, DEEPSTALL)                                  \
    X(  200, 134, GIMBAL_REPORT)                              \
    X(  201, 205, GIMBAL_CONTROL)                             \
    X(  214,  69, GIMBAL_TORQUE_CMD_REPORT)                   \
    X(  215, 101, GOPRO_HEARTBEAT)                            \
    X(  216,  50, GOPRO_GET_REQUEST)                          \
    X(  217, 202, GOPRO_GET_RESPONSE)                         \
    X(  218,  17, GOPRO_SET_REQUEST)                          \
    X(  219, 162, GOPRO_SET_RESPONSE)                         \
    X(  225, 208, EFI_STATUS)                                 \
    X(  226, 207, RPM)                                        \
    X(  230, 163, ESTIMATOR_STATUS)                           \
    X(  231, 105, WIND_COV)                                   \
    X(  232, 151, GPS_INPUT)                                  \
    X(  233,  35, GPS_RTCM_DATA)                              \
    X(  234, 150, HIGH_LATENCY)                               \
    X(  235, 179, HIGH_LATENCY2)                              \
    X(  241,  90, VIBRATION)                                  \
    X(  242, 104, HOME_POSITION)                              \
    X(  243,  85, SET_HOME_POSITION)                          \
    X(  244,  95, MESSAGE_INTERVAL)                           \
    X(  245, 130, EXTENDED_SYS_STATE)                         \
    X(  246, 184, ADSB_VEHICLE)                               \
    X(  247,  81, COLLISION)                                  \
    X(  248,   8, V2_EXTENSION)                               \
    X(  249, 204, MEMORY_VECT)                                \
    X(  250,  49, DEBUG_VECT)                                 \
    X(  251, 170, NAMED_VALUE_FLOAT)                          \
    X(  252,  44, NAMED_VALUE_INT)                            \
    X(  253,  83, STATUSTEXT)                                 \
    X(  254,  46, DEBUG)                                      \
    X(  256,  71, SETUP_SIGNING)                              \
    X(  257, 131, BUTTON_CHANGE)                              \
    X(  258, 187, PLAY_TUNE)                                  \
    X(  259,  92, CAMERA_INFORMATION)                         \
    X(  260, 146, CAMERA_SETTINGS)                            \
    X(  261, 179, STORAGE_INFORMATION)                        \
    X(  262,  12, CAMERA_CAPTURE_STATUS)                      \
    X(  263, 133, CAMERA_IMAGE_CAPTURED)                      \
    X(  264,  49, FLIGHT_INFORMATION)                         \
    X(  265,  26, MOUNT_ORIENTATION)                          \
    X(  266, 193, LOGGING_DATA)                               \
    X(  267,  35, LOGGING_DATA_ACKED)                         \
    X(  268,  14, LOGGING_ACK)                                \
    X(  269, 109, VIDEO_STREAM_INFORMATION)                   \
    X(  270,  59, VIDEO_STREAM_STATUS)                        \
    X(  271,  22, CAMERA_FOV_STATUS)                          \
    X(  275, 126, CAMERA_TRACKING_IMAGE_STATUS)               \
    X(  276,  18, CAMERA_TRACKING_GEO_STATUS)                 \
    X(  277,  62, CAMERA_THERMAL_RANGE)                       \
    X(  280,  70, GIMBAL_MANAGER_INFORMATION)                 \
    X(  281,  48, GIMBAL_MANAGER_STATUS)                      \
    X(  282, 123, GIMBAL_MANAGER_SET_ATTITUDE)                \
    X(  283,  74, GIMBAL_DEVICE_INFORMATION)                  \
    X(  284,  99, GIMBAL_DEVICE_SET_ATTITUDE)                 \
    X(  285, 137, GIMBAL_DEVICE_ATTITUDE_STATUS)              \
    X(  286, 210, AUTOPILOT_STATE_FOR_GIMBAL_DEVICE)          \
    X(  287,   1, GIMBAL_MANAGER_SET_PITCHYAW)                \
    X(  288,  20, GIMBAL_MANAGER_SET_MANUAL_CONTROL)          \
    X(  295, 234, AIRSPEED)                                   \
    X(  299,  19, WIFI_CONFIG_AP)                             \
    X(  301, 243, AIS_VESSEL)                                 \
    X(  310,  28, UAVCAN_NODE_STATUS)                         \
    X(  311,  95, UAVCAN_NODE_INFO)                           \
    X(  320, 243, PARAM_EXT_REQUEST_READ)                     \
    X(  321,  88, PARAM_EXT_REQUEST_LIST)                     \
    X(  322, 243, PARAM_EXT_VALUE)                            \
    X(  323,  78, PARAM_EXT_SET)                              \
    X(  324, 132, PARAM_EXT_ACK)                              \
    X(  330,  23, OBSTACLE_DISTANCE)                          \
    X(  331,  91, ODOMETRY)                                   \
    X(  332, 236, TRAJECTORY_REPRESENTATION_WAYPOINTS)        \
    X(  333, 231, TRAJECTORY_REPRESENTATION_BEZIER)           \
    X(  335, 225, ISBD_LINK_STATUS)                           \
    X(  339, 199, RAW_RPM)                                    \
    X(  340,  99, UTM_GLOBAL_POSITION)                        \
    X(  350, 232, DEBUG_FLOAT_ARRAY)                          \
    X(  370,  75, SMART_BATTERY_INFO)                         \
    X(  373, 117, GENERATOR_STATUS)                           \
    X(  375, 251, ACTUATOR_OUTPUT_STATUS)                     \
    X(  376, 199, RELAY_STATUS)                               \
    X(  385, 147, TUNNEL)                                     \
    X(  386, 132, CAN_FRAME)                                  \
    X(  387,   4, CANFD_FRAME)                                \
    X(  388,   8, CAN_FILTER_MODIFY)                          \
    X( 9000, 113, WHEEL_DISTANCE)                             \
    X( 9005, 117, WINCH_STATUS)                               \
    X(10001, 209, UAVIONIX_ADSB_OUT_CFG)                      \
    X(10002, 186, UAVIONIX_ADSB_OUT_DYNAMIC)                  \
    X(10003,   4, UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT)    \
    X(10004, 133, UAVIONIX_ADSB_OUT_CFG_REGISTRATION)         \
    X(10005, 103, UAVIONIX_ADSB_OUT_CFG_FLIGHTID)             \
    X(10006, 193, UAVIONIX_ADSB_GET)                          \
    X(10007,  71, UAVIONIX_ADSB_OUT_CONTROL)                  \
    X(10008, 240, UAVIONIX_ADSB_OUT_STATUS)                   \
    X(10151, 195, LOWEHEISER_GOV_EFI)                         \
    X(11000, 134, DEVICE_OP_READ)                             \
    X(11001,  15, DEVICE_OP_READ_REPLY)                       \
    X(11002, 234, DEVICE_OP_WRITE)                            \
    X(11003,  64, DEVICE_OP_WRITE_REPLY)                      \
    X(11004,  11, SECURE_COMMAND)                             \
    X(11005,  93, SECURE_COMMAND_REPLY)                       \
    X(11010,  46, ADAP_TUNING)                                \
    X(11011, 106, VISION_POSITION_DELTA)                      \
    X(11020, 205, AOA_SSA)                                    \
    X(11030, 144, ESC_TELEMETRY_1_TO_4)                       \
    X(11031, 133, ESC_TELEMETRY_5_TO_8)                       \
    X(11032,  85, ESC_TELEMETRY_9_TO_12)                      \
    X(11033, 195, OSD_PARAM_CONFIG)                           \
    X(11034,  79, OSD_PARAM_CONFIG_REPLY)                     \
    X(11035, 128, OSD_PARAM_SHOW_CONFIG)                      \
    X(11036, 177, OSD_PARAM_SHOW_CONFIG_REPLY)                \
    X(11037, 130, OBSTACLE_DISTANCE_3D)                       \
    X(11038,  47, WATER_DEPTH)                                \
    X(11039, 142, MCU_STATUS)                                 \
    X(11040, 132, ESC_TELEMETRY_13_TO_16)                     \
    X(11041, 208, ESC_TELEMETRY_17_TO_20)                     \
    X(11042, 201, ESC_TELEMETRY_21_TO_24)                     \
    X(11043, 193, ESC_TELEMETRY_25_TO_28)                     \
    X(11044, 189, ESC_TELEMETRY_29_TO_32)                     \
    X(12900, 114, OPEN_DRONE_ID_BASIC_ID)                     \
    X(12901, 254, OPEN_DRONE_ID_LOCATION)                     \
    X(12902, 140, OPEN_DRONE_ID_AUTHENTICATION)               \
    X(12903, 249, OPEN_DRONE_ID_SELF_ID)                      \
    X(12904,  77, OPEN_DRONE_ID_SYSTEM)                       \
    X(12905,  49, OPEN_DRONE_ID_OPERATOR_ID)                  \
    X(12915,  94, OPEN_DRONE_ID_MESSAGE_PACK)                 \
    X(12918, 139, OPEN_DRONE_ID_ARM_STATUS)                   \
    X(12919,   7, OPEN_DRONE_ID_SYSTEM_UPDATE)                \
    X(12920,  20, HYGROMETER_SENSOR)                          \
    X(42000, 227, ICAROUS_HEARTBEAT)                          \
    X(42001, 239, ICAROUS_KINEMATIC_BANDS)                    \
    X(50001, 246, CUBEPILOT_RAW_RC)                           \
    X(50002, 181, HERELINK_VIDEO_STREAM_INFORMATION)          \
    X(50003,  62, HERELINK_TELEM)                             \
    X(50004, 240, CUBEPILOT_FIRMWARE_UPDATE_START)            \
    X(50005, 152, CUBEPILOT_FIRMWARE_UPDATE_RESP)             \
    X(52000,  13, AIRLINK_AUTH)                               \
    X(52001, 239, AIRLINK_AUTH_RESPONSE)                      \

#define X(ID, EXTRA, NAME) MAVLINK_MSG_SLOT_##NAME,

typedef u16 MAVLinkMsgSlot;
enum : u16 {
    MAVLINK_MSG_XLIST
    MAVLINK_MSG_COUNT
};

#undef X

typedef u8 MAVLinkFrameFlag;
enum : u8 {
    MAVLINK_FRAME_FLAG_IS_VERSION_2 = (1 << 0),
    MAVLINK_FRAME_FLAG_IS_SIGNED    = (1 << 1)
};

typedef u8 MAVLinkFrameKind;
enum : u8 {
    MAVLINK_FRAME_KIND_OKAY,
    MAVLINK_FRAME_KIND_BAD_CRC,
    MAVLINK_FRAME_KIND_UNKNOWN_MSG,
    MAVLINK_FRAME_KIND_COUNT
};

typedef u8 MAVLinkField;
enum : u8 {
    MAVLINK_FIELD_INCOMPAT_FLAGS,
    MAVLINK_FIELD_COMPAT_FLAGS,
    MAVLINK_FIELD_SEQ,
    MAVLINK_FIELD_SYS_ID,
    MAVLINK_FIELD_COMP_ID,
    MAVLINK_FIELD_MSG_ID,
    MAVLINK_FIELD_PAYLOAD,
    MAVLINK_FIELD_NONE,
    MAVLINK_FIELD_COUNT
};

// @types______________________________________________________________________
struct MAVLinkFrame {
    u8*              Ptr;
    u32              Size;
    u32              CRCSize;
    u32              MsgID;
    u16              MsgSlot;
    MAVLinkFrameFlag Flag;
    u8               PayloadSize;
    u8               Seq;
    u8               SysID;
    u8               CompID;
};

struct MAVLinkByteFix {
    MAVLinkField Field;
    u8           Original;
};

// @runtime____________________________________________________________________
extern u16 MAVLINK_CRC_TABLE[MAVLINK_CRC_SLICE_COUNT][256];
extern u8 MAVLINK_CRC_REVERSE[256];
extern u16 MAVLINK_MSG_SLOTS[MAVLINK_MSG_SLOTS_COUNT];
extern const u32 MAVLINK_MSG_IDS[MAVLINK_MSG_COUNT];
extern const u8 MAVLINK_MSG_EXTRAS[MAVLINK_MSG_COUNT];
extern const Str8 MAVLINK_MSG_NAMES[MAVLINK_MSG_COUNT];
extern const Str8 MAVLINK_FIELD_NAMES[MAVLINK_FIELD_COUNT];

// @functions__________________________________________________________________
void MAVLinkInit(void);
INTERNAL u32 MAVLinkFrameSizeFromPtr(u8* Ptr);
INTERNAL MAVLinkFrame MAVLinkFrameFromPtr(u8* Ptr);
INTERNAL u16 MAVLinkCRC(u8* Ptr, u64 Size);
INTERNAL u16 MAVLinkExtraFromCRC(u16 CRC, u16 Target);
INTERNAL MAVLinkFrameKind MAVLinkFrameKindFromFrame(MAVLinkFrame* Frame, OUT u16* Extra);
MAVLinkByteFix MAVLinkByteFixFromFrame(MAVLinkFrame* Frame);

#endif // __MAVLINK_H__
