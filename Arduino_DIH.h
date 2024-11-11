/*
  Arduino Library for Device Interaction Hub (DIH)
	
  Copyright (c) 2024 Roman Divotkey.
  See the LICENSE file for more details.
  
  This program is provided "as is" without any warranties or guarantees.
  Use at your own risk.
*/

#ifndef Arduino_DIH_h
#define Arduino_DIH_h

#include <Arduino.h>
#include <ArduinoJson.h>

class Arduino_DIH {
public:

    enum ConnectionState {
        DISCONNECTED,
        REGISTERING,
        REGISTERED
    };

	// Callback-Typen
    typedef void (*DataCallback)(const JsonObject& payload);
    typedef void (*CommandCallback)(const JsonObject& payload);

    Arduino_DIH(Stream &stream);
	
    void begin(unsigned long baudRate);
    void setDeviceID(const String& deviceID);
    void registerDevice(const String& deviceID);
    void deregisterDevice();
    void sendData(const String& key, const String& value);
    void loop();
    bool isRegistered(); 
	int getState();
	
    // Methods for setting the callback functions
    void onData(DataCallback callback);
    void onCommand(CommandCallback callback);
	

private:

	// Callback-Funktionen
    DataCallback _dataCallback;
    CommandCallback _commandCallback;	
    ConnectionState _connectionState;
	
    unsigned long _lastAttemptTime;
    unsigned long _retryInterval;
    unsigned long _lastHeartbeatTime;
    unsigned long _heartbeatInterval;
    Stream &_stream;
    String _deviceID;
    bool _isRegistered;
	bool _awaitingHeartbeatAck;
    unsigned long _heartbeatAckTimeout;
    unsigned long _lastHeartbeatSentTime;
    String _lastHeartbeatMessageID;
    String _lastRegisterMessageID;
	
    void sendHeartbeat();
    void handleAck(const JsonObject& json);
    void handleNack(const JsonObject& json);
    void processIncomingMessage(const String& message);
    void sendMessage(const JsonObject& json);
	
	// Methods for processing data and commands
    void handleData(const JsonObject& json);
    void handleCommand(const JsonObject& json);
};

#endif
