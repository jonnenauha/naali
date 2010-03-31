// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LLStream.h"
#include "LLMessageManager/RexProtocolMsgIDs.h"

#include "QuatUtils.h"
#include "ConversionUtils.h"

#include <QUrl>
#include <QString>
#include <QStringList>

namespace RexNetworking
{
    const std::string &LLStream::logname = "LLStream";
    const std::string LL_MESSAGE_TEMPLATE_FILE = "./data/message_template.msg";
    extern Real LL_THROTTLE_MAX_BPS;

    struct NullConnectHandler { void operator()(){} };
    struct NullLLMessageHandler { void operator()(LLInMessage *){} };

    LLStream::LLStream () : 
        messagemgr_ (new LLMessageManager (LL_MESSAGE_TEMPLATE_FILE.c_str())),
        connected_(false), 
        block_serial_num_(0)
    {
        initialize_();
    }

    LLStream::LLStream (const LLStreamParameters &params) : 
        messagemgr_ (new LLMessageManager (LL_MESSAGE_TEMPLATE_FILE.c_str())),
        params_ (params),
        connected_(false), 
        block_serial_num_(0)
    {
        initialize_();
    }

    void LLStream::initialize_ ()
    {
        LogInfo("LLStream created and ready.");
        
        OnConnect                   = NullConnectHandler();
        OnDisconnect                = NullConnectHandler();
        OnRegionHandshake           = NullLLMessageHandler();
        OnAgentMovementComplete     = NullLLMessageHandler();
        OnAvatarAnimation           = NullLLMessageHandler();
        OnGenericMessage            = NullLLMessageHandler();
        OnLogoutReply               = NullLLMessageHandler();
        OnImprovedTerseObjectUpdate = NullLLMessageHandler();
        OnKillObject                = NullLLMessageHandler();
        OnObjectUpdate              = NullLLMessageHandler();
        OnObjectProperties          = NullLLMessageHandler();
        OnAttachedSound             = NullLLMessageHandler();
        OnAttachedSoundGainChange   = NullLLMessageHandler();
        OnSoundTrigger              = NullLLMessageHandler();
        OnPreloadSound              = NullLLMessageHandler();
        OnScriptDialog              = NullLLMessageHandler();
    }

    LLStream::~LLStream()
    {
        delete messagemgr_;
    }

    bool LLStream::Connect (std::string address, int port)
    {
        if (!connected_) 
        {
            if (messagemgr_-> ConnectTo (address.c_str(), port))
            {
                messagemgr_-> RegisterNetworkListener (this);
                connected_ = true;

                OnConnect ();

                SendLoginSuccessfullPackets ();
            }
        }

        return connected_;
    }

    bool LLStream::Disconnect ()
    {
        if (connected_)
        {
            messagemgr_-> RegisterNetworkListener (this);
            messagemgr_-> Disconnect ();
            connected_ = false;

            OnDisconnect ();
        }

        return true;
    }

    void LLStream::Pump ()
    {
        messagemgr_-> ProcessMessages ();
    }

    bool LLStream::IsConnected () const
    {
        return connected_;
    }

    void LLStream::SetParameters (const LLStreamParameters &params)
    {
        params_ = params;
    }
    
    void LLStream::OnMessageReceived (LLMsgID msg_id, LLInMessage *msg)
    {
        std::cout << "LLStream: received: " << msg_id << std::endl;

        switch (msg_id)
        {
            case RexNetMsgRegionHandshake:
                OnRegionHandshake (msg); break;

            case RexNetMsgAgentMovementComplete:
                OnAgentMovementComplete (msg); break;

            case RexNetMsgAvatarAnimation:
                OnAvatarAnimation (msg); break;

            case RexNetMsgGenericMessage:
                OnGenericMessage (msg); break;

            case RexNetMsgLogoutReply:
                OnLogoutReply (msg); break;

            case RexNetMsgImprovedTerseObjectUpdate:
                OnImprovedTerseObjectUpdate (msg); break;

            case RexNetMsgKillObject:
                OnKillObject (msg); break;

            case RexNetMsgObjectUpdate:
                OnObjectUpdate (msg); break;

            case RexNetMsgObjectProperties:
                OnObjectProperties (msg); break;

            case RexNetMsgAttachedSound:
                OnAttachedSound (msg); break;

            case RexNetMsgAttachedSoundGainChange:
                OnAttachedSoundGainChange (msg); break;

            case RexNetMsgSoundTrigger:
                OnSoundTrigger (msg); break;

            case RexNetMsgPreloadSound:
                OnPreloadSound (msg); break;

            case RexNetMsgScriptDialog:
                OnScriptDialog (msg); break;
        }
    }
            
