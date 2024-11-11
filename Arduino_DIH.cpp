/*
  Arduino Library for Device Interaction Hub (DIH)
	
  Copyright (c) 2024 Roman Divotkey.
  See the LICENSE file for more details.
  
  This program is provided "as is" without any warranties or guarantees.
  Use at your own risk.
*/

#include "Arduino_DIH.h"

Arduino_DIH::Arduino_DIH(Stream &stream) : _stream(stream) {
    _connectionState = DISCONNECTED;
    _retryInterval = 5000; 
	
    _dataCallback = nullptr;
    _commandCallback = nullptr;	
    _heartbeatInterval = 10000; 
	_heartbeatAckTimeout = 5000; 
    _lastHeartbeatTime = 0;
	_lastHeartbeatSentTime = 0;
	
    _isRegistered = false;
	_awaitingHeartbeatAck = false;
	
    _lastAttemptTime = 0;
}

void Arduino_DIH::onData(DataCallback callback) {
    _dataCallback = callback;
}

void Arduino_DIH::onCommand(CommandCallback callback) {
    _commandCallback = callback;
}

void Arduino_DIH::begin(unsigned long baudRate) {
    if (&_stream == &Serial) {
        Serial.begin(baudRate);
    }
    // Further streams can be initialized here
}

void Arduino_DIH::setDeviceID(const String& deviceID) {
    _deviceID = deviceID;
}

bool Arduino_DIH::isRegistered() {
    return _isRegistered;
}

int Arduino_DIH::getState() {
	return _connectionState;
}

void Arduino_DIH::registerDevice(const String& deviceID) {
    _deviceID = deviceID;
    StaticJsonDocument<256> doc;
    doc["type"] = "register";
    doc["device_id"] = _deviceID;
    String messageID = String(millis());
    doc["message_id"] = messageID;
    sendMessage(doc.as<JsonObject>());
    _lastAttemptTime = millis();
    _lastRegisterMessageID = messageID;
}


void Arduino_DIH::deregisterDevice() {
    StaticJsonDocument<200> doc;
    doc["type"] = "deregister";
    doc["device_id"] = _deviceID;
    doc["message_id"] = String(millis());
    sendMessage(doc.as<JsonObject>());
    _isRegistered = false;
}

void Arduino_DIH::sendData(const String& key, const String& value) {
    if (!_isRegistered) return;
    StaticJsonDocument<256> doc;
    doc["type"] = "data";
    doc["device_id"] = _deviceID;
    doc["message_id"] = String(millis());
    doc["payload"][key] = value;
    sendMessage(doc.as<JsonObject>());
    // Optional: Implement wait for ACK
}

void Arduino_DIH::loop() {
    unsigned long currentTime = millis();

    // Message processing
    while (_stream.available()) {
        String message = _stream.readStringUntil('\n');
        processIncomingMessage(message);
    }

    // Connection management
    switch (_connectionState) {
        case DISCONNECTED:
            if (currentTime - _lastAttemptTime >= _retryInterval) {
                registerDevice(_deviceID); // Attempt to register
                _lastAttemptTime = currentTime;
                _connectionState = REGISTERING;
            }
            break;
        case REGISTERING:
            if (currentTime - _lastAttemptTime >= _retryInterval) {
                // No ACK received, try again
                _connectionState = DISCONNECTED;
            }
            break;
        case REGISTERED:
            if (_awaitingHeartbeatAck) {
                if (currentTime - _lastHeartbeatSentTime >= _heartbeatAckTimeout) {
                    // No ACK received for heartbeat, connection lost
                    _connectionState = DISCONNECTED;
                    _isRegistered = false;
                    _awaitingHeartbeatAck = false;
                }
            } else {
                if (currentTime - _lastHeartbeatSentTime >= _heartbeatInterval) {
                    sendHeartbeat();
                    // _lastHeartbeatTime is updated when ACK is received
                }
            }
            break;
    }
}


void Arduino_DIH::processIncomingMessage(const String& message) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        // TODO: Error handling
        return;
    }
    String type = doc["type"];
    if (type == "ack") {
        handleAck(doc.as<JsonObject>());
    } else if (type == "nack") {
        handleNack(doc.as<JsonObject>());
    } else if (type == "data") {
        handleData(doc.as<JsonObject>());
    } else if (type == "command") {
        handleCommand(doc.as<JsonObject>());
    } else {
        // Unknown message type
    }
}


void Arduino_DIH::handleAck(const JsonObject& json) {
    String messageID = json["message_id"];
    if (_connectionState == REGISTERING && messageID == _lastRegisterMessageID) {
        _connectionState = REGISTERED;
        _isRegistered = true;
        _awaitingHeartbeatAck = false;
        _lastHeartbeatTime = millis();
    } else if (_awaitingHeartbeatAck && messageID == _lastHeartbeatMessageID) {
        // ACK received for Heartbeat
        _awaitingHeartbeatAck = false;
        _lastHeartbeatTime = millis();
    }
    // Additional ACK processing
}

void Arduino_DIH::handleNack(const JsonObject& json) {
    // NACK processing
    _connectionState = DISCONNECTED;
    _isRegistered = false;
}

void Arduino_DIH::sendHeartbeat() {
    StaticJsonDocument<200> doc;
    doc["type"] = "heartbeat";
    doc["device_id"] = _deviceID;
    String messageID = String(millis());
    doc["message_id"] = messageID;
    sendMessage(doc.as<JsonObject>());

    // Save that we are waiting for an ACK
    _awaitingHeartbeatAck = true;
    _lastHeartbeatSentTime = millis();
    _lastHeartbeatMessageID = messageID;
}

void Arduino_DIH::sendMessage(const JsonObject& json) {
    String output;
    serializeJson(json, output);
    _stream.println(output);
    // Optional: Save message in queue to resend if ACK is missing
}


void Arduino_DIH::handleData(const JsonObject& json) {
    if (_dataCallback) {
        JsonObject payload = json["payload"].as<JsonObject>();
        _dataCallback(payload);
    }
    // Optional: Send ACK
    String messageID = json["message_id"];
    StaticJsonDocument<128> ackDoc;
    ackDoc["type"] = "ack";
    ackDoc["message_id"] = messageID;
    sendMessage(ackDoc.as<JsonObject>());
}

void Arduino_DIH::handleCommand(const JsonObject& json) {
    if (_commandCallback) {
        JsonObject payload = json["payload"].as<JsonObject>();
        _commandCallback(payload);
    }
    // Optional: Send ACK
	/*
    String messageID = json["message_id"];
    StaticJsonDocument<128> ackDoc;
    ackDoc["type"] = "ack";
    ackDoc["message_id"] = messageID;
    sendMessage(ackDoc.as<JsonObject>());
	*/
}
