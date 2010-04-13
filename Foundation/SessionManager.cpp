// For conditions of distribution and use, see copyright notice in license.txt

#include "SessionManager.h"

namespace Foundation
{
    using std::string;
    using std::auto_ptr;

    SessionManager::SessionManager (Framework *framework)
        : session_type_id_ (1), framework_ (framework)
    {}

    SessionManager::~SessionManager ()
    {
        LogoutAll();
    }

    bool SessionManager::Register (auto_ptr <SessionHandler> handl, const std::string &type)
    {
        handl-> type = get_session_type_id (type);
        session_handlers_.push_back (handl.release());
    }

    int SessionManager::GetType (const Session *session) const
    {
        SessionHandler *owner = get_owner (session);
        return owner-> type;
    }

    int SessionManager::GetType (const string &type) const
    {
        SessionTypeMap::const_iterator i = session_types_.find (type);
        return (i != session_types_.end())? i-> second : -1;
    }

    SessionHandler *SessionManager::GetHandler (const std::string &type) const
    {
        return get_handler (type);
    }

    Session *SessionManager::Login (const Session::LoginParameters &params)
    {
        SessionHandler *accepted = get_accepted (params);
        Session *session = (accepted)? accepted-> Login (params) : 0;

        if (session) active_.insert (std::make_pair (session-> Type(), session));

        return session;
    }

    bool SessionManager::Logout (const Session *session)
    {
        SessionHandler *owner = get_owner (session);
        bool success = (owner)? owner-> Logout () : false;

        if (success) active_.erase (session-> Type());

        return success;
    }

    void SessionManager::LogoutAll ()
    {
        SessionHandlerList::iterator i = session_handlers_.begin();
        SessionHandlerList::iterator e = session_handlers_.end();
        for (; i != e; ++i) (*i)-> Logout ();
    }
            
    const SessionMap &SessionManager::ActiveSessions () const
    {
        return active_;
    }

    int SessionManager::get_session_type_id (const string &type)
    {
        using std::make_pair; 

        if (!session_types_.count (type))
            session_types_.insert (make_pair (type, session_type_id_++));

        return session_types_[type];
    }

    SessionHandler *SessionManager::get_accepted (const Session::LoginParameters &p) const
    {
        SessionHandler *accepted = 0;

        SessionHandlerList::const_iterator i = session_handlers_.begin();
        SessionHandlerList::const_iterator e = session_handlers_.end();

        for (; i != e; ++i)
            if ((*i)-> Accepts (p))
            {
                accepted = *i;
                break;
            }

        return accepted;
    }

    SessionHandler *SessionManager::get_owner (const Session *session) const
    {
        SessionHandler *owner = 0;

        SessionHandlerList::const_iterator i = session_handlers_.begin();
        SessionHandlerList::const_iterator e = session_handlers_.end();

        for (; i != e; ++i)
            if ((*i)-> Owns (session))
            {
                owner = *i;
                break;
            }

        return owner;
    }

    SessionHandler *SessionManager::get_handler (const std::string &type) const
    {
        SessionHandler *handler = 0;

        SessionHandlerList::const_iterator i = session_handlers_.begin();
        SessionHandlerList::const_iterator e = session_handlers_.end();
        int t = GetType (type);

        for (; i != e; ++i)
            if ((*i)-> type == t)
            {
                handler = *i;
                break;
            }
                
        return handler;
    }
}
