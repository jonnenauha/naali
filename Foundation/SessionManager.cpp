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

    int SessionManager::GetType (const string &type) const
    {
        SessionTypeMap::const_iterator s = session_types_.find (type);
        return (s != session_types_.end())? s-> second : -1;
    }

    int SessionManager::GetType (const Session *session) const
    {
        SessionHandler *owner = get_owner (session);
        return owner-> type;
    }

    Session *SessionManager::Login (const Session::LoginParameters &params)
    {
        SessionHandler *accepted = get_accepted (params);
        return (accepted)? accepted-> Login (params) : 0;
    }

    bool SessionManager::Logout (const Session *session)
    {
        SessionHandler *owner = get_owner (session);
        return (owner)? owner-> Logout () : false;
    }

    void SessionManager::LogoutAll ()
    {
        SessionHandlerList::iterator i = session_handlers_.begin();
        SessionHandlerList::iterator e = session_handlers_.end();
        for (; i != e; ++i) (*i)-> Logout ();
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
}
