// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_INetMessageListener_h
#define incl_RexNetworking_INetMessageListener_h

#include "CoreDefines.h"
#include "CoreModuleApi.h"

namespace RexNetworking
{

    /** \brief Interface that is implemented by an object that is interested in receiving 
        network messages directly from a LLMessageManager object.

        In Rex architecture, the OpenSimProtocolModule
        implements this interface and manages the messaging between other modules.
        Individual module writers should not use this.
        Use LLMessageManager::RegisterNetworkListener to register the listener. */
    class MODULE_API INetMessageListener
    {

    public:
        INetMessageListener() {}
        virtual ~INetMessageListener() {}

        /// Called by LLMessageManager for each network message received. 
        /// The callee can process the inputted message any way he wants.
        /// @param msgID The type of the message.
        /// @param msg The actual message contents.
        virtual void OnMessageReceived(LLMsgID msgID, LLInMessage *msg) = 0;

        /// Called for each network message that is sent. This callback is provided just for 
        /// debugging/stats collecting. \todo Provide a converter object that converts a 
        //  LLOutMessage to a LLMessage base object or a LLInMessage object to allow reading the contents.
        /// @param msg The message contents.
        virtual void OnMessageSent(const LLOutMessage *msg) {}
    };

}

#endif // incl_RexNetworking_INetMessageListener_h
