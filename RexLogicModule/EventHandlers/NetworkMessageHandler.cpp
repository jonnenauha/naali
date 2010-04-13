// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/NetworkMessageHandler.h"
#include "LLMessageManager/LLInMessage.h"
#include "LLMessageManager/LLProtocolMsgIDs.h"
#include "LLMessageManager/BitStream.h"
#include "LLMessageManager/GenericMessageUtils.h"
#include "RexLogicModule.h"
#include "Entity.h"
#include "Avatar/AvatarControllable.h"
#include "ConversionUtils.h"
#include "Avatar/Avatar.h"
#include "Environment/Primitive.h"
#include "SceneEvents.h"
#include "SoundServiceInterface.h"
#include "AssetServiceInterface.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "Communications/ScriptDialogHandler.h"
// Ogre renderer -specific.
#include <OgreMaterialManager.h>

namespace
{
    /// Clones a new Ogre material that renders using the given ambient color. 
    /// This function will be removed or refactored later on, once proper material system is present. -jj.
    void DebugCreateAmbientColorMaterial(const std::string &materialName, float r, float g, float b)
    {
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);
        if (material.get()) // The given material already exists, so no need to create it again.
            return;

        material = mm.getByName("SolidAmbient");
        if (!material.get())
            return;

        Ogre::MaterialPtr newMaterial = material->clone(materialName);
        newMaterial->setAmbient(r, g, b);
    }
}

namespace RexLogic
{
    using namespace RexNetworking;

    NetworkMessageHandler::NetworkMessageHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;

        Foundation::ModuleWeakPtr renderer = framework_->GetModuleManager()->GetModule(Foundation::Module::MT_Renderer);
        if (renderer.expired() == false)
        {
            DebugCreateAmbientColorMaterial("AmbientWhite", 1.f, 1.f, 1.f);
            DebugCreateAmbientColorMaterial("AmbientGreen", 0.f, 1.f, 0.f);
            DebugCreateAmbientColorMaterial("AmbientRed", 1.f, 0.f, 0.f);
        }

