// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_NetworkEvents_h
#define incl_RexNetworking_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"

namespace RexNetworking
{
    class LLStream;

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

    class LLStreamReadyEvent : public Foundation::EventDataInterface
    {
        public:
            explicit LLStreamReadyEvent(LLStream *stream);
            virtual ~LLStreamReadyEvent();
            
            LLStream *stream;
    };

    /// Event data interface for EVENT_USER_CONNECTED and EVENT_USER_DISCONNECTED
    class UserConnectivityEvent : public Foundation::EventDataInterface
    {
    public:
        /// Constructor
        /// @param agent_id Agent ID of the user.
        explicit UserConnectivityEvent(const RexUUID &agent_id) : agentId(agent_id) {}
        /// Agent ID.
        RexUUID agentId;
        /// Local ID.
        int32_t localId;
        /// Name.
        std::string fullName;
    };
}

#endif // incl_Protocol_NetworkEvents_h
