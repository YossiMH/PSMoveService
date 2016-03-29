#ifndef DEVICE_INTERFACE_H
#define DEVICE_INTERFACE_H

#include <string>

struct CommonDeviceVector
{
    float i, j, k;

    inline void clear()
    {
        i = j = k = 0.f;
    }
};

struct CommonDevicePosition
{
    float x, y, z;

    inline void clear()
    {
        x = y = z = 0.f;
    }
};

struct CommonDeviceQuaternion
{
    float x, y, z, w;

    inline void clear()
    {
        x = y = z = 0.f;
        w = 0.f;
    }
};

struct CommonDevicePose
{
    CommonDevicePosition Position;
    CommonDeviceQuaternion Orientation;

    void clear()
    {
        Position.clear();
        Orientation.clear();
    }
};

struct CommonDeviceState
{
    enum eDeviceClass
    {
        Controller = 0x00,
        TrackingCamera = 0x10,
        HeadMountedDisplay = 0x20
    };
    
    enum eDeviceType
    {
        PSMove = Controller + 0x00,
        PSNavi = Controller + 0x01,
        SUPPORTED_CONTROLLER_TYPE_COUNT = Controller + 0x02,
        
        PS3EYE = TrackingCamera + 0x00,
        SUPPORTED_CAMERA_TYPE_COUNT = TrackingCamera + 0x01,

        OculusHMD = HeadMountedDisplay + 0x00,
        OculusDK2 = OculusHMD,
        SUPPORTED_HMD_TYPE_COUNT = HeadMountedDisplay + 0x01
    };
    
    eDeviceType DeviceType;
    int PollSequenceNumber;
    
    inline CommonDeviceState()
    {
        clear();
    }
    
    inline void clear()
    {
        DeviceType= SUPPORTED_CONTROLLER_TYPE_COUNT; // invalid
        PollSequenceNumber= 0;
    }

    static const char *getDeviceTypeString(eDeviceType device_type)
    {
        const char *result = nullptr;

        switch (device_type)
        {
        case PSMove:
            result= "PSMove";
            break;
        case PSNavi:
            result = "PSNavi";
            break;
        case PS3EYE:
            result = "PSEYE";
            break;
        case OculusDK2:
            result = "Oculus DK2";
            break;
        default:
            result = "UNKNOWN";
        }

        return result;
    }
};

struct CommonControllerState : CommonDeviceState
{
    enum ButtonState {
        Button_UP = 0x00,       // (00b) Not pressed
        Button_PRESSED = 0x01,  // (01b) Down for one frame only
        Button_DOWN = 0x03,     // (11b) Down for >1 frame
        Button_RELEASED = 0x02, // (10b) Up for one frame only
    };

    enum BatteryLevel {
        Batt_MIN = 0x00, /*!< Battery is almost empty (< 20%) */
        Batt_20Percent = 0x01, /*!< Battery has at least 20% remaining */
        Batt_40Percent = 0x02, /*!< Battery has at least 40% remaining */
        Batt_60Percent = 0x03, /*!< Battery has at least 60% remaining */
        Batt_80Percent = 0x04, /*!< Battery has at least 80% remaining */
        Batt_MAX = 0x05, /*!< Battery is fully charged (not on charger) */
        Batt_CHARGING = 0xEE, /*!< Battery is currently being charged */
        Batt_CHARGING_DONE = 0xEF, /*!< Battery is fully charged (on charger) */
    };

    enum BatteryLevel Battery;
    unsigned int AllButtons;                    // all-buttons, used to detect changes

    //TODO: high-precision timestamp. Need to do in hidapi?
    
    inline CommonControllerState()
    {
        clear();
    }

    inline void clear()
    {
        CommonDeviceState::clear();
        DeviceType= SUPPORTED_CONTROLLER_TYPE_COUNT; // invalid
        Battery= Batt_MAX;
        AllButtons= 0;
    }
};

struct CommonHMDState : CommonDeviceState
{
    CommonDevicePose Pose;

    inline CommonHMDState()
    {
        clear();
    }

    inline void clear()
    {
        CommonDeviceState::clear();

        Pose.clear();
    }
};


/// Abstract base class for any device interface. Further defined in specific device abstractions.
class IDeviceInterface
{
public:
    enum ePollResult
    {
        _PollResultSuccessNoData,
        _PollResultSuccessNewData,
        _PollResultFailure,
    };
    
    // Return true if device path matches
    virtual bool matchesDeviceEnumerator(const class DeviceEnumerator *enumerator) const = 0;
    
    // Opens the HID device for the device at the given enumerator
    virtual bool open(const class DeviceEnumerator *enumerator) = 0;
    
    // Returns true if hidapi opened successfully
    virtual bool getIsOpen() const  = 0;
    
    virtual bool getIsReadyToPoll() const = 0;
    
    // Polls for new device data
    virtual ePollResult poll() = 0;
    
    // Closes the HID device for the device
    virtual void close() = 0;
    
    // Get the number of milliseconds we're willing to accept no data from the device before we disconnect it
    virtual long getMaxPollFailureCount() const = 0;
    
    // Returns what type of device
    virtual CommonDeviceState::eDeviceType getDeviceType() const = 0;
    
    // Fetch the device state at the given sample index.
    // A lookBack of 0 corresponds to the most recent data.
    virtual const CommonDeviceState * getState(int lookBack = 0) const = 0;
};

/// Abstract class for controller interface. Implemented in PSMoveController.cpp
class IControllerInterface : public IDeviceInterface
{
public:
    // Sets the address of the bluetooth adapter on the host PC with the controller
    virtual bool setHostBluetoothAddress(const std::string &address) = 0;

    // -- Getters
    // Returns true if the device is connected via Bluetooth, false if by USB
    virtual bool getIsBluetooth() const = 0;

    // Returns the full usb device path for the controller
    virtual std::string getUSBDevicePath() const = 0;

    // Gets the bluetooth address of the adapter on the host PC that's registered with the controller
    virtual std::string getHostBluetoothAddress() const = 0;

    // Returns the serial number for the controller
    virtual std::string getSerial() const  = 0;
};

/// Abstract class for Tracker interface. Implemented Tracker classes
class ITrackerInterface : public IDeviceInterface
{
public:
    enum eDriverType
    {
        Libusb,
        CL,
        CLMulti,
        Generic_Webcam,

        SUPPORTED_DRIVER_TYPE_COUNT,
    };

    // -- Getters
    // Returns the driver type being used by this camera
    virtual eDriverType getDriverType() const = 0;

    // Returns the full usb device path for the tracker
    virtual std::string getUSBDevicePath() const = 0;

    // Returns the video frame size (used to compute frame buffer size)
    virtual bool getVideoFrameDimensions(int *out_width, int *out_height, int *out_stride) const = 0;

    // Returns a pointer to the last video frame buffer captured
    virtual const unsigned char *getVideoFrameBuffer() const = 0;

    static const char *getDriverTypeString(eDriverType device_type)
    {
        const char *result = nullptr;

        switch (device_type)
        {
        case Libusb:
            result = "Libusb";
            break;
        case CL:
            result = "CL";
            break;
        case CLMulti:
            result = "CLMulti";
            break;
        case Generic_Webcam:
            result = "Generic_Webcam";
            break;
        default:
            result = "UNKNOWN";
        }

        return result;
    }
};

/// Abstract class for HMD interface. Implemented HMD classes
class IHMDInterface : public IDeviceInterface
{
public:
    // -- Getters
    // Returns the full usb device path for the HMD
    virtual std::string getUSBDevicePath() const = 0;
};

#endif // DEVICE_INTERFACE_H