// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

#include "Foundation.h"

namespace RexLogic
{
    class InputStateInterface;
    class RexLogicModule;

    //! Performs the keyboard and mouse input handling of RexLogic.
    class InputEventHandler
    {
    public:
        InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~InputEventHandler();
        
        //! handle an input event
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        void Update(Core::f64 frametime);

    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
    };
}

#endif