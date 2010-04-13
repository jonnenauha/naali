// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolModuleTaiga_ProtocolModuleTaiga_h
#define incl_ProtocolModuleTaiga_ProtocolModuleTaiga_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include "LLSession.h"
#include "LLStream.h"
#include "LLMessageManager/LLProtocolMsgIDs.h"

namespace Foundation
{
    class StreamInterface;
}

namespace RexNetworking
{
    typedef std::vector <Foundation::Session *> SessionList;

    /** \defgroup RexNetworking 
      This module contains implementations of world Sessions and Streams
      used by RexLogic to speak with a network protocol.
      @{
      */

    /// 
    class RexNetworkingModule : public Foundation::ModuleInterfaceImpl
    {
        public: 
            RexNetworkingModule();
            virtual ~RexNetworkingModule();

            virtual void Initialize();
            virtual void Uninitialize();
            virtual void Update(f64 frametime);

            MODULE_LOGGING_FUNCTIONS

            //! Returns name of this module. Needed for logging.
            static const std::string &NameStatic() 
            { 
                return Foundation::Module::NameFromType(type_static_); 
            }

            //! Returns type of this module. Needed for logging.
            static const Foundation::Module::Type type_static_ = 
                Foundation::Module::MT_Networking;

        private:
            SessionList active_;
    };

    /// @}
}


#endif
