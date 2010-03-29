// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworking_LLInputStream_h
#define incl_RexNetworking_LLInputStream_h

#include "ModuleLoggingFunctions.h"
#include "LLMessageManager/LLMessageManager.h"
#include "StreamInterface.h"

#include <QObject>

#include <tr1/functional>
using std::tr1::function;

namespace RexNetworking
{
    class LLInputStream : public QObject, public Foundation::InputStreamInterface
    {
        Q_OBJECT

        public:
            MODULE_LOGGING_FUNCTIONS;
            static std::string NameStatic() { return "LLInputStream"; }

            LLInputStream ();
            virtual ~LLInputStream();

            bool IsConnected() const { return connected_; }

        public:
            // C++-style callback registering
            void OnConnect (function <void (void)> handler);
            void OnDisconnect (function <void (void)> handler);
            void OnRegionHandshake (function <void (LLInMessage*)> handler);
            void OnAgentMovementComplete (function <void (LLInMessage*)> handler);
            void OnAvatarAnimation (function <void (LLInMessage*)> handler);
            void OnGenericMessage (function <void (LLInMessage*)> handler);
            void OnLogoutReply (function <void (LLInMessage*)> handler);
            void OnImprovedTerseObjectUpdate (function <void (LLInMessage*)> handler);
            void OnKillObject (function <void (LLInMessage*)> handler);
            void OnObjectUpdate (function <void (LLInMessage*)> handler);
            void OnObjectProperties (function <void (LLInMessage*)> handler);
            void OnAttachedSound (function <void (LLInMessage*)> handler);
            void OnAttachedSoundGainChange (function <void (LLInMessage*)> handler);
            void OnSoundTrigger (function <void (LLInMessage*)> handler);
            void OnPreloadSound (function <void (LLInMessage*)> handler);
            void OnScriptDialog (function <void (LLInMessage*)> handler);

        signals:
            // Qt-style signals
            void OnConnect ();
            void OnDisconnect ();
            void OnRegionHandshake (LLInMessage* msg);
            void OnAgentMovementComplete (LLInMessage* msg);
            void OnAvatarAnimation (LLInMessage* msg);
            void OnGenericMessage (LLInMessage* msg);
            void OnLogoutReply (LLInMessage* msg);
            void OnImprovedTerseObjectUpdate (LLInMessage* msg);
            void OnKillObject (LLInMessage* msg);
            void OnObjectUpdate (LLInMessage* msg);
            void OnObjectProperties (LLInMessage* msg);
            void OnAttachedSound (LLInMessage* msg);
            void OnAttachedSoundGainChange (LLInMessage* msg);
            void OnSoundTrigger (LLInMessage* msg);
            void OnPreloadSound (LLInMessage* msg);
            void OnScriptDialog (LLInMessage* msg);

        private:
            /// Is client connected to a server.
            bool connected_;

            /// LLUDP Packet manager
            LLMessageManager *messagemgr_;
    };
}

#endif // incl_RexNetworking_LLInputStream_h
