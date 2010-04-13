// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkMessageHandler_h
#define incl_NetworkMessageHandler_h

#include "Framework.h"
#include "RexNetworkingModule.h"

namespace RexNetworking
{
    class LLInMessage;
}

namespace RexLogic
{
    class RexLogicModule;
	class ScriptDialogHandler;
	typedef boost::shared_ptr<ScriptDialogHandler> ScriptDialogHandlerPtr;

    /// Handles incoming LLUDP network events in a reX-specific way. \todo Break down into more logical functions.
    class NetworkMessageHandler
    {
    public:
        NetworkMessageHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~NetworkMessageHandler();

        // !Handler functions for Opensim network events
        bool HandleOSNE_AgentMovementComplete(RexNetworking::LLInMessage *data);
        bool HandleOSNE_ImprovedTerseObjectUpdate(RexNetworking::LLInMessage *data);
        bool HandleOSNE_KillObject(RexNetworking::LLInMessage *data);
        bool HandleOSNE_LogoutReply(RexNetworking::LLInMessage *data);
        bool HandleOSNE_ObjectUpdate(RexNetworking::LLInMessage *data);
        bool HandleOSNE_RegionHandshake(RexNetworking::LLInMessage *data);
        bool HandleOSNE_SoundTrigger(RexNetworking::LLInMessage *data);
        bool HandleOSNE_PreloadSound(RexNetworking::LLInMessage *data);
		bool HandleOSNE_ScriptDialog(RexNetworking::LLInMessage *data);

        //! Handler functions for GenericMessages
        bool HandleOSNE_GenericMessage(RexNetworking::LLInMessage *data);

    private:
        Foundation::Framework *framework_;
        RexLogicModule *rexlogicmodule_;
		ScriptDialogHandlerPtr script_dialog_handler_;
    };
}

#endif
