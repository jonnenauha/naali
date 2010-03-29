// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <boost/cstdint.hpp>

#include "LLMessageList.h"
#include "CoreDefines.h"

using namespace std;
using boost::uint32_t;

namespace RexNetworking
{

/// Indicates the state of the parsing process.
enum ParseState
{
    TopBraceOpen,
    PacketType,
    PacketDataBraceOpen,
    PacketDataBraceOpenOrBraceClose,
    PacketBlockName,
    PacketBlockDataOrBraceClose
};

/// Identifies the priority level of a network message.
enum LLPriorityLevel
{
    LLPriorityInvalid = 0,
    LLPriorityHigh,
    LLPriorityMed,
    LLPriorityLow,
    LLPriorityFixed
};

/// Parses one line of the template file.
/// @param file A C string of the contents of the file in memory.
int ParseFirstLine(const char *file, char *dst, int dstLen);

/// Loads the given file into a C string. Call delete[] when you're done with it.
char *LoadFileToString(const char *filename);

/// Cuts whitespaces off the string in-place. Replaces them with '\0'.
size_t TrimTrailingWhitespaces(char *str);

/// Cuts single-line comments off the line, in-place. Replaces them with '\0'.
size_t RemoveComments(char *line);

LLMessageList::LLMessageList(const char *filename)
{
    ParseMessageListFromFile(filename);
}

LLMessageList::~LLMessageList()
{

}

/// @return Line length, or -1 if at end.
int ParseFirstLine(const char *file, char *dst, int dstLen)
{
    if (dstLen == 0)
        return -1;

    if (!file)
    {
        *dst = 0;
        return -1;
    }

    if (*file == '\0')
    {
        *dst = 0;
        return -1;
    }

    int trimmedLength = 0;
    // Trim whitespace and invalid characters from line start.
    while(((unsigned char)*file <= 0x20 || (unsigned char)*file >= 0x80) && *file != '\0')
    {
        ++trimmedLength;
        ++file;
    }

    if (*file == '\0')
    {
        *dst = 0;
        return -1;
    }

    // Go through the line data.
    int length = 0;
    while(*file != '\0' && *file != '\n' && (unsigned char)*file <= 0x7f)
    {
        *dst = *file;
        ++length;
        ++file;
        ++dst;
        if (length+1 >= dstLen)
            break;
    }
    *dst = '\0';

    return length + trimmedLength;
}

///\todo Memory management.
char *LoadFileToString(const char *filename)
{
    unsigned long len;
    char *data;
    std::fstream filestr(filename, fstream::in);
    
    if (!filestr)
        return 0;
    
    filestr.seekg (0, ios::end);
    len = filestr.tellg();
    filestr.seekg (ios::beg);
    data = new char[len+1];
    memset(data, 0, len+1);
    filestr.read(data, len);
    data[len] = '\0';
    filestr.close();

    return data;
}


/// @return Length of the new string.
size_t TrimTrailingWhitespaces(char *str)
{    
    size_t len = strlen(str);
    char *s = &str[len-1];

    while((unsigned char)*s <= 0x20 && s >= str)
    {
        *s = '\0';
        --s;
        --len;
    }

    return len;
}

/// @return Length of the new string.
size_t RemoveComments(char *line)
{
    char *occur = strstr(line, "//");

    if (!occur)
        return strlen(line);
    else
    {
        *occur = '\0';
        return TrimTrailingWhitespaces(line);
    }
}

/// @return Variable type as an integer.
LLVariableType StrToVariableType(const char *str)
{
    const char *data[] = { "Invalid", "U8", "U16", "U32", "U64", "S8", "S16", "S32", "S64", "F32", "F64", "LLVector3", "LLVector3d", "LLVector4",
                           "LLQuaternion", "LLUUID", "BOOL", "IPADDR", "IPPORT", "Fixed", "Variable" };

    for(int i = 0; i < NUMELEMS(data); ++i)

#ifdef _MSC_VER
        if (!_strcmpi(data[i], str))
#else
        if (!strcasecmp(data[i], str))
#endif
            return (LLVariableType)i;

    return LLVarInvalid;
}

/// @return Block type as an integer
LLMessageBlockType StrToBlockType(const char *str)
{
    if (!strcmp(str, "Single")) return LLBlockSingle;
    if (!strcmp(str, "Multiple")) return LLBlockMultiple;
    if (!strcmp(str, "Variable")) return LLBlockVariable;
    return LLBlockInvalid;
}

/// @return Priority level as an string
const char *PriorityLevelToStr(LLPriorityLevel p)
{
    if (p == LLPriorityFixed) return "Fixed";
    if (p == LLPriorityHigh) return "High";
    if (p == LLPriorityMed) return "Med";
    if (p == LLPriorityLow) return "Low";
    return "Invalid";
}

/// @return Priority level as an integer
LLPriorityLevel StrToPriorityLevel(const char *p)
{
    if (!strcmp(p, "High")) return LLPriorityHigh;
    if (!strcmp(p, "Medium")) return LLPriorityMed;
    if (!strcmp(p, "Low")) return LLPriorityLow;
    if (!strcmp(p, "Fixed")) return LLPriorityFixed;
    return LLPriorityInvalid;
}

/// @return Priority number as an integer
uint32_t StrToPriorityNumber(const char *p)
{
    if (!strncmp(p, "0x", 2))
    {
        p += 2;
        stringstream str(stringstream::in | stringstream::out);
        str << p;
        unsigned long i;
        str >> std::hex >> i;
        return i;
    }

    stringstream str;
    str << p;
    unsigned long i;
    str >> i;
    return i;
}

/// @return Priority number as an integer
LLTrustLevel StrToTrustLevel(const char *str)
{
    if (!strcmp(str, "NotTrusted")) return LLNotTrusted;
    if (!strcmp(str, "Trusted")) return LLTrusted;
    return LLTrustLevelInvalid;
}

/// @return Priority number as a boolean
LLEncoding StrToEncoding(const char *str)
{
    if (!strcmp(str, "Unencoded")) return LLUnencoded;
    if (!strcmp(str, "Zerocoded")) return LLZeroEncoded;
    return LLEncodingInvalid;
}

static LLMsgID PriorityAndMsgNumberToMsgID(LLPriorityLevel priority, uint32_t number)
{
    switch(priority)
    {
    case LLPriorityHigh:
    case LLPriorityFixed:
        return (LLMsgID)number;
        break;
    case LLPriorityMed:
        return number | 0xFF00;
    case LLPriorityLow:
        return number | 0xFFFF0000;
    default:
        assert(false); ///\todo Error propagation.
        return 0;
    }
}

/// @return Parsing state as a string
const char *ParseStateToString(ParseState s)
{
    const char *d[] = { "TopBraceOpen", "PacketType", "PacketDataBraceOpen", "PacketDataBraceOpenOrBraceClose",
                        "PacketBlockName", "PacketBlockDataOrBraceClose" };
    return d[s];
}

const LLMessageInfo *LLMessageList::GetMessageInfoByID(LLMsgID id) const
{
    LLworkMessageMap::const_iterator iter = messages.find(id);
    if (iter != messages.end())
        return &iter->second;
    else
        return 0;
}

void LLMessageList::ParseMessageListFromFile(const char *filename)
{
    char *file = LoadFileToString(filename);
    char *data = file;

    const int maxLen = 512;
    char line[maxLen];

    ParseState parseState = TopBraceOpen;
    LLMessageInfo *curMsg = 0;
    LLMessageBlock *curBlock = 0;

    for(;;)
    {
        // Read next line.    
        size_t length = ParseFirstLine(data, line, 510);
        if (length == -1) // If an error, quit.
            break;
        data += length;
        length = RemoveComments(line);
        if (length <= 0) // If line empty, go to next.
            continue;
        length = TrimTrailingWhitespaces(line);
        if (length <= 0) // If line empty, go to next.
            continue;

        switch(parseState)
        {
        case TopBraceOpen:
            if (!strcmp(line, "{"))
                parseState = PacketType;
            break;
        case PacketType:
            {
                char packetName[maxLen];
                char priorityType[maxLen];
                char priorityNumber[maxLen];
                char trusted[maxLen];
                char zeroCoded[maxLen];
                
                sscanf(line, "%s %s %s %s %s", packetName, priorityType, priorityNumber, trusted, zeroCoded);
                LLMessageInfo msgInfo;
                msgInfo.name = packetName;
                LLPriorityLevel priorityLevel = StrToPriorityLevel(priorityType);
                uint32_t number = StrToPriorityNumber(priorityNumber);
/*                if (msgInfo.priority == LLPriorityFixed)
                {
                    msgInfo.priority = LLPriorityLow;
                    msgInfo.priorityNumber &= 0xFFFF;
                }*/
                msgInfo.id = PriorityAndMsgNumberToMsgID(priorityLevel, number);
                msgInfo.trustLevel = StrToTrustLevel(trusted);
                msgInfo.encoding = StrToEncoding(zeroCoded);
                messages[msgInfo.id] = msgInfo;
                curMsg = &messages[msgInfo.id];
                curBlock = 0;

                parseState = PacketDataBraceOpenOrBraceClose;
            }
            break;
        case PacketDataBraceOpenOrBraceClose:
            if (!strcmp(line, "}"))
                parseState = TopBraceOpen;
            // fall through.
        case PacketDataBraceOpen:
            if (!strcmp(line, "{"))
                parseState = PacketBlockName;
            break;
        case PacketBlockName:
            {            
                char blockName[maxLen];
                char blockType[maxLen];
                int blockRepeatCount = 0;
                sscanf(line, "%s %s %d", blockName, blockType, &blockRepeatCount);
                LLMessageBlock msgBlock;
                msgBlock.name = blockName;

                msgBlock.type = StrToBlockType(blockType);
                if (msgBlock.type == LLBlockSingle)
                    blockRepeatCount = 1;
                msgBlock.repeatCount = blockRepeatCount;
                curMsg->blocks.push_back(msgBlock);
                curBlock = &curMsg->blocks.back();
                parseState = PacketBlockDataOrBraceClose;
            }
            break;
        case PacketBlockDataOrBraceClose:
            {
                if (line[0] == '{')
                {
                    char varName[maxLen];
                    char varType[maxLen];
                    int varSize = 0;
                    sscanf(&line[1], "%s %s %d", varName, varType, &varSize);
                    LLMessageVariable msgVar;
                    msgVar.name = varName;

                    msgVar.type = StrToVariableType(varType);
                    if (msgVar.type == LLVarBufferByte)
                        msgVar.count = 0;
                    else
                        msgVar.count = varSize;

                    // Is this a 2-byte encoded variable sized parameter?
                    if (msgVar.type == LLVarBufferByte && varSize == 2)
                        msgVar.type = LLVarBuffer2Bytes;

                    curBlock->variables.push_back(msgVar);
                }
                else if (line[0] == '}')
                    parseState = PacketDataBraceOpenOrBraceClose;
            }
            break;
        } // ~switch
    } // ~for
    delete[] file;
}

static bool LLMessageInfoCmp(const LLMessageInfo &a, const LLMessageInfo &b)
{
    return a.id < b.id;
}

void LLMessageList::GenerateHeaderFile(const char *filename) const
{
    ofstream out(filename);

    out << "/* This file defines constants for all the messages used in the protocol. The values" << endl
        << "match to the IDs of the messages, and are used for identifying different kinds of " << endl
        << "packets. This file is automatically generated from the message template file, so no " << endl
        << "point modifying it here. */" << endl
        << endl
        << "#ifndef RexProtocolMsgIDs" << endl
        << "#define RexProtocolMsgIDs" << endl
        << endl;

    std::vector<LLMessageInfo> msgs;
    for(LLworkMessageMap::const_iterator iter = messages.begin(); iter != messages.end(); ++iter)
    {
        const LLMessageInfo &msg = iter->second;
        msgs.push_back(msg);
    }
    std::sort(msgs.begin(), msgs.end(), LLMessageInfoCmp);

    for(std::vector<LLMessageInfo>::const_iterator iter = msgs.begin(); iter != msgs.end(); ++iter)
    {
        const LLMessageInfo &msg = *iter;
        out << "const LLMsgID RexNetMsg" << setw(40) << left << msg.name << " = " << "0x" << hex << msg.id << ";" << endl;
    }

    out << endl << "#endif" << endl;
}

}
