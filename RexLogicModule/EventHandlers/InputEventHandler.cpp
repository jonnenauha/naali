// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
#include "InputServiceInterface.h"
#include "LLSession.h"

namespace RexLogic
{
    InputEventHandler::InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;
    }

    InputEventHandler::~InputEventHandler()
    {
    }
    
    bool InputEventHandler::HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Input::Events::SWITCH_CAMERA_STATE)
        {
            // Only switch if connected
            if (rexlogicmodule_->GetLLSession()->IsConnected())
                rexlogicmodule_->SwitchCameraState();
            return false;
        }
        return false;
    }

    void InputEventHandler::Update(f64 frametime)
    {
    }
}
