// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"
#include "SessionManager.h"
#include "Session.h"

#include "UiEventHandler.h"

namespace RexLogic
{
    using Foundation::SessionManager;
    using Foundation::SessionManagerPtr;
    using Foundation::Session;

    UiLoginHandler::UiLoginHandler (SessionManagerPtr mgr, QObject *sub)
        : sessionmgr_ (mgr)
    {
         connect(sub, SIGNAL(StartOsLogin(QMap<QString,QString>)), 
                 SLOT(Login(QMap<QString,QString>)));

        connect(sub, SIGNAL(StartRexLogin(QMap<QString,QString>)), 
                SLOT(Login(QMap<QString,QString>)));

        connect(sub, SIGNAL(Quit()), SLOT(Logout()));
    }

    void UiLoginHandler::Login (QMap <QString,QString> params)
    {
        sessionmgr_-> Login (params);
    }

    void UiLoginHandler::Logout ()
    {
        sessionmgr_-> LogoutAll ();
    }
}
