// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_GenericMessageUtils_h
#define incl_RexNetworking_GenericMessageUtils_h

#include "RexNetworkingModuleApi.h"

namespace RexNetworking
{
    class LLInMessage;

    //! parse method from generic message packet
    REXNETWORKING_MODULE_API std::string ParseGenericMessageMethod(LLInMessage& message);

    //! parse parameter list from generic message packet
    REXNETWORKING_MODULE_API StringVector ParseGenericMessageParameters(LLInMessage& message);
}

#endif 
