// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_StreamInterface_h
#define incl_Foundation_StreamInterface_h

namespace Foundation
{
    //! Interface for output Streams
    class InputStreamInterface
    {
        public:
            virtual void OnConnect () = 0;
            virtual void OnDisconnect () = 0;
            virtual bool IsConnected () const = 0;
    };

    //! Interface for input Streams
    class OutputStreamInterface
    {
        public:
            virtual bool Connect (std::string address, int port) = 0;
            virtual bool Disconnect () = 0;
            virtual bool IsConnected () const = 0;
    };
}

#endif
