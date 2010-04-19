// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment module. Environment module is be responsible of visual environment features like terrain, sky & water.
 */

#include "StableHeaders.h"
#include "EnvironmentModule.h"
#include "OgreTextureResource.h"
#include "SceneManager.h"
#include "NetworkEvents.h"
#include "InputEvents.h"

#include "Terrain.h"
#include "Water.h"
#include "Environment.h"
#include "Sky.h"
#include "EnvironmentEditor.h"
#include "EC_Water.h"
#include <OgreRenderingModule.h>
#include "PostProcessWidget.h"
#include "ModuleManager.h"
#include "EventManager.h"
#include "RexNetworkUtils.h"
#include "RexNetworkingModule.h"
#include "LLMessageManager/GenericMessageUtils.h"

namespace Environment
{

    struct LayerDataHandler
    {
        EnvironmentModule   *module;
        EnvironmentEditor   *editor;

        LayerDataHandler (EnvironmentModule *m, EnvironmentEditor *e)
            : module (m), editor (e)
        {}

        void operator() (RexNetworking::LLInMessage *msg)
        {
            if(module->GetTerrainHandler().get())
            {
                static int count = 0;
                bool kill_event = module->GetTerrainHandler()->HandleOSNE_LayerData(msg);
                if (editor) editor->UpdateTerrain();
            }
        }
    };

    struct GenericMessageHandler
    {
        EnvironmentModule   *module;
        PostProcessWidget   *postprocess;

        GenericMessageHandler (EnvironmentModule *m, PostProcessWidget *p)
            : module (m), postprocess (p)
        {}

