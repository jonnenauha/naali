// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_LLStream_h
#define incl_RexNetworking_LLStream_h

#include "ModuleLoggingFunctions.h"
#include "Vector3D.h"
#include "Quaternion.h"

#include "StreamInterface.h"
#include "LLMessageManager/LLMessageManager.h"
#include "LLParameters.h"

#include <QObject>
#include <tr1/functional>
using std::tr1::function;

namespace RexNetworking
{
    /// Struct for object name update
    struct ObjectNameInfo
    {
        entity_id_t local_id;
        std::string name;
    };

    /// Struct for multipleobject update
    struct ObjectUpdateInfo
    {
        entity_id_t local_id;
        Vector3df position;
        Quaternion orientation;
        Vector3df scale;
    };

    /// Struct for description update
    struct ObjectDescriptionInfo
    {
        entity_id_t local_id;
        std::string description;
    };

    typedef std::map <LLMsgID, function <void (LLInMessage*)> > MessageHandlerMap;

    class LLStream : 
        public QObject, 
        public Foundation::StreamInterface, 
        public INetMessageListener
    {
        Q_OBJECT

        public:

            //=================================================================
            // Logging

            MODULE_LOGGING_FUNCTIONS;

            /// Name used for logging.
            static const std::string &logname;

            //! returns name of this module. Needed for logging.
            static const std::string &NameStatic() { return logname; }

            //=================================================================
            // Constructors

            //! Constructed without required parameters
            LLStream ();

            //! Constructed from parameters discovered during session establishment
            LLStream (const LLStreamParameters &params);

            //! Destructor
            virtual ~LLStream();

            //=================================================================
            // Methods for receiving messages

        public:
            //! Register message handlers by id
            virtual void RegisterHandler (LLMsgID id, function <void (LLInMessage*)> handler);

            //! Receives messages from LLMessageManager
            virtual void OnMessageReceived (LLMsgID id, LLInMessage *msg);

            //! Called when connected
            virtual void OnConnect ();

            //! Called when disconnected
            virtual void OnDisconnect ();

            //! TODO
            virtual void OnRegionHandshake (LLInMessage* msg);

            //! TODO
            virtual void OnAgentMovementComplete (LLInMessage* msg);

            //! TODO
            virtual void OnAvatarAnimation (LLInMessage* msg);

            //! TODO
            virtual void OnGenericMessage (LLInMessage* msg);

            //! TODO
            virtual void OnLogoutReply (LLInMessage* msg);

            //! TODO
            virtual void OnImprovedTerseObjectUpdate (LLInMessage* msg);

            //! TODO
            virtual void OnKillObject (LLInMessage* msg);

            //! TODO
            virtual void OnObjectUpdate (LLInMessage* msg);

            //! TODO
            virtual void OnObjectProperties (LLInMessage* msg);

            //! TODO
            virtual void OnAttachedSound (LLInMessage* msg);

            //! TODO
            virtual void OnAttachedSoundGainChange (LLInMessage* msg);

            //! TODO
            virtual void OnSoundTrigger (LLInMessage* msg);

            //! TODO
            virtual void OnPreloadSound (LLInMessage* msg);

            //! TODO
            virtual void OnScriptDialog (LLInMessage* msg);

            //=================================================================
            // Methods for stream control

        public slots:
            /// Establishes a real-time stream connect using LLUDP
            virtual bool Connect (std::string address, int port);

            /// Disconnects the stream
            virtual bool Disconnect ();

            /// @return True if the client connected to a server.
            virtual bool IsConnected () const;

            /// Process LLMessageManager
            virtual void Pump ();

            /// Set require stream parameters
            void SetParameters (const LLStreamParameters &params);

            //=================================================================
            // Methods for sending messages

            /// Send the UDP chat packet.
            void SendChatFromViewerPacket(const std::string &text, s32 channel = 0);

            /// Sends the first UDP packet to open up the circuit with the server.
            void SendUseCircuitCodePacket();

            /// Signals that agent is coming into the region. The region should be expecting 
            /// the agent. Server starts to send object updates etc after it has received 
            /// this packet.
            void SendCompleteAgentMovementPacket();

            /// Sends wearables request to the server
            /// In reX mode, this causes the server to send the avatar appearance address
            void SendAgentWearablesRequestPacket();

            /// Tells client bandwidth to the server
            /// \todo make configurable or measure, now a fixed value
            void SendAgentThrottlePacket();

            /// Sends a RexStartup state generic message
            void SendRexStartupPacket(const std::string& state);

            /// Sends a message requesting logout from the server. The server is then going 
            /// to flood us with some inventory UUIDs after that, but we'll be ignoring those.
            void SendLogoutRequestPacket();

            /// Sends a message which creates a default prim the in world.
            /// @param position Position in the world.
            void SendObjectAddPacket(const RexTypes::Vector3 &position);

            /// Sends a message which requests object removal.
            /// @param local_id Local ID.
            /// @param force God trying to force delete.
            void SendObjectDeletePacket(const uint32_t &local_id, const bool &force = false);

            /// Sends a message which requests object removal.
            /// @param local_id_list List of Local ID's.
            /// @param force God trying to force delete.
            void SendObjectDeletePacket
                (const std::vector<uint32_t> &local_id_list, 
                 const bool &force = false);

            // Sends the basic movement message
            void SendAgentUpdatePacket
                (Quaternion bodyrot,
                 Quaternion headrot,
                 uint8_t state,
                 RexTypes::Vector3 camcenter,
                 RexTypes::Vector3 camataxis,
                 RexTypes::Vector3 camleftaxis,
                 RexTypes::Vector3 camupaxis,
                 float far,
                 uint32_t controlflags,
                 uint8_t flags);

            /// Sends a packet which indicates selection of a group of prims.
            /// @param Local ID of the object which is selected.
            void SendObjectSelectPacket (const unsigned int object_id);

            /// Sends a packet which indicates selection of a prim.
            /// @param List of local ID's of objects which are selected.
            void SendObjectSelectPacket (std::vector<entity_id_t> list);

            /// Sends a packet which indicates deselection of prim(s).
            /// @param Local ID of the object which is deselected.
            void SendObjectDeselectPacket(entity_id_t object_id);

            /// Sends a packet which indicates deselection of a group of prims.
            /// @param List of local ID's of objects which are deselected.
            void SendObjectDeselectPacket(std::vector<entity_id_t> list);

            /// Sends a packet indicating change in Object's position, rotation and scale.
            /// @param List of updated entity id's/pos/rot/scale
            void SendMultipleObjectUpdatePacket(const std::vector<ObjectUpdateInfo>& list);

            /// Sends a packet indicating change in Object's name.
            /// @param List of updated entity ids/names
            void SendObjectNamePacket(const std::vector<ObjectNameInfo>& list);

            /// Sends a packet which indicates object has been touched.
            /// @param Local ID of the object which has been touched.
            void SendObjectGrabPacket(entity_id_t object_id);

            /// Sends a packet indicating change in Object's description
            /// @param List of updated entity pointers.
            void SendObjectDescriptionPacket (const std::vector<ObjectDescriptionInfo>& list);

            /// Sends handshake reply packet
            void SendRegionHandshakeReplyPacket
                (RexUUID agent_id, 
                 RexUUID session_id, 
                 uint32_t flags);

            /// Sends hardcoded agentappearance packet
            void SendAgentSetAppearancePacket();

            /** Sends packet which modifies the world terrain.
             *  @param x World x position
             *  @param y World y position
             *  @param brush Brush size (small = 0, medium = 1, large = 2)
             *  @param action Modify land action type (flatten = 0, raise = 1, lower = 2, 
             *  smooth = 3, roughen = 4, revert = 5)
             *  @param seconds How long terrain has been modified on viewer (delta time).
             *  @param height Previous height value on spesific world position.
             */
            void SendModifyLandPacket
                (f32 x, f32 y, 
                 u8 brush, u8 action, 
                 Real seconds, Real height);

            /** Send a new terrain texture that we want to use.
             *  @param new_texture_id id for asset resouce that we want to use as our 
             *  terrain texture.
             *  @param texture_index switch texture we want to change currently supports 4 
             *  different textures. (0 = lowest and 3 = highest)
             */
            void SendTextureDetail
                (const RexTypes::RexAssetID &new_texture_id, 
                 uint texture_index);

            /** Sends EstateOwnerMessage that will inculde data of terrain texture height, 
             * range and corner.
             *  @param start_height height value where texture start to show (meters).
             *  @param height_range how much up texture will go from the 
             *  texture_start_height (meters)
             *  @param corner what corner will the texture be used (0 = SW, 1 = NW, 
             *  2 = SE and 3 = NE)
             *          Note: in Rex this variable will only tell what texture height values 
             *          we are changing.
             */
            void SendTextureHeightsMessage
                (Real start_height, 
                 Real height_range, 
                 uint corner);

            /// Request new region information from the server(RegionHandshake is sented 
            /// every client on that server)
            /// ReqionHandshakeMessage will contain all new information about spesific region 
            /// (e.g. new TerrainBase/TerrainDetail textures,
            /// terrain texture startheights/ranges and WaterHeight)
            void SendTextureCommitMessage();

            /** Sends a packet which creates a new inventory folder.
             *  @param parent_id The parent folder UUID.
             *  @param folder_id UUID of the folder.
             *  @param type Asset type for the folder.
             *  @param name Name of the folder.
             */
            void SendCreateInventoryFolderPacket
                (const RexUUID &parent_id,
                 const RexUUID &folder_id,
                 const asset_type_t &type,
                 const std::string &name);

            /** Sends a which moves inventory folder and its contains to other folder.
             *  Used when deleting/removing folders to the Trash folder.
             *  @param parent_id The parent folder (folder where we want to move another 
             *  folder) UUID.
             *  @param folder_id UUID of the folder to be moved.
             *  @param re_timestamp Should the server re-timestamp children.
             */
            void SendMoveInventoryFolderPacket
                (const RexUUID &folder_id,
                 const RexUUID &parent_id,
                 const bool &re_timestamp = true);

            /** Sends a packet which deletes inventory folder.
             *  @param folders List of new folders.
             *  @param re_timestamp Should the server re-timestamp children.
             */
            //void SendMoveInventoryFolderPacket(
            //    std::list<ProtocolUtilities::InventoryFolderSkeleton *> folders,
            //    const bool &re_timestamp = true);

            /// Send a packet which deletes inventory folder.
            /// Works when to folder is in the Trash folder.
            /// @param folder_id Folder ID.
            void SendRemoveInventoryFolderPacket(const RexUUID &folder_id);

            /// Send a packet which deletes inventory folders.
            /// Works when to folder is in the Trash folder.
            /// @param folders List of folders to be deleted.
            //void SendRemoveInventoryFolderPacket(
            //    std::list<ProtocolUtilities::InventoryFolderSkeleton *> folders);

            /// Sends packet which moves an inventory item to another folder within 
            /// My Inventory.
            /// @param item_id ID of the item to be moved.
            /// @param folder_id ID of the destionation folder.
            /// @param new_name New name for the item. Can be the same as before.
            /// @param re_timestamp Should the server re-timestamp children.
            void SendMoveInventoryItemPacket
                (const RexUUID &item_id,
                 const RexUUID &folder_id,
                 const std::string &new_name,
                 const bool &re_timestamp = true);

            /// Sends packet which requests an inventory item copy.
            /// @param old_agent_id Agent ID.
            /// @param old_item_id Item ID.
            /// @param new_folder_id Destionation folder ID.
            /// @param new_name New name for the item. Can be the same as before.
            void SendCopyInventoryItemPacket
                (const RexUUID &old_agent_id,
                 const RexUUID &old_item_id,
                 const RexUUID &new_folder_id,
                 const std::string &new_name);

            /// Sends packet which moves an inventory item to another folder.
            /// @param item_id ID of the item to be moved.
            void SendRemoveInventoryItemPacket(const RexUUID &item_id);

            /// Sends packet which moves an inventory item to another folder.
            /// @param item_id_list List of ID's of the items to be removed.
            void SendRemoveInventoryItemPacket(std::list<RexUUID> item_id_list);

            /// Sends packet which modifies inventory folder's name and/or type.
            /// @param parent_id The parent folder ID.
            /// @param folder_id Folder ID.
            /// @param type New type.
            /// @param name New name.
            void SendUpdateInventoryFolderPacket
                (const RexUUID &folder_id,
                 const RexUUID &parent_id,
                 const int8_t &type,
                 const std::string &name);

            /// Sends packet which modifies inventory item's name, description, type etc.
            /// @param item_id Item ID.
            /// @param folder_id Folder ID.
            /// @param asset_type Asset type.
            /// @param inventory_type Inventory type.
            /// @param name Name.
            /// @param description Description.
            void SendUpdateInventoryItemPacket
                (const RexUUID &item_id,
                 const RexUUID &folder_id,
                 const asset_type_t &asset_type,
                 const inventory_type_t &inventory_type,
                 const std::string &name,
                 const std::string &description);

            /** Sends a packet requesting contents of a inventory folder.
             *  @param folder_id Folder UUID.
             *  @param owner_id Owner UUID. If null, we use agent ID.
             *  @param sort_order Sort order, 0 = name, 1 = time.
             *  @param fetch_folders False will omit folders in query.
             *  @param fetch_items False will omit items in query.
             */
            void SendFetchInventoryDescendentsPacket
                (const RexUUID &folder_id,
                 const RexUUID &owner_id = RexUUID(),
                 const int32_t &sort_order = 0,
                 const bool &fetch_folders = true,
                 const bool &fetch_items = true);

            /**
             *  Send a packet to Opensim server to accept friend request
             *  @param transaction_id Unknown
             *  @param folder_id Folder for calling card for this friend
             *  @todo Find out the meaning of transaction_id argument
             */
            void SendAcceptFriendshipPacket
                (const RexUUID &transaction_id, 
                 const RexUUID &folder_id);

            /**
             *  Send a packet to Opensim server to decline received friend request
             *  @param transaction_id Unknown
             *  @todo Find out the meaning of transaction_id argument
             */
            void SendDeclineFriendshipPacket(const RexUUID &transaction_id);

            /**
             *  Send friend request to Opensim server
             *  @param dest_id Target id
             */
            void SendFormFriendshipPacket(const RexUUID &dest_id);

            /**
             *  Sends a packet to remove friend from friend list.
             *  @param other_id Unknown
             *  @todo Find out meaning of the other_id argument
             */
            void SendTerminateFriendshipPacket(const RexUUID &other_id);

            /**
             *
             *
             */
            void SendImprovedInstantMessagePacket
                (const RexUUID &target, 
                 const std::string &text);

            //! Sends a generic message
            /*! \param method Method name
              \param strings Vector of data strings to be sent
              */
            void SendGenericMessage(const std::string& method, const StringVector& strings);

            //a qt version for pythonqt & qtscript
            void SendGenericMessage(QString method, QStringList& strings);

            //! Sends a generic message with binary data packed to the end
            /*! \param method Method name
              \param strings Vector of data strings to be sent
              \param binary Vector of binary data. Will be split into smaller chunks as necessary
              */
            void SendGenericMessageBinary
                (const std::string& method,
                 const StringVector& strings,
                 const std::vector<uint8_t>& binary);

            /// Sends a packet informing server that client is blocked (e.g. modal File 
            /// Open window is blocking main thread).
            void SendAgentPausePacket();

            /// Sends a packet informing server that client is running normal again.
            void SendAgentResumePacket();

            /// sends the derez packet for the entity ent_id (have to use unsigned longs 
            /// for PythonQt correctness...)
            void SendObjectDeRezPacket(const unsigned long ent_id, const QString &trash_id);

            /// sends the undo packet for the entity ent_id
            void SendObjectUndoPacket(const QString &ent_id);

            /// sends the redo packet for the entity ent_id
            void SendObjectRedoPacket(const QString &ent_id);

            /// duplicate the object (have to use unsigned longs for PythonQt correctness...)
            void SendObjectDuplicatePacket
                (const unsigned long ent_id,
                 const unsigned long flags,
                 const Vector3df &offset);

            /// same as above but takes the offset vector as three floats (have to use 
            /// unsigned longs for PythonQt correctness...)
            void SendObjectDuplicatePacket
                (const unsigned long ent_id,
                 const unsigned long flags,
                 const float offset_x,
                 const float offset_y,
                 const float offset_z);

            /// without the offset, reverts to Vector.ZERO (have to use unsigned longs for 
            /// PythonQt correctness...)
            void SendObjectDuplicatePacket
                (const unsigned long ent_id, 
                 const unsigned long flags);

            /// Sends an UUIDNameRequest for UUID-username lookup.
            ///@param user_id User ID.
            void SendUUIDNameRequestPacket(const RexUUID &user_id);

            /// Sends an UUIDNameRequest for UUID-username lookup.
            /// Translate a UUID into first and last names
            ///@param user_ids List of user ID's.
            void SendUUIDNameRequestPacket(const std::vector<RexUUID> &user_ids);

            /// Sends an UUIDGroupNameRequest packet for UUID-group name lookup.
            ///@param group_id Group ID.
            void SendUUIDGroupNameRequestPacket(const RexUUID &group_id);

            /// Sends an UUIDGroupNameRequest packet for UUID-group name lookup.
            ///@param group_ids List of group ID's.
            void SendUUIDGroupNameRequestPacket(const std::vector<RexUUID> &group_ids);

            /// Sends an ObjectLink packet.
            ///@param local_ids List of local entity ID's.
            void SendObjectLinkPacket(const std::vector<entity_id_t> &local_ids);
            void SendObjectLinkPacket(const QStringList& strings);
            /// Sends an ObjectDelink packet.
            ///@param local_ids List of local entity ID's.
            void SendObjectDelinkPacket(const std::vector<entity_id_t> &local_ids);
            void SendObjectDelinkPacket(const QStringList& strings);

            //=================================================================

        private:
            /// Sends all the needed packets to server when connection successfull
            void SendLoginSuccessfullPackets();

            /// Convenience function to get the weak pointer when building messages.
            LLOutMessage *StartMessageBuilding(const LLMsgID &message_id);

            /// Convenience function to get the weak pointer when sending messages.
            void FinishMessageBuilding(LLOutMessage *msg);

            /// WriteFloatToBytes
            void WriteFloatToBytes(float value, uint8_t* bytes, int& idx);

        private:
            /// LLUDP Packet manager
            LLMessageManager *messagemgr_;

            /// Server-spesific info for this client.
            LLStreamParameters params_;

            /// table to store individual message handlers
            MessageHandlerMap handlers_;

            /// Is client connected to a server.
            bool connected_;

            /// Block serial number used for AgentPause and AgentResume messages.
            uint32_t block_serial_num_;
    };
}

#endif
