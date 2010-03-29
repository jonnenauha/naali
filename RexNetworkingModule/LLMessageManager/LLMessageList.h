// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexNetworking_NetMessageList_h
#define incl_RexNetworking_NetMessageList_h

#pragma warning( push )
#pragma warning( disable : 4396 )
#include <boost/unordered_map.hpp>
#pragma warning( pop )

#include "LLMessage.h"

namespace RexNetworking
{

/// A data structure that contains a list of known network messages.
class LLMessageList
{
public:
    /// @param filename The file to load the message list from.
    LLMessageList(const char *filename);
    ~LLMessageList();

    /// @return The message info structure corresponding to the message with the given ID, or 0 if no such
    ///         message is known.
    const LLMessageInfo *GetMessageInfoByID(LLMsgID id) const;

    /// Generates a C++ header file out of all the IDs of the known message definitions.
    void GenerateHeaderFile(const char *filename) const;

private:
    LLMessageList(const LLMessageList &);
    void operator=(const LLMessageList &);

    typedef boost::unordered_map<LLMsgID, LLMessageInfo> LLworkMessageMap;

    /// Contains all the messages known by this list.
    LLworkMessageMap messages;

    /// Reads in new messages from the given file.
    void ParseMessageListFromFile(const char *filename);
};

}
#endif