        void operator() (RexNetworking::LLInMessage *msg)
        {
            std::string methodname = RexNetworking::ParseGenericMessageMethod(*msg);

            if (methodname == "RexPostP")
            {
                boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module =
                    module->GetFramework()->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>(Foundation::Module::MT_Renderer).lock();
                if (rendering_module.get())
                {
                    OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();
                    OgreRenderer::CompositionHandler &c_handler = renderer->GetCompositionHandler();
                    StringVector vec = RexNetworking::ParseGenericMessageParameters(*msg);
                    //Since postprocessing effect was enabled/disabled elsewhere, we have to notify the dialog about the event.
                    //Also, no need to put effect on from the CompositionHandler since the dialog will notify CompositionHandler when 
                    //button is checked
                    if (postprocess)
                    {
                        QString effect_name = c_handler.MapNumberToEffectName(vec.at(0)).c_str();
                        bool enabled = true;
                        if (vec.at(1) == "False")
                            enabled = false;

                        postprocess->EnableEffect(effect_name,enabled);
                    }
                }
            }
            else if(methodname == "RexSky" && module->GetSkyHandler().get())
            {
                module->GetSkyHandler()->HandleRexGM_RexSky(msg);
            }
            else if (methodname == "RexWaterHeight")
            {
                msg->ResetReading();
                msg->SkipToFirstVariableByName("Parameter");

                // Variable block begins, should have currently (at least) 1 instances.
                size_t instance_count = msg->ReadCurrentBlockInstanceCount();
                if (instance_count < 1)
                    return;

                if (module->GetWaterHandler().get() != 0)
                {
                    std::string message = msg->ReadString();
                    // Convert to float.
                    try
                    {
                        float height = boost::lexical_cast<float>(message);
                        module->GetWaterHandler()->SetWaterHeight(height);
                    }
                    catch(boost::bad_lexical_cast&)
                    {
                    }
                }
            }
            else if (methodname == "RexDrawWater")
            {
                msg->ResetReading();
                msg->SkipToFirstVariableByName("Parameter");

                // Variable block begins, should have currently (at least) 1 instances.
                size_t instance_count = msg->ReadCurrentBlockInstanceCount();
                if (instance_count < 1 )
                    return;

                std::string message = msg->ReadString();
                bool draw = ParseBool(message);
                if (draw)
                    if (module->GetWaterHandler().get())
                        module->GetWaterHandler()->CreateWaterGeometry();
                    else
                        module->CreateWater();
                else
                    module->GetWaterHandler()->RemoveWaterGeometry();
            }
            else if (methodname == "RexFog")
            {
                /**
                 * Currently we interprent that this message information is for water fog ! Not for ground fog.
                 * @todo Someone needs to add more parameters to this package so that we can make ground fog also,
                 */

                StringVector parameters = RexNetworking::ParseGenericMessageParameters(*msg); 
                if ( parameters.size() < 5)
                    return;

                // may have , instead of . so replace
                ReplaceCharInplace(parameters[0], ',', '.');
                ReplaceCharInplace(parameters[1], ',', '.');
                ReplaceCharInplace(parameters[2], ',', '.');
                ReplaceCharInplace(parameters[3], ',', '.');
                ReplaceCharInplace(parameters[4], ',', '.');
                float fogStart = 0.0, fogEnd = 0.0, fogC_r = 0.0, fogC_g = 0.0, fogC_b = 0.0;

                try
                {
                    fogStart = boost::lexical_cast<float>(parameters[0]);
                    fogEnd = boost::lexical_cast<float>(parameters[1]);
                    fogC_r = boost::lexical_cast<float>(parameters[2]);
                    fogC_g = boost::lexical_cast<float>(parameters[3]);
                    fogC_b = boost::lexical_cast<float>(parameters[4]);
                }
                catch(boost::bad_lexical_cast&)
                {
                    return;
                }
                if (module->GetEnvironmentHandler() != 0)
                {
                    // Adjust fog.
                    QVector<float> color;
                    color<<fogC_r<<fogC_g<<fogC_b;
                    module->GetEnvironmentHandler()->SetWaterFog(fogStart, fogEnd, color); 
                }
            }
            else if (methodname == "RexAmbientL")
            {
                /**
                 * Deals RexAmbientLight message. 
                 **/

                StringVector parameters = RexNetworking::ParseGenericMessageParameters(*msg); 
                if ( parameters.size() < 3)
                    return;

                // may have , instead of . so replace
                ReplaceCharInplace(parameters[0], ',', '.');
                ReplaceCharInplace(parameters[1], ',', '.');
                ReplaceCharInplace(parameters[2], ',', '.');

                const QChar empty(' ');
                StringVector sun_light_direction = SplitString(parameters[0].c_str(), empty.toAscii() );
                StringVector sun_light_color = SplitString(parameters[1].c_str(), empty.toAscii());
                StringVector ambient_light_color = SplitString(parameters[2].c_str(), empty.toAscii());

                if ( module->GetEnvironmentHandler() != 0 )
                {
                    module->GetEnvironmentHandler()->SetSunDirection(module->GetEnvironmentHandler()->ConvertToQVector<float>(sun_light_direction));
                    module->GetEnvironmentHandler()->SetSunColor(module->GetEnvironmentHandler()->ConvertToQVector<float>(sun_light_color));
                    module->GetEnvironmentHandler()->SetAmbientLight(module->GetEnvironmentHandler()->ConvertToQVector<float>(ambient_light_color));
                }
            }
        }
    };

    struct SimulatorViewerTimeMessageHandler
    {
        EnvironmentModule *module;

        SimulatorViewerTimeMessageHandler (EnvironmentModule *m)
            : module (m)
        {}

        void operator() (RexNetworking::LLInMessage *msg)
        {
            if (module->GetEnvironmentHandler()!= 0)
                module->GetEnvironmentHandler()->HandleSimulatorViewerTimeMessage(msg);
        }
    };

    struct RegionHandshakeHandler
    {
        EnvironmentModule *module;
        EnvironmentEditor *editor;

        RegionHandshakeHandler (EnvironmentModule *m, EnvironmentEditor *e)
            : module (m), editor (e)
        {}

        void operator() (RexNetworking::LLInMessage *msg)
        {
            bool kill_event = module->HandleOSNE_RegionHandshake(msg);
            if (editor)
                editor->UpdateTerrainTextureRanges();
        }
    };

    struct RegionInfoHandler
    {
        RexNetworking::LLStream *stream;
        bool *waiting_for_regioninfo;

        RegionInfoHandler (RexNetworking::LLStream *s, bool *w)
            : stream (s), waiting_for_regioninfo (w)
        {}

        void operator() (RexNetworking::LLInMessage *msg)
        {
            if (*waiting_for_regioninfo)
            {
                stream->SendTextureCommitMessage();
                *waiting_for_regioninfo = false;
            }
        }
    };

