// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_NetworkEvents_h
#define incl_Protocol_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "LLMessageManager/LLMessage.h"

#include <boost/smart_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace RexNetworking
{
    class LLInMessage;
    class LLOutMessage;

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

    /// Event data interface for authentication type identification.
    /// \ingroup OpenSimProtocolClient
    class AuthenticationEventData : public Foundation::EventDataInterface
    {
    public:
        AuthenticationEventData(const AuthenticationType &auth_type, const std::string &identity_url = "", const std::string &host_Url = "") 
            : type(auth_type), identityUrl(identity_url), hostUrl(host_Url) {}
        virtual ~AuthenticationEventData() {}
        void SetIdentity(const std::string &url) { identityUrl = url; }
        void SetHost(const std::string &url) { hostUrl = url; }
        AuthenticationType type;
        std::string identityUrl;
        std::string hostUrl;
    };

    /// Event data interface for inbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventInboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventInboundData(LLMsgID id, LLInMessage *msg) :
            message(msg), messageID(id) {}
        virtual ~NetworkEventInboundData() {}

        LLMsgID messageID;
        LLInMessage *message;
    };

    /// Event data interface for outbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventOutboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventOutboundData(LLMsgID id, const LLOutMessage *msg) :
            message(msg), messageID(id) {}
        virtual ~NetworkEventOutboundData() {}

        LLMsgID messageID;
        const LLOutMessage *message;
    };
}

#endif // incl_Protocol_NetworkEvents_h
