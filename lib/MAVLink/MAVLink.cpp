#if defined(MAVLINK_ENABLED)
#include <Arduino.h>
#include "MAVLink.h"
#include <config.h>

// Returns whether the message is a control message, i.e. a message where we don't want to
// pass the message to the flight controller, but rather handle it ourselves. If the message
// is a control message, the function handles it internally.
bool MAVLink::handleControlMessage(mavlink_message_t *msg)
{
    bool shouldForward = true;
    // Check for messages addressed to the Backpack
    switch (msg->msgid)
    {
    case MAVLINK_MSG_ID_COMMAND_INT:
        mavlink_command_int_t commandMsg;
        mavlink_msg_command_int_decode(msg, &commandMsg);
        if (commandMsg.target_component == MAV_COMP_ID_UDP_BRIDGE)
        {
            shouldForward = false;
            constexpr uint8_t ELRS_MODE_CHANGE = 0x8;
            switch (commandMsg.command)
            {
            case MAV_CMD_USER_1:
                switch ((int)commandMsg.param1)
                {
                case ELRS_MODE_CHANGE:
                    switch ((int)commandMsg.param2)
                    {
                    case 0: // TX_NORMAL_MODE
                        config.SetStartWiFiOnBoot(false);
                        ESP.restart();
                        break;
                    case 1: // TX_MAVLINK_MODE
                        if (config.GetWiFiService() != WIFI_SERVICE_MAVLINK_TX)
                        {
                            config.SetWiFiService(WIFI_SERVICE_MAVLINK_TX);
                            config.SetStartWiFiOnBoot(true);
                            config.Commit();
                            ESP.restart();
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
    return shouldForward;
}
#endif