// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_SessionManager_h
#define incl_Foundation_SessionManager_h

#include "CoreTypes.h"
#include "SessionInterface.h"
#include "Framework.h"

namespace Foundation
{
    //! Container for the functions that create Sessions at login or 
    //  finalizes them at logout
    struct SessionHandler
    {
        virtual bool Accepts (const LoginParameters &params) = 0;
        virtual SessionInterface *Login (const LoginParameters &params) = 0;

        virtual bool Owns (const SessionInterface *session) = 0;
        virtual bool Logout () = 0;

        int priority;
        int type;
    };

    //! Manages the lifetime of world sessions
    //! Types are used to discriminate on protocol types the sessions implement
    class SessionManager
    {
        public:
            SessionManager (Framework *framework);
            ~SessionManager ();

            bool Register (std::auto_ptr <SessionHandler> handler, const std::string &type);

            int GetType (const std::string &type) const;
            int GetType (const SessionInterface *session) const;

            SessionInterface *Login (const LoginParameters &params);

            bool Logout (const SessionInterface *session);
            void LogoutAll ();

         private:
            typedef std::vector <SessionHandler *>      SessionHandlerList;
            typedef std::map <std::string, int>         SessionTypeMap;

            SessionHandlerList  session_handlers_;
            SessionTypeMap      session_types_;

         private:
            SessionHandler *get_accepted (const LoginParameters &params) const;
            SessionHandler *get_owner (const SessionInterface *session) const;

            int get_session_type_id (const std::string &type);
            int session_type_id_;

            Framework *framework_;
    };
}

#endif