        script_dialog_handler_ = ScriptDialogHandlerPtr(new ScriptDialogHandler(framework));
    }

    NetworkMessageHandler::~NetworkMessageHandler()
    {
    }

    bool NetworkMessageHandler::HandleOSNE_ObjectUpdate(LLInMessage* msg)
    {
        msg->ResetReading();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();

        if (msg->GetCurrentBlock() >= msg->GetBlockCount())
        {
            RexLogicModule::LogDebug("Empty ObjectUpdate packet received, ignoring.");
            return false;
        }

        size_t instance_count = msg->ReadCurrentBlockInstanceCount();
        bool result = false;
        if (instance_count > 0)
        {
            msg->SkipToFirstVariableByName("PCode");
            uint8_t pcode = msg->ReadU8();
            switch(pcode)
            {
                case 0x09:
                    result = rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_ObjectUpdate(msg);
                    break;

                case 0x2f:
                    result = rexlogicmodule_->GetAvatarHandler()->HandleOSNE_ObjectUpdate(msg);
                    break;
            }
        }

        return result;
    }

    bool NetworkMessageHandler::HandleOSNE_GenericMessage(LLInMessage* msg)
    {
        std::string methodname = RexNetworking::ParseGenericMessageMethod(*msg);

        if (methodname == "RexMediaUrl")
            return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexMediaUrl(msg);
        else if (methodname == "RexData")
            return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexFreeData(msg); 
        else if (methodname == "RexPrimData")
            return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexPrimData(msg); 
        else if (methodname == "RexPrimAnim")
            return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexPrimAnim(msg); 
        else if (methodname == "RexAppearance")
            return rexlogicmodule_->GetAvatarHandler()->HandleRexGM_RexAppearance(msg);
        else if (methodname == "RexAnim")
            return rexlogicmodule_->GetAvatarHandler()->HandleRexGM_RexAnim(msg);
        else
            return false;
    }

    bool NetworkMessageHandler::HandleOSNE_RegionHandshake(LLInMessage* msg)
    {
        msg->ResetReading();

        msg->SkipToNextVariable(); // RegionFlags U32
        msg->SkipToNextVariable(); // SimAccess U8

        std::string sim_name = msg->ReadString();
        rexlogicmodule_->GetLLSession()->GetSessionParameters().sim_name = sim_name.c_str();
        RexLogicModule::LogInfo("Joined to sim " + sim_name);

        /*msg->SkipToNextVariable(); // SimOwner
          msg->SkipToNextVariable(); // IsEstateManager
          msg->SkipToNextVariable(); // WaterHeight
          msg->SkipToNextVariable(); // BillableFactor
          msg->SkipToNextVariable(); // CacheID
          for(int i = 0; i < 4; ++i)
          msg->SkipToNextVariable(); // TerrainBase0..3
          RexAssetID terrain[4];
        // TerrainDetail0-3
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();

        //TerrainStartHeight
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();

        // TerrainHeightRange
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();
        msg->SkipToNextVariable();*/

        // Create the "World" scene.
        rexlogicmodule_->GetLLStream()->SendRegionHandshakeReplyPacket
            (rexlogicmodule_->GetLLSession()->GetStreamParameters().agent_id, 
             rexlogicmodule_->GetLLSession()->GetStreamParameters().session_id, 0);

        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_LogoutReply(LLInMessage* msg)
    {
        msg->ResetReading();

        RexUUID aID = msg->ReadUUID();
        RexUUID sID = msg->ReadUUID();

        if (aID == rexlogicmodule_->GetLLSession()->GetStreamParameters().agent_id &&
                sID == rexlogicmodule_->GetLLSession()->GetStreamParameters().session_id)
        {
            RexLogicModule::LogInfo("LogoutReply received with matching IDs. Logging out.");
            rexlogicmodule_->GetLLStream()->Disconnect();
            rexlogicmodule_->DeleteScene("World");
        }

        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_AgentMovementComplete(LLInMessage* msg)
    {
        msg->ResetReading();

        RexUUID agentid = msg->ReadUUID();
        RexUUID sessionid = msg->ReadUUID();

        //    if (agentid == rexlogicmodule_->GetServerConnection()->GetInfo().agentID &&
        //    sessionid == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
        {
            Vector3 position = msg->ReadVector3(); 
            Vector3 lookat = msg->ReadVector3();

            assert(rexlogicmodule_->GetAvatarControllable() && "Handling agent movement complete event before avatar controller is created.");
            rexlogicmodule_->GetAvatarControllable()->HandleAgentMovementComplete(
                    OpenSimToOgreCoordinateAxes(position), OpenSimToOgreCoordinateAxes(lookat));

            /// \todo tucofixme, what to do with regionhandle & timestamp?
            uint64_t regionhandle = msg->ReadU64();
            uint32_t timestamp = msg->ReadU32(); 
        }

        rexlogicmodule_->GetLLStream()->SendAgentSetAppearancePacket();
        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_ImprovedTerseObjectUpdate(LLInMessage* msg)
    {
        msg->ResetReading();

        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.

        if (msg->GetCurrentBlock() >= msg->GetBlockCount())
        {
            RexLogicModule::LogDebug("Empty ImprovedTerseObjectUpdate packet received, ignoring.");
            return false;
        }

        // Variable block
        size_t instance_count = msg->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < instance_count; i++)
        {
            size_t bytes_read = 0;
            const uint8_t *bytes = msg->ReadBuffer(&bytes_read);

            uint32_t localid = 0;
            switch(bytes_read)
            {
                case 30:
                    rexlogicmodule_->GetAvatarHandler()->HandleTerseObjectUpdate_30bytes(bytes); 
                    break;
                case 60:
                    localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[0]); 
                    if (rexlogicmodule_->GetPrimEntity(localid)) 
                        rexlogicmodule_->GetPrimitiveHandler()->HandleTerseObjectUpdateForPrim_60bytes(bytes);
                    else if (rexlogicmodule_->GetAvatarEntity(localid))
                        rexlogicmodule_->GetAvatarHandler()->HandleTerseObjectUpdateForAvatar_60bytes(bytes);
                    break;
                default:
                    std::stringstream ss; 
                    ss << "Unhandled ImprovedTerseObjectUpdate block of size " << bytes_read << "!";
                    RexLogicModule::LogInfo(ss.str());
                    break;
            }

            msg->SkipToNextVariable(); ///\todo Unhandled inbound variable 'TextureEntry'.
        }
        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_KillObject(LLInMessage* msg)
    {
        msg->ResetReading();

        // Variable block
        size_t instance_count = msg->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < instance_count; ++i)
        {
            uint32_t killedobjectid = msg->ReadU32();
            if (rexlogicmodule_->GetPrimEntity(killedobjectid))
                return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_KillObject(killedobjectid);
            if (rexlogicmodule_->GetAvatarEntity(killedobjectid))
                return rexlogicmodule_->GetAvatarHandler()->HandleOSNE_KillObject(killedobjectid);
        }

        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_PreloadSound(LLInMessage* msg)
    {
        msg->ResetReading();

        size_t instance_count = msg->ReadCurrentBlockInstanceCount();
        while(instance_count)
        {
            msg->ReadUUID(); // ObjectID
            msg->ReadUUID(); // OwnerID
            std::string asset_id = msg->ReadUUID().ToString(); // Sound asset ID

            // Preload the sound asset into cache, the sound service will get it from there when actually needed.
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
                framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
                asset_service->RequestAsset(asset_id, RexTypes::ASSETTYPENAME_SOUNDVORBIS);

            --instance_count;
        }

        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_SoundTrigger(LLInMessage* msg)
    {
        msg->ResetReading();

        std::string asset_id = msg->ReadUUID().ToString(); // Sound asset ID
        msg->ReadUUID(); // OwnerID
        msg->ReadUUID(); // ObjectID
        msg->ReadUUID(); // ParentID
        msg->ReadU64(); // Regionhandle, todo handle
        Vector3df position = msg->ReadVector3(); // Position
        Real gain = msg->ReadF32(); // Gain

        // Because sound triggers are not supposed to stop the previous sound, like attached sounds do, 
        // it is easy to spam with 100's of sound trigger requests.
        // What we do now is that if we find the same sound playing "too many" times, we stop one of them
        static const uint MAX_SOUND_INSTANCE_COUNT = 4;
        uint same_sound_detected = 0;
        sound_id_t sound_to_stop = 0;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem =
            rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return false;

        std::vector<sound_id_t> playing_sounds = soundsystem->GetActiveSounds();
        for (uint i = 0; i < playing_sounds.size(); ++i)
        {
            if ((soundsystem->GetSoundName(playing_sounds[i]) == asset_id) &&
                    (soundsystem->GetSoundType(playing_sounds[i]) == Foundation::SoundServiceInterface::Triggered))
            {
                same_sound_detected++;
                // This should be the oldest instance of the sound, because soundsystem gives channel ids from a map (ordered).
                if (!sound_to_stop)
                    sound_to_stop = playing_sounds[i]; 
            }
        }

        if (same_sound_detected >= MAX_SOUND_INSTANCE_COUNT)
            soundsystem->StopSound(sound_to_stop);

        sound_id_t new_sound = soundsystem->PlaySound3D(asset_id, Foundation::SoundServiceInterface::Triggered, false, position);
        soundsystem->SetGain(new_sound, gain);

        return false;
    }

    bool NetworkMessageHandler::HandleOSNE_ScriptDialog(LLInMessage* msg)
    {
        msg->ResetReading();

        std::string object_id = msg->ReadUUID().ToString(); // ObjectID
        std::string first_name = msg->ReadString(); // FirstName
        std::string last_name = msg->ReadString(); // LastName
        std::string object_name = msg->ReadString(); // ObjectName
        std::string message = msg->ReadString(); // Message
        s32 chat_channel = msg->ReadS32(); // ChatChannel
        std::string image_id = msg->ReadUUID().ToString(); // ImageID

        size_t instance_count = msg->ReadCurrentBlockInstanceCount();
        std::vector<std::string> button_labels;
        while(instance_count)
        {
            std::string button_label = msg->ReadString(); // ButtonLabel
            button_labels.push_back(button_label);
            --instance_count;
        }

        std::string owners_name = first_name + " " + last_name;
        ScriptDialogRequest request(object_name, owners_name, message, chat_channel, button_labels);
        if (script_dialog_handler_)
            script_dialog_handler_->Handle(request);

        return false;
    }
}