    void LLStream::SendUseCircuitCodePacket()
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUseCircuitCode);
        assert(m);

        m->AddU32(params_.circuit_code);
        m->AddUUID(params_.session_id);
        m->AddUUID(params_.agent_id);
        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentWearablesRequestPacket()
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentWearablesRequest);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendCompleteAgentMovementPacket()
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgCompleteAgentMovement);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddU32(params_.circuit_code);
        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentThrottlePacket()
    {
        if (!connected_)
            return;

        int idx = 0;
        static const size_t size = 7 * sizeof(Real);
        u8 throttle_block[size];

        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.1f, throttle_block, idx); // resend
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.1f, throttle_block, idx); // land
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.02f, throttle_block, idx); // wind
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.02f, throttle_block, idx); // cloud
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.25f, throttle_block, idx); // task
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.26f, throttle_block, idx); // texture
        WriteFloatToBytes(LL_THROTTLE_MAX_BPS * 0.25f, throttle_block, idx); // asset

        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentThrottle);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddU32(params_.circuit_code);
        m->AddU32(0); // Generation counter
        m->AddBuffer(size, throttle_block); // throttles
        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendRexStartupPacket(const std::string& state)
    {
        StringVector strings;

        strings.push_back(params_.agent_id.ToString());
        strings.push_back(state);
        SendGenericMessage("RexStartup", strings);
    }

    void LLStream::SendLogoutRequestPacket()
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgLogoutRequest);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        FinishMessageBuilding(m);

        LogInfo("Sent a Logout Request to the server... waiting for reply before quitting.");
    }

    void LLStream::SendChatFromViewerPacket(const std::string &text, s32 channel)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgChatFromViewer);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBuffer(text.length(), (uint8_t*)text.c_str()); // Message
        m->AddU8(1); // Type
        m->AddS32(channel); // Channel

        FinishMessageBuilding(m);
    }

    void LLStream::SendImprovedInstantMessagePacket(const RexUUID &target, const std::string &text)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgImprovedInstantMessage);
        assert(m);

        unsigned int parent_estate_id = 0; //! @todo Find out proper value
        unsigned int time_stamp = 0; 
        std::string from_name = "Fixme"; //! TODO: Get value from session

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBool(false); // from group
        m->AddUUID(target);
        m->AddU32(parent_estate_id);
        m->AddUUID(params_.region_id); //! @todo Find out proper value
        m->AddVector3(RexTypes::Vector3());//! @todo Find out proper value
        m->AddU8(0);//! @todo Find out proper value
        m->AddU8(0); // dialog type
        m->AddUUID(RexUUID());
        m->AddU32(time_stamp); // TODO: Timestamp
        m->AddBuffer( strlen(from_name.c_str()), (uint8_t*)(from_name.c_str()) );
        m->AddBuffer( strlen(text.c_str()), (uint8_t*)(text.c_str()) );
        m->AddBuffer(0, 0); // BinaryBucket

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectAddPacket(const RexTypes::Vector3 &position)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectAdd);
        assert(m);

        Vector3 scale(0.5f, 0.5f, 0.5f);
        Quaternion rotation(0, 0, 0, 1);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID());      // GroupID

        // ObjectData
        m->AddU8(0x09);             // PCode: 0x09 - prim
        m->AddU8(3);                // Material
        m->AddU32(0x02);            // AddFlags: 0x01 - use physics, 0x02 - create selected

        m->AddU8(16);               // PathCurve
        m->AddU8(1);                // ProfileCurve
        m->AddU16(0);               // PathBegin
        m->AddU16(0);               // PathEnd
        m->AddU8(100);              // PathScaleX
        m->AddU8(100);              // PathScaleY
        m->AddU8(0);                // PathShearX
        m->AddU8(0);                // PathShearY
        m->AddS8(0);                // PathTwist
        m->AddS8(0);                // PathTwistBegin
        m->AddS8(0);                // PathRadiusOffset
        m->AddS8(0);                // PathTaperX
        m->AddS8(0);                // PathTaperY
        m->AddU8(0);                // PathRevolutions
        m->AddS8(0);                // PathSkew
        m->AddU16(0);               // ProfileBegin
        m->AddU16(0);               // ProfileEnd
        m->AddU16(0);               // ProfileHollow
        m->AddU8(1);                // BypassRaycast
        m->AddVector3(position);    // RayStart     ///\note We use same position for both RayStart and RayEnd.
        m->AddVector3(position);    // RayEnd
        m->AddUUID(RexUUID());      // RayTargetID
        m->AddU8(0);                // RayEndIsIntersection
        m->AddVector3(scale);       // Scale
        m->AddQuaternion(rotation); // Rotation
        m->AddU8(0);                // State

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDeletePacket(const uint32_t &local_id, const bool &force)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelete);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBool(force);

        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(local_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDeletePacket(const std::vector<uint32_t> &local_id_list, const bool &force)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelete);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBool(force);

        // ObjectData
        m->SetVariableBlockCount(local_id_list.size());
        for(size_t i = 0; i < local_id_list.size(); ++i)
            m->AddU32(local_id_list[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentUpdatePacket(
            Quaternion bodyrot,
            Quaternion headrot,
            uint8_t state,
            RexTypes::Vector3 camcenter,
            RexTypes::Vector3 camataxis,
            RexTypes::Vector3 camleftaxis,
            RexTypes::Vector3 camupaxis,
            float fardist,
            uint32_t controlflags,
            uint8_t flags)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentUpdate);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddQuaternion(OgreToOpenSimQuaternion(bodyrot));
        m->AddQuaternion(OgreToOpenSimQuaternion(headrot));
        m->AddU8(state);
        m->AddVector3(camcenter);
        m->AddVector3(camataxis);
        m->AddVector3(camleftaxis);
        m->AddVector3(camupaxis);
        m->AddF32(fardist);
        m->AddU32(controlflags);
        m->AddU8(flags);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectSelectPacket(const unsigned int object_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32((entity_id_t)object_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectSelectPacket(std::vector<entity_id_t> object_id_list)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDeselectPacket(entity_id_t object_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(object_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDeselectPacket(std::vector<entity_id_t> object_id_list)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendMultipleObjectUpdatePacket(const std::vector<ObjectUpdateInfo>& update_info_list)
    {
        if (!connected_)
            return;

        if (!update_info_list.size())
            return;

        // Will actually send two packets: first one for position & scale, then one for rotation.
        // Protocol does not seem to support sending all three

        // 1. Position & scale packet                             
        LLOutMessage *m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        ///\todo Update just the necessary parameters (use update flags) & test with multiple objects.
        size_t offset = 0;
        uint8_t data[2048]; ///\todo What is the max size?

        m->SetVariableBlockCount(update_info_list.size());

        for(size_t i = 0; i < update_info_list.size(); ++i)
        {
            m->AddU32(update_info_list[i].local_id);
            m->AddU8(13);

            // Position
            Vector3 pos (OgreToOpenSimCoordinateAxes (update_info_list[i].position));
            memcpy(&data[offset], &pos, sizeof(Vector3));
            offset += sizeof(Vector3);

            // Scale
            Vector3 scale (OgreToOpenSimCoordinateAxes (update_info_list[i].scale));
            memcpy(&data[offset], &scale, sizeof(Vector3));
            offset += sizeof(Vector3);
        }

        // Add the data.
        m->AddBuffer(offset, data);
        FinishMessageBuilding(m);

        // 2. Rotation packet                          
        m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        offset = 0;

        m->SetVariableBlockCount(update_info_list.size());

        for(size_t i = 0; i < update_info_list.size(); ++i)
        {
            m->AddU32(update_info_list[i].local_id);
            m->AddU8(2);

            // Rotation
            Vector3 val = PackQuaternionToFloat3(update_info_list[i].orientation);
            memcpy(&data[offset], &val, sizeof(Vector3));
            offset += sizeof(Vector3);
        }

        // Add the data.
        m->AddBuffer(offset, data);
        FinishMessageBuilding(m);    
    }

    void LLStream::SendObjectNamePacket(const std::vector<ObjectNameInfo>& name_info_list)
    {
        if (!connected_)
            return;

        if (!name_info_list.size())
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectName);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(name_info_list.size());
        for(size_t i = 0; i < name_info_list.size(); ++i)
        {
            m->AddU32(name_info_list[i].local_id);
            m->AddString(name_info_list[i].name);
        }
    }

    void LLStream::SendObjectGrabPacket(entity_id_t object_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectGrab);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->AddU32(object_id);
        //! \todo Touch offset is not send / calculated currently since it is not really used by the server anyway. -cm
        m->AddVector3(Vector3::ZERO);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDescriptionPacket(const std::vector<ObjectDescriptionInfo>& description_info_list)
    {
        if (!connected_)
            return;

        if (!description_info_list.size())
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDescription);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(description_info_list.size());
        for(size_t i = 0; i < description_info_list.size(); ++i)
        {
            m->AddU32(description_info_list[i].local_id);
            m->AddString(description_info_list[i].description);
        }
    }

    void LLStream::SendRegionHandshakeReplyPacket(RexUUID agent_id, RexUUID session_id, uint32_t flags)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgRegionHandshakeReply);

        m->AddUUID(agent_id);
        m->AddUUID(session_id); 
        m->AddU32(flags);

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentSetAppearancePacket()
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentSetAppearance);

        // Agentdata
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id); 
        m->AddU32(1);
        m->AddVector3(RexTypes::Vector3(0.45f, 0.6f, 1.708378f)); // Values for a default avatar height based on server

        // Wearabledata (empty)
        m->SetVariableBlockCount(0);
        m->AdvanceToNextVariable();

        // ObjectData (empty)
        m->AddBuffer(0,0);

        // VisualParam, these determine avatar's height on the server
        uint8_t visualparams[218];
        for(size_t i = 0; i < 218; ++i)
            visualparams[i] = 0;

        // Params based on source of server, which uses 218 params. 
        visualparams[25] = 122; // Body height
        visualparams[77] = 0; // Shoe heel height
        visualparams[78] = 0; // Shoe platform height
        visualparams[120] = 68; // Head size
        visualparams[125] = 178; // Leg length
        visualparams[148] = 117; // Neck length

        m->SetVariableBlockCount(218);
        for(size_t i = 0; i < 218; ++i)
            m->AddU8(visualparams[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendModifyLandPacket(f32 x, f32 y, u8 brush, u8 action, Real seconds, Real height)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgModifyLand);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ModifyBlock
        m->AddU8(action);
        m->AddU8(brush);
        m->AddF32(seconds);
        m->AddF32(height);

        // ParcelData
        m->SetVariableBlockCount(1);
        m->AddS32(-1);//LocalID
        m->AddF32(x); //west
        m->AddF32(y); //south
        m->AddF32(x); //east
        m->AddF32(y); //north

        FinishMessageBuilding(m);
    }

    void LLStream::SendTextureDetail(const RexTypes::RexAssetID &new_texture_id, uint texture_index)
    {
        if (!connected_)
            return;

        RexNetworking::LLOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID::CreateRandom());

        // MethodData
        /*QString method = "textureheights";
          QByteArray bytearray = method.toUtf8();*/
        std::string method = "texturedetail";
        m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
        m->AddUUID(RexUUID::CreateRandom());

        // texture detail id
        m->SetVariableBlockCount(1);
        std::string data = QString("%1").arg(texture_index).toStdString() + " " + new_texture_id;
        m->AddBuffer(data.size() + 1, (uint8_t*)data.c_str());

        FinishMessageBuilding(m);
    }

    void LLStream::SendTextureHeightsMessage(Real start_height, Real height_range, uint corner)
    {
        if (!connected_)
            return;

        RexNetworking::LLOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID::CreateRandom());

        // MethodData
        /*QString method = "textureheights";
          QByteArray bytearray = method.toUtf8();*/
        std::string method = "textureheights";
        m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
        m->AddUUID(RexUUID::CreateRandom());

        // HeightData
        m->SetVariableBlockCount(1);
        // Convert all number parameters into one string parameter that will be send into the server.
        std::string data = QString("%1").arg(corner).toStdString() + " " + QString("%1").arg(start_height).toStdString() + " " + QString("%1").arg(height_range).toStdString();
        m->AddBuffer(data.size() + 1, (uint8_t*)data.c_str());

        FinishMessageBuilding(m);
    }

    void LLStream::SendTextureCommitMessage()
    {
        if (!connected_)
            return;

        RexNetworking::LLOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID::CreateRandom());

        // MethodData
        /*QString method = "textureheights";
          QByteArray bytearray = method.toUtf8();*/
        std::string method = "texturecommit";
        m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
        m->AddUUID(RexUUID::CreateRandom());

        m->SetVariableBlockCount(0);

        FinishMessageBuilding(m);
    }

    void LLStream::SendCreateInventoryFolderPacket(
            const RexUUID &parent_id,
            const RexUUID &folder_id,
            const asset_type_t &type,
            const std::string &name)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgCreateInventoryFolder);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // FolderData
        m->AddUUID(folder_id);
        m->AddUUID(parent_id);
        m->AddS8(type);
        m->AddString(name);

        FinishMessageBuilding(m);
    }

    void LLStream::SendMoveInventoryFolderPacket(
            const RexUUID &folder_id,
            const RexUUID &parent_id,
            const bool &re_timestamp)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryFolder);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBool(re_timestamp);

        // FolderData
        m->SetVariableBlockCount(1);
        m->AddUUID(folder_id);
        m->AddUUID(parent_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendRemoveInventoryFolderPacket(
            const RexUUID &folder_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryFolder);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // FolderData
        m->SetVariableBlockCount(1);
        m->AddUUID(folder_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendMoveInventoryItemPacket(
            const RexUUID &item_id,
            const RexUUID &folder_id,
            const std::string &new_name,
            const bool &re_timestamp)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryItem);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddBool(re_timestamp);

        // InventoryData, variable
        m->SetVariableBlockCount(1);
        m->AddUUID(item_id);
        m->AddUUID(folder_id);
        m->AddString(new_name);

        FinishMessageBuilding(m);
    }

    void LLStream::SendCopyInventoryItemPacket(
            const RexUUID &old_agent_id,
            const RexUUID &old_item_id,
            const RexUUID &new_folder_id,
            const std::string &new_name)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgCopyInventoryItem);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // InventoryData, variable
        m->SetVariableBlockCount(1);
        m->AddU32(0); // CallbackID, we don't need this.
        m->AddUUID(old_agent_id);
        m->AddUUID(old_item_id);
        m->AddUUID(new_folder_id);
        m->AddString(new_name);

        FinishMessageBuilding(m);
    }

    void LLStream::SendRemoveInventoryItemPacket(const RexUUID &item_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // InventoryData, variable
        m->SetVariableBlockCount(1);
        m->AddUUID(item_id);

        FinishMessageBuilding(m);
    }

    ///\todo Test this function.
    void LLStream::SendRemoveInventoryItemPacket(std::list<RexUUID> item_id_list)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // InventoryData, variable
        m->SetVariableBlockCount(item_id_list.size());
        std::list<RexUUID>::iterator it;
        for(it = item_id_list.begin(); it != item_id_list.end(); ++it)
            m->AddUUID(*it);

        FinishMessageBuilding(m);
    }

    void LLStream::SendUpdateInventoryFolderPacket(
            const RexUUID &folder_id,
            const RexUUID &parent_id,
            const int8_t &type,
            const std::string &name)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUpdateInventoryFolder);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // FolderData, variable
        m->SetVariableBlockCount(1);
        m->AddUUID(folder_id);
        m->AddUUID(parent_id);
        m->AddS8(type);
        m->AddString(name);

        FinishMessageBuilding(m);
    }

    void LLStream::SendUpdateInventoryItemPacket(
            const RexUUID &item_id,
            const RexUUID &folder_id,
            const asset_type_t &asset_type,
            const inventory_type_t &inventory_type,
            const std::string &name,
            const std::string &description)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUpdateInventoryItem);
        assert(m);

        // TransactionID, new items only?
        RexUUID transaction_id;

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(transaction_id);

        // InventoryData, Variable
        m->SetVariableBlockCount(1);
        m->AddUUID(item_id);
        m->AddUUID(folder_id);
        m->AddU32(0);                       // CallbackID
        m->AddUUID(RexUUID());              // CreatorID
        m->AddUUID(RexUUID());              // OwnerID
        m->AddUUID(RexUUID());              // GroupID

        // Permissions-related:
        m->AddU32(0);                       //BaseMask
        m->AddU32(0);                       // OwnerMask
        m->AddU32(0);                       // GroupMask
        m->AddU32(0);                       // EveryoneMask
        m->AddU32(0);                       // NextOwnerMask
        m->AddBool(false);                  // GroupOwned

        m->AddUUID(transaction_id);
        m->AddS8(asset_type);               // Type
        m->AddS8(inventory_type);           // InvType
        m->AddU32(0);                       // Flags
        m->AddU8(0);                        // SaleType
        m->AddS32(0);                       // SalePrice
        m->AddString(name);                 // Name 
        m->AddString(description);          // Description
        m->AddS32(0);                       // CreationDAta
        m->AddU32(0);                       // CRC

        FinishMessageBuilding(m);
    }

    void LLStream::SendFetchInventoryDescendentsPacket(
            const RexUUID &folder_id,
            const RexUUID &owner_id,
            const int32_t &sort_order,
            const bool &fetch_folders,
            const bool &fetch_items)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgFetchInventoryDescendents);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // InventoryData
        m->AddUUID(folder_id);
        if (owner_id.IsNull())
            m->AddUUID(params_.agent_id);
        else
            m->AddUUID(owner_id);
        m->AddS32(sort_order);
        m->AddBool(fetch_folders);
        m->AddBool(fetch_items);

        FinishMessageBuilding(m);
    }

    void LLStream::SendAcceptFriendshipPacket(const RexUUID &transaction_id, const RexUUID &folder_id)
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgAcceptFriendship);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(transaction_id);
        m->SetVariableBlockCount(1);
        m->AddUUID(folder_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendDeclineFriendshipPacket(const RexUUID &transaction_id)
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgDeclineFriendship);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(transaction_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendFormFriendshipPacket(const RexUUID &dest_id)
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgFormFriendship);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(dest_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendTerminateFriendshipPacket(const RexUUID &other_id)
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgTerminateFriendship);
        assert(m);

        m->AddUUID(params_.agent_id);
        m->AddUUID(other_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendGenericMessage(QString method, QStringList& strings)
    {
        StringVector stringvec;
        //QVector<QString> qstv = strings.toVector();
        //stringvec = qstv.toStdVector(); //QString != std::string so this can't work, right?

        for (QStringList::const_iterator const_iter = strings.constBegin();
                const_iter != strings.constEnd(); ++const_iter)
        {
            QString qstr = *const_iter;
            stringvec.push_back(qstr.toStdString());
        }

        SendGenericMessage(method.toStdString(), stringvec);
    }

    void LLStream::SendGenericMessage(const std::string& method, const StringVector& strings)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgGenericMessage);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // Transaction ID
        m->AddUUID(RexUUID::CreateRandom());

        // Method
        m->AddString(method);

        // Invoice ID
        m->AddUUID(RexUUID::CreateRandom());

        // Variable count of strings
        m->SetVariableBlockCount(strings.size());

        // Strings
        for(uint i = 0; i < strings.size(); ++i)
            m->AddString(strings[i]);

        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendGenericMessageBinary(
            const std::string& method,
            const StringVector& strings,
            const std::vector<uint8_t>& binary)
    {
        if (!connected_)
            return;

        const size_t max_string_size = 200;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgGenericMessage);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // Transaction ID
        m->AddUUID(RexUUID::CreateRandom());

        // Method
        m->AddString(method);

        // Invoice ID
        m->AddUUID(RexUUID::CreateRandom());

        // See how many binary strings 
        size_t binarystrings = 0;
        size_t count = binary.size();
        while (count)
        {
            if (count > max_string_size)
                count -= max_string_size;
            else
                count -= count;
            binarystrings++;
        }

        // Variable count of strings
        m->SetVariableBlockCount(strings.size() + binarystrings);

        // Strings
        for(uint i = 0; i < strings.size(); ++i)
            m->AddString(strings[i]);

        // Binary strings
        size_t idx = 0;
        count = binary.size();   
        for(uint i = 0; i < binarystrings; ++i)
        {
            size_t size = count;
            if (size > max_string_size)
                size = max_string_size;

            m->AddBuffer(size, (uint8_t*)&binary[idx]);
            idx += size;
            count -= size;
        }

        m->MarkReliable();

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentPausePacket()
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentPause);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddU32(++block_serial_num_);

        FinishMessageBuilding(m);
    }

    void LLStream::SendAgentResumePacket()
    {
        LLOutMessage *m = StartMessageBuilding(RexNetMsgAgentResume);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddU32(++block_serial_num_);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDeRezPacket(const unsigned long ent_id, const QString &trash_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgDeRezObject);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID()); //group_id

        // ObjectData
        m->AddU8(4); //trash ?
        RexUUID ruuid = RexUUID();
        ruuid.FromString(trash_id.toStdString());
        m->AddUUID(ruuid);
        m->AddUUID(RexUUID::CreateRandom()); //transaction_id
        m->AddU8(1);
        m->AddU8(1);

        m->SetVariableBlockCount(1);
        m->AddU32(ent_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectUndoPacket(const QString &ent_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUndo);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID());      // GroupID

        // ObjectData
        m->SetVariableBlockCount(1);
        RexUUID ruuid = RexUUID();
        ruuid.FromString(ent_id.toStdString());
        m->AddUUID(ruuid);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectRedoPacket(const QString &ent_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgRedo);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID());      // GroupID

        // ObjectData
        m->SetVariableBlockCount(1);
        RexUUID ruuid = RexUUID();
        ruuid.FromString(ent_id.toStdString());
        m->AddUUID(ruuid);

        FinishMessageBuilding(m);
    }


    void LLStream::SendObjectDuplicatePacket(
            const unsigned long ent_id,
            const unsigned long flags,
            const Vector3df &offset)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDuplicate);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);
        m->AddUUID(RexUUID());      // GroupID

        // SharedData
        m->AddVector3(offset); //umm, prolly needs the location... offset
        m->AddU32(flags);
        m->SetVariableBlockCount(1);
        m->AddU32(ent_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDuplicatePacket(
            const unsigned long ent_id,
            const unsigned long flags,
            const float offset_x,
            const float offset_y,
            const float offset_z)
    {
        SendObjectDuplicatePacket(ent_id, flags, Vector3df(offset_x, offset_y, offset_z));
    }

    void LLStream::SendObjectDuplicatePacket(const unsigned long ent_id, const unsigned long flags)
    {
        SendObjectDuplicatePacket(ent_id, flags, Vector3df(0,0,0));
    }

    void LLStream::SendUUIDNameRequestPacket(const RexUUID &user_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUUIDNameRequest);
        assert(m);

        m->SetVariableBlockCount(1);
        m->AddUUID(user_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendUUIDNameRequestPacket(const std::vector<RexUUID> &user_ids)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUUIDNameRequest);
        assert(m);

        m->SetVariableBlockCount(user_ids.size());
        for(int i = 0; i < user_ids.size(); ++i)
            m->AddUUID(user_ids[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendUUIDGroupNameRequestPacket(const RexUUID &group_id)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUUIDGroupNameRequest);
        assert(m);

        m->SetVariableBlockCount(1);
        m->AddUUID(group_id);

        FinishMessageBuilding(m);
    }

    void LLStream::SendUUIDGroupNameRequestPacket(const std::vector<RexUUID> &group_ids)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgUUIDGroupNameRequest);
        assert(m);

        m->SetVariableBlockCount(group_ids.size());
        for(int i = 0; i < group_ids.size(); ++i)
            m->AddUUID(group_ids[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectLinkPacket(const std::vector<entity_id_t> &local_ids)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectLink);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(local_ids.size());
        for(int i = 0; i < local_ids.size(); ++i)
            m->AddU32(local_ids[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectLinkPacket(const QStringList& strings)
    {
        std::vector<entity_id_t> vec;

        for (QStringList::const_iterator const_iter = strings.constBegin();
                const_iter != strings.constEnd(); ++const_iter)
        {
            QString qstr = *const_iter;
            vec.push_back(qstr.toUInt());
        }

        SendObjectLinkPacket(vec);
    }

    void LLStream::SendObjectDelinkPacket(const std::vector<entity_id_t> &local_ids)
    {
        if (!connected_)
            return;

        LLOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelink);
        assert(m);

        // AgentData
        m->AddUUID(params_.agent_id);
        m->AddUUID(params_.session_id);

        // ObjectData
        m->SetVariableBlockCount(local_ids.size());
        for(int i = 0; i < local_ids.size(); ++i)
            m->AddU32(local_ids[i]);

        FinishMessageBuilding(m);
    }

    void LLStream::SendObjectDelinkPacket(const QStringList& strings)
    {
        std::vector<entity_id_t> vec;

        for (QStringList::const_iterator const_iter = strings.constBegin();
                const_iter != strings.constEnd(); ++const_iter)
        {
            QString qstr = *const_iter;
            vec.push_back(qstr.toUInt());
        }

        SendObjectDelinkPacket(vec);
    }

    void LLStream::SendLoginSuccessfullPackets()
    {
        // Send the necessary UDP packets.
        SendUseCircuitCodePacket();

        ///\todo IS THE BELOW STATEMENT STILL RELEVANT?!
        ///
        //! \todo release mode viewer sends the following packets "too fast" for some 
        //! old rexservers to cope. Wait a while. Proper solution would be to wait for 
        //! ack from the UseCircuitCode packet before continuing to send packets.  
        //! It may also be that the issue is only an issue on a localhost server 
        //! (ie. with no real network delay)
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        SendCompleteAgentMovementPacket();
        SendAgentThrottlePacket();
        SendAgentWearablesRequestPacket();
        SendRexStartupPacket("started"); 
    }

    LLOutMessage *LLStream::StartMessageBuilding(const LLMsgID &message_id)
    {
        return messagemgr_->StartNewMessage(message_id);
    }

    void LLStream::FinishMessageBuilding(LLOutMessage *msg)
    {
        messagemgr_->FinishMessage(msg);
    }

    void LLStream::WriteFloatToBytes(float value, uint8_t* bytes, int& idx)
    {
        *(float*)(&bytes[idx]) = value;
        idx += sizeof(float);
    }

}
