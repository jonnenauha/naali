// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexNetworking_NetMessageManager_h
#define incl_RexNetworking_NetMessageManager_h

#include <list>
#include <set>
#include <boost/shared_ptr.hpp>

#include "NetworkConnection.h"

#include "LLInMessage.h"
#include "LLOutMessage.h"
#include "LLMessage.h"

#include "INetMessageListener.h"
#include "EventHistory.h"

#include "RexNetworkingModuleApi.h"

namespace RexNetworking
{

    /// Manages both in- and outbound UDP communication. Implements a packet queue, packet sequence numbering, ACKing,
    /// pinging, and reliable communications. reX-protocol specific. Used internally by OpenSimProtocolModule, external
    /// module users don't need to work on this.
    class REXNETWORKING_MODULE_API LLMessageManager
    {
    public:
        /// The message manager starts in a disconnected state.
        /// @param The filename to take the message definitions from.
        LLMessageManager(const char *messageListFilename);

        /// Destructor
        ~LLMessageManager();

        /// Connects to the given server.
        bool ConnectTo(const char *serverAddress, int port);
        
        /// Disconnets from the current server.
        void Disconnect();

        /// If connected to a server.
        bool IsConnected();

        /// To start building a new outbound message, call this.
        /// @return An empty message holder where the message can be built.
        LLOutMessage *StartNewMessage(LLMsgID msgId);
        
        /// To tell the manager that building the message is now finished and can be put into the outbound queue, call this.
        void FinishMessage(LLOutMessage *message);
        
        /// Reads in all inbound UDP messages and processes them forward to the application through the listener.
        /// Checks and resends any timed out reliable outbound messages. This could be moved into a separate thread, but not that timing specific so not necessary atm.
        void ProcessMessages();

        /// Interprets the given byte stream as a message and dumps it contents out to the log. Useful only for diagnostics and such.
        void DumpNetworkMessage(LLMsgID id, LLInMessage *msg);

        /// @return The Message Info structure associated with the given message ID.
        const LLMessageInfo *GetMessageInfoByID(LLMsgID id) const;

    #ifndef RELEASE
        void DebugSendHardcodedTestPacket();
        void DebugSendHardcodedRandomPacket(size_t numBytes);
    #endif

        /// Sets the object that receives the network packets. Replaces the old. Currently supports only one listener.
        /// \todo weakptr'ize. \todo delegate/event \todo pub/sub or something else.
        void RegisterNetworkListener(INetMessageListener *listener) { messageListener = listener; }
        void UnregisterNetworkListener(INetMessageListener *listener) { messageListener = 0; }

#ifdef PROFILING
        EventHistory sentDatagrams;
        EventHistory sentDatabytes;
        EventHistory receivedDatagrams;
        EventHistory receivedDatabytes;
        /// A history of occurrences of when a packet has had to be resent.
        EventHistory resentPackets;
        /// A history of occurrences when we assume we have lost an incoming packet (we generate false positives when receiving data out-of-order, but that's not critical)
        EventHistory lostPackets;
        /// A history of occurrences of when we have received a duplicate packet and have discarded it.
        EventHistory duplicatesReceived;
#endif

    private:
        /// Deallocates all memory used for outbound message structs.
        void ClearMessagePoolMemory();
    
        /// @return A new sequence number for outbound UDP messages.
        size_t GetNewSequenceNumber() { return sequenceNumber++; }

        /// Queues acking the packet with the given packetID.
        void QueuePacketACK(uint32_t packetID);
        
        /// Sends pending acks to the server.
        void SendPendingACKs();

        /// Processes a single raw datagram received from the network.
        void HandleInboundBytes(std::vector<uint8_t> &data);

        /// Processes a received PacketAck message.
        void ProcessPacketACK(LLInMessage *msg);
        
        /// Processes a received PacketAck ID.
        void ProcessPacketACK(uint32_t id);
        
        /// Responds to a ping check from the server with a CompletePingCheck message.
        void SendCompletePingCheck(uint8_t pingID);

        /// Called to send out a message that is already binary-mangled to the proper final format. (packet number, zerocoding, flags, ...)
        void SendProcessedMessage(LLOutMessage *msg);

        /// Adds message to the queue of reliable outbound messages.
        void AddMessageToResendQueue(LLOutMessage *msg);
        
        /// Removes message from the queue of reliable outbound messages.
        void RemoveMessageFromResendQueue(uint32_t packetID);
        
        /// @return True, if the resend queue is empty, false otherwise.
        bool ResendQueueIsEmpty() const { return messageResendQueue.empty(); }

        /// Checks each reliable message in outbound queue and resends any of the if an Ack was not received within a time-out period.
        void ProcessResendQueue();

        LLMessageManager(const LLMessageManager &);
        void operator=(const LLMessageManager &);

        /// All incoming UDP packets are routed to this handler.
        INetMessageListener *messageListener;

        /// The socket for the UDP connection.
        boost::shared_ptr<NetworkConnection> connection;

        /// List of messages this manager can handle.
        boost::shared_ptr<LLMessageList> messageList;

        /// A pool of allocated unused LLOutMessage structures. Used to avoid unnecessary allocations at runtime.
        std::list<LLOutMessage*> unusedMessagePool;

        /// A pool of LLOutMessage structures, which have been handed out to the application and are currently being built.
        std::list<LLOutMessage*> usedMessagePool;
        
        /// Packet acks pending to be sent
        std::set<uint32_t> pendingACKs;

        typedef std::list<std::pair<time_t, LLOutMessage*> > MessageResendList;
        /// A pool of LLOutMessages that are in the outbound queue. Need to keep the unacked reliable messages in
        /// memory for possible resending.
        MessageResendList messageResendQueue;
        
        /// A running sequence number for outbound messages.
        size_t sequenceNumber;

        /// The sequence number of the most recent packet we received. Note that this can go up and down if we receive data out of order (or if we receive spoofed data)
        size_t lastReceivedSequenceNumber;
        
        /// A set of received messages' sequence numbers.
        std::set<uint32_t> receivedSequenceNumbers;

        /// Connection status
        bool connected;
    };

}

#endif // incl_RexNetworking_NetMessageManager_h
