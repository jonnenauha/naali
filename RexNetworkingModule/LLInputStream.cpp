// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Framework.h"

#include "LLInputStream.h"

namespace RexNetworking
{
    LLInputStream::LLInputStream () 
        : connected_(false)
    {
        LogInfo("LLInputStream created and ready.");
    }

    LLInputStream::~LLInputStream()
    {
    }


} // namespace RexNetworking
