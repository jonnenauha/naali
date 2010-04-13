// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_UiEventHandler_h
#define incl_RexLogic_UiEventHandler_h

#include <QObject>
#include <QMap>
#include "ForwardDefines.h"

namespace RexLogic
{
    // Simple glue that Listens to UI events (Qt signals from UiModule) to handle login/out
    class UiLoginHandler : QObject
    {
        Q_OBJECT

        public:
            UiLoginHandler (Foundation::SessionManagerPtr manager, QObject *subject);

        public slots:
            void Login (QMap <QString,QString> params);
            void Logout ();

        private:
            Foundation::SessionManagerPtr   sessionmgr_;
    };
}

#endif

