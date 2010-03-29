// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_SessionInterface_h
#define incl_Foundation_SessionInterface_h

#include "CoreTypes.h"
#include "StreamInterface.h"

#include <QVariantMap>

namespace Foundation
{
    typedef QVariantMap LoginParameters;

    //! Interface for Session
    class SessionInterface
    {
        public:
            virtual int Type () const = 0;
            virtual bool IsConnected () const = 0;

            virtual bool Login (const LoginParameters &params) = 0;
            virtual bool Logout () = 0;

            virtual InputStreamInterface& GetInputStream () = 0;
            virtual OutputStreamInterface& GetOutputStream () = 0;
    };
}

#endif
