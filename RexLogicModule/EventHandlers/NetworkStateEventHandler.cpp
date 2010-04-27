// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "RexLogicModule.h"
#include "RexNetworkingModule.h"
#include "NetworkEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "Inworld/NotificationManager.h"
#include "Inworld/Notifications/MessageNotification.h"

namespace RexLogic
{

NetworkStateEventHandler::NetworkStateEventHandler(Foundation::Framework *fw, RexLogicModule *owner)
    : framework_(fw), owner_(owner)
{
}

NetworkStateEventHandler::~NetworkStateEventHandler()
{
}

bool NetworkStateEventHandler::HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    switch(event_id)
    {
        case RexNetworking::Events::EVENT_SERVER_CONNECTED:
        {   
            // The client has connected to the server. Create a new scene for that.
            owner_->CreateNewActiveScene("World");

            // Send WorldStream as internal event
            event_category_id_t framework_category_id = framework_->GetEventManager()->QueryEventCategory("Framework");
            RexNetworking::LLSessionReadyEvent session_event_data(owner_->GetLLSession());
            framework_->GetEventManager()->SendEvent(framework_category_id, Foundation::WORLD_SESSION_READY, &session_event_data);
            RexNetworking::LLStreamReadyEvent stream_event_data(owner_->GetLLStream());
            framework_->GetEventManager()->SendEvent(framework_category_id, Foundation::WORLD_STREAM_READY, &stream_event_data);

            break;
        }
        case RexNetworking::Events::EVENT_SERVER_DISCONNECTED:
        {
            // Might be user quitting or server dropping connection.
            // This event occurs when OpenSimProtocolModule has already closed connection. 
            // Make sure the rexlogic also thinks connection is closed.
            if (owner_->GetLLStream()->IsConnected())
                owner_->GetLLStream()->Disconnect();
            if (framework_->HasScene("World"))
                owner_->DeleteScene("World");
            break;
        }
        case RexNetworking::Events::EVENT_USER_CONNECTED:
        {
            /// Show notification about new user
            RexNetworking::UserConnectivityEvent *event_data = checked_static_cast<RexNetworking::UserConnectivityEvent *>(data);
            assert(event_data);
            if (!event_data)
                return false;

            UiModulePtr ui_module = owner_->GetFramework()->GetModuleManager()->GetModule
                <UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
            if (ui_module.get() && !sent_avatar_notifications_.contains(event_data->agentId))
            {
                ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification(
                    QString("%1 joined the world").arg(event_data->fullName.c_str())));
                sent_avatar_notifications_.append(event_data->agentId);
            }
            break;
        }
        case RexNetworking::Events::EVENT_USER_DISCONNECTED:
        {
            RexNetworking::UserConnectivityEvent *event_data = checked_static_cast<RexNetworking::UserConnectivityEvent *>(data);
            assert(event_data);
            if (!event_data)
                return false;

            UiModulePtr ui_module = owner_->GetFramework()->GetModuleManager()->GetModule
                <UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
            if (ui_module.get())
            {
                ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification(
                    QString("%1 logged out").arg(event_data->fullName.c_str())));
                sent_avatar_notifications_.removeOne(event_data->agentId);
            }
            break;
        }
        default:
            break;
    }

    return false;
}

}
