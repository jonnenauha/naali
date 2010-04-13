// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_GenericMessageUtils_h
#define incl_RexNetworking_GenericMessageUtils_h

namespace RexNetworking
{
    class LLInMessage;

    //! parse method from generic message packet
    std::string ParseGenericMessageMethod(LLInMessage& message);

    //! parse parameter list from generic message packet
    StringVector ParseGenericMessageParameters(LLInMessage& message);
}

#endif 
