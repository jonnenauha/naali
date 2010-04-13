// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "GenericMessageUtils.h"
#include "LLInMessage.h"

namespace RexNetworking
{
    std::string ParseGenericMessageMethod(LLInMessage& message)
    {
        message.ResetReading();
        message.SkipToNextVariable(); // AgentId
        message.SkipToNextVariable(); // SessionId
        message.SkipToNextVariable(); // TransactionId
        std::string methodname = message.ReadString(); // Method

        return methodname;
    }

    StringVector ParseGenericMessageParameters(LLInMessage& message)
    {
        message.ResetReading();
        message.SkipToFirstVariableByName("Parameter");

        StringVector ret;

        // Variable block begins
        size_t instance_count = message.ReadCurrentBlockInstanceCount();

        while (instance_count--)
        {
            ret.push_back(message.ReadString());
        }

        return ret;
    }
}
