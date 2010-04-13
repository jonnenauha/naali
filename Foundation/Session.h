// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Session_h
#define incl_Foundation_Session_h

#include "CoreTypes.h"
#include "Stream.h"

#include <QVariantMap>

namespace Foundation
{
    //! Base class for world Sessions; manages Streams
    class Session
    {
        public:
            typedef QMap<QString,QString> LoginParameters;

            virtual int Type () const = 0;
            virtual bool IsConnected () const = 0;

            virtual bool Login (const LoginParameters &params) = 0;
            virtual bool Logout () = 0;

            virtual Stream& GetStream () = 0;
    };
}

#endif
