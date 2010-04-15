// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_NetworkEvents_h
#define incl_RexNetworking_NetworkEvents_h

#include "EventDataInterface.h"

namespace RexNetworking
{
    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    /// \ingroup OpenSimProtocolClient 
    namespace Events
    {
        /**
         * Notify event which is sent when OpenSimProtocol module has disconnect connection between server and client. 
         * @note this event should never send from outside module. Use DisconnectFromServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_DISCONNECTED = 0x01;

        /** 
         * Notify event which is send when connection is made to the server. 
         * @note this event should never send from outside module. Use ConnectToServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_CONNECTED = 0x02;

        /**
         * Notify event which can be send when connection has failed. 
         * @note this event should never send from outside module. 
         */
        static const event_id_t EVENT_CONNECTION_FAILED = 0x03;

        /**
         * Notifies when a ProtocolModule has registered networking category
         * so other modules can query the event category
         */
        static const event_id_t EVENT_NETWORKING_REGISTERED = 0x04;
    }
}

#endif // incl_Protocol_NetworkEvents_h
