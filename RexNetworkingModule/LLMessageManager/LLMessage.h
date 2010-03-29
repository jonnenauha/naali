// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexNetworking_NetMessage_h
#define incl_RexNetworking_NetMessage_h

#include <string>
#include <vector>
//#include "RexTypes.h"

namespace RexNetworking
{
    /// UDP message header flags.
    enum LLHeaderFlag
    {
        LLFlagAck = 0x10, ///< The message contains appended ACKs.
        LLFlagResent = 0x20, ///< This message is a resend, since the other end didn't ACK it in time.
        LLFlagReliable = 0x40, ///< This message needs to be ACKed.
        LLFlagZeroCode = 0x80  ///< This message is compressed by RLE-encoding zeroes.
    };

    /// Identifies the type of a variable inside a network message block.
    /// \ingroup OpenSimProtocolClient
    enum LLVariableType
    {
        LLVarInvalid = 0,
        LLVarU8,
        LLVarU16,
        LLVarU32,
        LLVarU64,
        LLVarS8,
        LLVarS16,
        LLVarS32,
        LLVarS64,
        LLVarF32,
        LLVarF64,
        LLVarVector3,
        LLVarVector3d,
        LLVarVector4,
        LLVarQuaternion,
        LLVarUUID,
        LLVarBOOL,
        LLVarIPADDR,
        LLVarIPPORT,
        LLVarFixed,
        LLVarBufferByte,   ///< A variable-length buffer where the length is encoded by one byte. (<= 255 bytes in size)
        LLVarBuffer2Bytes, ///< A variable-length buffer where the length is encoded by two bytes. (<= 65535 bytes in size)
        LLVarBuffer4Bytes, ///< A variable-length buffer where the length is encoded by four bytes. (<= 4GB bytes in size)
        LLVarNone, ///< Tells the message generator that no more variables are expected, used as a sentinel.
        LLVariableTypeEnumCount, ///< The number of valid variable types.
    };

    /// Identifies the size of variables inside a network message block. The indices match the ones above. ///\todo Replace with a type trait / utility function.
    const size_t LLVariableSizes[] = { 0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 12, 24, 16, 12, 16, 1, 4, 2, 0, 1, 2, 4, 0 };

    /// Identifies the trust level of a message.
    enum LLTrustLevel
    {
        LLTrustLevelInvalid = 0,
        LLNotTrusted,
        LLTrusted,
        LLTrustLevelEnumCount,
    };

    /// Identifies the encoding of a message.
    enum LLEncoding
    {
        LLEncodingInvalid = 0,
        LLUnencoded,
        LLZeroEncoded,
        LLEncodingEnumCount,
    };

    /// Describes a variable that is present in a message block.
    /// \ingroup OpenSimProtocolClient
    struct LLMessageVariable
    {
        std::string name;
        LLVariableType type;

        ///< The number of times this variable occurs in the stream, or the length of the buffer in bytes, if type == LLVarBufferXX.
        size_t count; 
    };

    /// Identifies the type of a message block inside a network message.
    /// \ingroup OpenSimProtocolClient
    enum LLMessageBlockType
    {
        LLBlockInvalid = 0,
        LLBlockSingle,
        LLBlockMultiple,
        LLBlockVariable 
    };

    /// LLwork messages consist of message blocks. This structure describes a single block.
    /// \ingroup OpenSimProtocolClient
    struct LLMessageBlock
    {
        std::string name;
        LLMessageBlockType type;
        /// How many times this block is repeated in the message.
        size_t repeatCount;
        /// A block consists of one or several variables. They appear in the order they're present in this vector.
        std::vector<LLMessageVariable> variables;
        /// Specifies the multiplicity of this block. If repeatCount == LLMessageBlock::cVariableCount, then the number of blocks
        /// is given by a single byte in the network packet.
        ///\todo Is this needed? Doesn't "LLMessageBlockType == LLBlockVariable"  indicate this also?
        static const int cVariableCount = -1;
    };

    // Probably going to remove this, useless (for now at least).
    /*enum LLPriorityLevel
    {
        LLPriorityInvalid = 0,
        LLPriorityHigh,
        LLPriorityMedium,
        LLPriorityLow,
        LLPriorityEnumCount
    };*/

    /// Identifies different UDP message packets.
    /// \ingroup OpenSimProtocolClient
    typedef unsigned long LLMsgID;

    /// Describes the format of an UDP network message.
    /// \ingroup OpenSimProtocolClient
    struct LLMessageInfo
    {
        std::string name;
        std::vector<LLMessageBlock> blocks;

        LLMsgID id;
        LLTrustLevel trustLevel;
        LLEncoding encoding;
    };

}

#endif // incl_RexNetworking_NetMessage_h
