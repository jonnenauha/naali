// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Stream_h
#define incl_Foundation_Stream_h

#include <boost/function.hpp>

namespace Foundation
{
    //! Base class for world Streams; bi-directional real-time network communication channels
    class Stream
    {
        public:
            typedef boost::function <void()> ConnectionHandler;

            /// Returns whether stream is connected
            virtual bool IsConnected () const = 0;

            /// Modify the state of the stream
            virtual bool Connect (std::string address, int port) = 0;
            virtual bool Disconnect () = 0;
            
            /// Forces stream messages to be sent/received
            virtual void Pump () = 0;

            /// Handlers for changes in the state of the stream
            ConnectionHandler OnConnect;
            ConnectionHandler OnDisconnect;
    };
}

#endif
