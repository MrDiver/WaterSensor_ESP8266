#ifndef MQTT_PLANT_HANDLER_H
#define MQTT_PLANT_HANDLER_H

#define MQTT_VERSION 5
#include <PubSubClient.h>

class PlantMqtt
{
private:
    String serverAddress;
    int port;
    String id;
    String topic;
    WiFiClient wifiClient;

    void incomingMqttMessage()
    {
    }

public:
    PubSubClient mqtt;

    PlantMqtt()
    {
        mqtt.setClient(wifiClient);
    }

    bool setup(String serverAddress, int port = 1883, String id = "NONE", String topic = "t/plant/humidity", String username = "mqtt_user", String password = "mqtt_user", String willTopic = "t/plant/humidity", bool willRetain = false, String willMessage = "BATTERY DIED")
    {
        this->serverAddress = serverAddress;
        this->port = port;
        this->id = id;
        this->topic = topic;
        mqtt.setServer(serverAddress.c_str(), port);
        mqtt.setKeepAlive(60 * 60);
        Serial.println("[MQTT] Trying to connect with " + id + " to " + serverAddress + ":" + String(port));
        return mqtt.connect(id.c_str(), username.c_str(), password.c_str(), (willTopic + "/" + id).c_str(), 1, willRetain, willMessage.c_str());
    }

    void publishPlantState(unsigned long state)
    {
        Serial.println("Topic: " + (topic + "/" + id) + "\nMessage: " + String(state));
        bool result = mqtt.publish((topic + "/" + id).c_str(), String(state).c_str(), true);
        if (result)
        {
            Serial.println("Publish Succeeded");
        }
        else
        {
            Serial.println("Publish Failed");
        }
        switch (mqtt.state())
        {
        case MQTT_CONNECTION_TIMEOUT:
            Serial.println("the server didn't respond within the keepalive time");
            break;

        case MQTT_CONNECTION_LOST:
            Serial.println("the network connection was broken");
            break;
        case MQTT_CONNECT_FAILED:
            Serial.println("the network connection failed");
            break;
        case MQTT_DISCONNECTED:
            Serial.println("the client is disconnected cleanly");
            break;
        case MQTT_CONNECTED:
            Serial.println("the client is connected");
            break;
        case MQTT_CONNECT_BAD_PROTOCOL:
            Serial.println("the server doesn't support the requested version of MQTT");
            break;
        case MQTT_CONNECT_BAD_CLIENT_ID:
            Serial.println("the server rejected the client identifier");
            break;

        case MQTT_CONNECT_UNAVAILABLE:
            Serial.println("the server was unable to accept the connection");
            break;
        case MQTT_CONNECT_BAD_CREDENTIALS:
            Serial.println("the username/password were rejected");
            break;
        case MQTT_CONNECT_UNAUTHORIZED:
            Serial.println("the client was not authorized to connect");

        default:
            break;
        }
    }
};

#endif