    EnvironmentModule::EnvironmentModule() :
        ModuleInterfaceImpl(Foundation::Module::MT_Environment),
        waiting_for_regioninfomessage_(false),
        environment_editor_(0),
        postprocess_dialog_(0),
        resource_event_category_(0),
        scene_event_category_(0),
        framework_event_category_(0),
        input_event_category_(0)
    {
    }

    EnvironmentModule::~EnvironmentModule()
    {
    }

    void EnvironmentModule::Load()
    {
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_Water);
    }

    void EnvironmentModule::Initialize()
    {
        //initialize postprocess dialog
        boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
            framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>(Foundation::Module::MT_Renderer).lock();
        if (!rendering_module)
            return;

        OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();

        postprocess_dialog_ = new PostProcessWidget(renderer->GetCompositionHandler().GetAvailableCompositors());
        postprocess_dialog_->SetHandler(&renderer->GetCompositionHandler());
        postprocess_dialog_->AddSelfToScene(this);
    }

    void EnvironmentModule::PostInitialize()
    {
        event_manager_ = framework_->GetEventManager();

        resource_event_category_ = event_manager_->QueryEventCategory("Resource");
        if (resource_event_category_ == 0)
            LogError("Failed to query \"Resource\" event category");

        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        if (scene_event_category_ == 0)
            LogError("Failed to query \"Scene\" event category");

        framework_event_category_ = event_manager_->QueryEventCategory("Framework");
        if (framework_event_category_ == 0)
            LogError("Failed to query \"Framework\" event category");

        input_event_category_ = event_manager_->QueryEventCategory("Input");
        if (input_event_category_ == 0)
            LogError("Failed to query \"Input\" event category");
    }

    void EnvironmentModule::SubscribeToNetworkEvents()
    {
        network_in_event_category_ = event_manager_->QueryEventCategory("NetworkIn");
        if (network_in_event_category_ == 0)
            LogError("Failed to query \"NetworkIn\" event category");

        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");
        if (network_state_event_category_ == 0)
            LogError("Failed to query \"NetworkState\" event category");
    }

    void EnvironmentModule::Uninitialize()
    {
        SAFE_DELETE(environment_editor_);
        SAFE_DELETE(postprocess_dialog_);
        terrain_.reset();
        water_.reset();
        environment_.reset();
        sky_.reset();
        event_manager_.reset();

        waiting_for_regioninfomessage_ = false;
    }

    void EnvironmentModule::Update(f64 frametime)
    {
        // HACK Initialize editor_widget_ in correct time. 
        if (environment_editor_ == 0 && terrain_.get() != 0 && water_.get() != 0)
            environment_editor_ = new EnvironmentEditor(this);

        if ((currentWorldStream_) && currentWorldStream_->IsConnected())
        {
            if (environment_.get())
                environment_->Update(frametime);
        }
    }

    bool EnvironmentModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(category_id == framework_event_category_)
        {
            HandleFrameworkEvent(event_id, data);
        }
        else if(category_id == resource_event_category_)
        {
            HandleResouceEvent(event_id, data);
        }
        else if (category_id == network_state_event_category_)
        {
            if (event_id == RexNetworking::Events::EVENT_SERVER_CONNECTED)
            {
                if (GetFramework()->GetDefaultWorldScene().get())
                {
                    CreateTerrain();
                    CreateWater();
                    CreateEnvironment();
                    CreateSky();
                }
            }

            if (event_id == RexNetworking::Events::EVENT_SERVER_DISCONNECTED)
                if(postprocess_dialog_)
                    postprocess_dialog_->DisableAllEffects();
        }
        else if(category_id == input_event_category_)
        {
            HandleInputEvent(event_id, data);
        }
        return false;
    }

    bool EnvironmentModule::HandleResouceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Resource::Events::RESOURCE_READY)
        {
            Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
            assert(res);
            if (!res)
                return false;

            OgreRenderer::OgreTextureResource *tex = dynamic_cast<OgreRenderer::OgreTextureResource *>(res->resource_.get()); 
            if (tex)
            {
                // Pass the texture asset to the terrain manager - the texture might be in the terrain.
                if (terrain_.get())
                    terrain_->OnTextureReadyEvent(res);

                // Pass the texture asset to the sky manager - the texture might be in the sky.
                if (sky_.get())
                    sky_->OnTextureReadyEvent(res);
            }
            Foundation::TextureInterface *decoded_tex = decoded_tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
            if (decoded_tex)
                // Pass the texture asset to environment editor.
                if (environment_editor_)
                    environment_editor_->HandleResourceReady(res);
        }

        return false;
    }

    bool EnvironmentModule::HandleFrameworkEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        switch(event_id)
        {
            case Foundation::NETWORKING_REGISTERED:
            {
                // Begin to listen network events.
                SubscribeToNetworkEvents();
                return false;
            }
            case Foundation::WORLD_STREAM_READY:
            {
                RexNetworking::LLStreamReadyEvent *event_data = dynamic_cast<RexNetworking::LLStreamReadyEvent *>(data);
                if (event_data)
                {
                    currentWorldStream_ = event_data->stream;
                }

                return false;
            }
        }

        return false;
    }

    void EnvironmentModule::SetStream(RexNetworking::LLStream* stream)
    {
        using std::make_pair;

        RexNetworking::LLStream::MessageHandlerMap map;
        map.insert(make_pair(RexNetMsgLayerData, LayerDataHandler(this, environment_editor_)));
        map.insert(make_pair(RexNetMsgGenericMessage, GenericMessageHandler(this, postprocess_dialog_)));
        map.insert(make_pair(RexNetMsgSimulatorViewerTimeMessage, SimulatorViewerTimeMessageHandler(this)));
        map.insert(make_pair(RexNetMsgRegionHandshake, RegionHandshakeHandler(this, environment_editor_)));
        map.insert(make_pair(RexNetMsgRegionInfo, RegionInfoHandler(stream, &waiting_for_regioninfomessage_)));

        stream-> SetHandlers (map);
        currentWorldStream_ = stream;
    }

    TerrainPtr EnvironmentModule::GetTerrainHandler() const
    {
        return terrain_;
    }

    EnvironmentPtr EnvironmentModule::GetEnvironmentHandler() const
    {
        return environment_;
    }

    SkyPtr EnvironmentModule::GetSkyHandler() const
    {
        return sky_;
    }

    WaterPtr EnvironmentModule::GetWaterHandler() const
    {
        return water_;
    }

    void EnvironmentModule::SendModifyLandMessage(f32 x, f32 y, u8 brush, u8 action, Real seconds, Real height)
    {
        if (currentWorldStream_)
            currentWorldStream_->SendModifyLandPacket(x, y, brush, action, seconds, height);
    }

    void EnvironmentModule::SendTextureHeightMessage(Real start_height, Real height_range, uint corner)
    {
        if (currentWorldStream_)
        {
            currentWorldStream_->SendTextureHeightsMessage(start_height, height_range, corner);
            waiting_for_regioninfomessage_ = true;
        }
    }

    void EnvironmentModule::SendTextureDetailMessage(const RexTypes::RexAssetID &new_texture_id, uint texture_index)
    {
        if (currentWorldStream_)
        {
            currentWorldStream_->SendTextureDetail(new_texture_id, texture_index);
            waiting_for_regioninfomessage_ = true;
        }
    }

    void EnvironmentModule::CreateTerrain()
    {
        terrain_ = TerrainPtr(new Terrain(this));

        Scene::EntityPtr entity = GetFramework()->GetDefaultWorldScene()->CreateEntity(GetFramework()->GetDefaultWorldScene()->GetNextFreeId());
        entity->AddComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_Terrain"));

        terrain_->FindCurrentlyActiveTerrain();
    }

    void EnvironmentModule::CreateWater()
    {
        water_ = WaterPtr(new Water(this));
        water_->CreateWaterGeometry();
    }

    void EnvironmentModule::CreateEnvironment()
    {
        environment_ = EnvironmentPtr(new Environment(this));
        environment_->CreateEnvironment();
    }

    void EnvironmentModule::CreateSky()
    {
        sky_ = SkyPtr(new Sky(this));
        Scene::EntityPtr sky_entity = GetFramework()->GetDefaultWorldScene()->CreateEntity(GetFramework()->GetDefaultWorldScene()->GetNextFreeId());
        sky_entity->AddComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_OgreSky"));

        sky_->FindCurrentlyActiveSky();

        if (!GetEnvironmentHandler()->IsCaelum())
            sky_->CreateDefaultSky();
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Environment;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(EnvironmentModule)
POCO_END_MANIFEST
