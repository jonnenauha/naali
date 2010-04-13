// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_SessionManager_h
#define incl_Foundation_SessionManager_h

#include "CoreTypes.h"
#include "Session.h"
#include "Framework.h"

namespace Foundation
{
    typedef std::map <int, Foundation::Session*> SessionMap;

    //! Container for the functions that create Sessions at login or 
    //! finalizes them at logout
    struct SessionHandler
    {
        virtual bool Accepts (const Session::LoginParameters &params) = 0;
        virtual Session *Login (const Session::LoginParameters &params) = 0;

        virtual bool Owns (const Session *session) = 0;
        virtual bool Logout () = 0;

        virtual Session *GetSession () = 0;

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

            int GetType (const Session *session) const;
            int GetType (const std::string &type) const;
            SessionHandler *GetHandler (const std::string &type) const;

            Session *Login (const Session::LoginParameters &params);

            bool Logout (const Session *session);
            void LogoutAll ();

            const SessionMap &ActiveSessions () const;

         private:
            typedef std::vector <SessionHandler *>      SessionHandlerList;
            typedef std::map <std::string, int>         SessionTypeMap;

            SessionHandlerList  session_handlers_;
            SessionTypeMap      session_types_;
            SessionMap          active_;

         private:
            SessionHandler *get_accepted (const Session::LoginParameters &params) const;
            SessionHandler *get_owner (const Session *session) const;
            SessionHandler *get_handler (const std::string &type) const;

            int get_session_type_id (const std::string &type);
            int session_type_id_;

            Framework *framework_;
    };
}

#endif
