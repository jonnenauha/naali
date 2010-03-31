// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_StreamInterface_h
#define incl_Foundation_StreamInterface_h

namespace Foundation
{
    //! Interface for World Streams, bi-directional real-time network communication channels
    class StreamInterface
    {
        public:
            /// Returns whether stream is connected
            virtual bool IsConnected () const = 0;

            /// Modify the state of the stream
            virtual bool Connect (std::string address, int port) = 0;
            virtual bool Disconnect () = 0;
            
            /// Reacts to changes in the state of the stream
            virtual void OnConnect () = 0;
            virtual void OnDisconnect () = 0;

            /// Forces stream messages to be sent/received
            virtual void Pump () = 0;
    };
}

#endif
