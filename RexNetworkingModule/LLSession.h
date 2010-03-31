/* llsession.h -- llsession implementation
 *
 */

#ifndef incl_LLSession_h
#define incl_LLSession_h

#include <QVariantMap>

#include "SessionManager.h"
#include "SessionInterface.h"

#include "XmlRpc.h"
#include "Capabilities.h"
#include "LLParameters.h"
#include "LLStream.h"

class QNetworkAccessManager;
class QNetworkReply;


namespace RexNetworking
{
    class LLSession;

    //=========================================================================
    // Login object for LLSession

    class LLLogin : public QObject
    {
        Q_OBJECT

        public:
            LLLogin (LLSession *session);
            bool operator() (const LLLoginParameters &params);

        public slots:
            void on_login_result ();
            void on_caps_result ();

        private:
            LLSession               *session_;

            XmlRpc::Call            *login_;
            Capabilities::Caps      *caps_;
            QNetworkAccessManager   *http_;
    };

    //=========================================================================
    // Login object for LLSession

    class LLLogout
    {
        public:
            LLLogout (LLSession *sesson);
            bool operator() ();

        private:
            LLSession   *session_;
    };

    //=========================================================================
    // LL Session 

    class LLSession : public Foundation::SessionInterface
    {
        public:
            LLSession (int type);
            ~LLSession ();

            bool Login (const Foundation::LoginParameters &params);
            bool Logout ();

            int Type () const { return type_; }
            bool IsConnected() const { return connected_; }

            Foundation::StreamInterface& Stream ();

            LLAgentParameters   GetAgentParameters () { return agentparam_; } 
            LLSessionParameters GetSessionParameters () { return sessionparam_; }
            LLStreamParameters  GetStreamParameters () { return streamparam_; }

        private:
            bool    connected_;
            int     type_;
        
            LLLogin     login_;
            LLLogout    logout_;

            LLAgentParameters   agentparam_;
            LLSessionParameters sessionparam_;
            LLStreamParameters  streamparam_;

            LLStream  stream_;

            friend class LLLogin;
            friend class LLLogout;
    };
    
    //=========================================================================
    // Handler for LLSession

    class LLSessionHandler : public Foundation::SessionHandler
    {
        public:
            LLSessionHandler ();
            virtual ~LLSessionHandler ();

            bool Accepts (const Foundation::LoginParameters &params);
            Foundation::SessionInterface *Login (const Foundation::LoginParameters &params);

            bool Owns (const Foundation::SessionInterface *session);
            bool Logout ();

            LLSession *GetSession();

        private:
            LLSession   *session_;
    };
}
#endif
