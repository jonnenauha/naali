// For conditions of distribution and use, see copyright notice in license.txt

//#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "Profiler.h"

#include "LLSession.h"

#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>


namespace RexNetworking
{
    using Foundation::Session;
   

    //=========================================================================
    // pretty printers
    
    void variant_print (const QVariant &v, const QString &p);
    void variant_print (const QVariantMap &m, const QString &p);
    void variant_print (const QVariantList &l, const QString &p);
    void variant_print (const QStringList &l, const QString &p);
    void variant_print (const QString &s, const QString &p);
    void variant_print (bool v, const QString &p);
    void variant_print (int v, const QString &p);

    void variant_print (const QVariant &v, const QString &p)
    {
        QString pre (p + " ");
        switch (v.type())
        {
            case QVariant::Bool:
                variant_print (v.toBool(), pre);
                break;

            case QVariant::List:
                variant_print (v.toList(), pre);
                break;

            case QVariant::StringList:
                variant_print (v.toStringList(), pre);
                break;

            case QVariant::Map:
                variant_print (v.toMap(), pre);
                break;

            case QVariant::String:
                variant_print (v.toString(), pre);
                break;

            case QVariant::Int:
                variant_print (v.toInt(), pre);
                break;

            default:
                std::cout << "no handler for type: " << v.type() << std::endl;
        }
    }

    void variant_print (const QVariantMap &m, const QString &p)
    {
        QString pre (p + " ");
        foreach (QString s, m.keys())
        {
            std::cout << qPrintable (pre) << "(M)" << qPrintable (s) << ": ";
            variant_print (m[s], pre);
            std::cout << "\n";
        }
    }

    void variant_print (const QVariantList &l, const QString &p)
    {
        QString pre (p + " ");
        foreach (QVariant v, l)
        {
            std::cout << qPrintable (pre) << "(L)";
            variant_print (v, pre);
            std::cout << "\n";
        }
    }

    void variant_print (const QStringList &l, const QString &p)
    {
        QString pre (p + " ");
        foreach (QString s, l)
        {
            std::cout << qPrintable (pre) << "(L)";
            variant_print (s, pre);
            std::cout << "\n";
        }
    }

    void variant_print (const QString &s, const QString &p)
    {
        QString pre (p + " ");
        std::cout << qPrintable (pre) << "(S) " << qPrintable (s);
    }

    void variant_print (bool v, const QString &p)
    {
        QString pre (p + " ");
        std::cout << qPrintable (pre) << "(B) " << v;
    }

    void variant_print (int v, const QString &p)
    {
        QString pre (p + " ");
        std::cout << qPrintable (pre) << "(I) " << v;
    }
 
    //=========================================================================
    // parsers for LLLogin

    static LLAgentParameters parse_agent_params (const QVariantMap &m);
    static LLStreamParameters parse_stream_params (const QVariantMap &m);
    static LLSessionParameters parse_session_params (const QVariantMap &m);
    static LLBuddyList parse_buddies (const QVariantMap &m);
    static LLBuddyList parse_buddy_list (const QVariantList &l);
    static LLBuddy parse_buddy (const QVariantMap &m);
    static LLInventorySkeleton parse_inventory (const QVariantMap &m);
    static LLInventorySkeleton parse_inventory_skeleton (const QVariantMap &m);
    static LLFolderSkeleton parse_root_folder (const QVariantList &l);
    static LLFolderSkeleton parse_folder_skeleton (const QVariantMap &m);
    static LLLoginParameters parse_login_params (const Session::LoginParameters &p);


    static LLAgentParameters parse_agent_params (const QVariantMap &m)
    {
        LLAgentParameters params;

        params.buddies = parse_buddies (m);
        params.inventory = parse_inventory (m);

        params.first_name = m["first_name"].toString();
        params.last_name = m["last_name"].toString();

        params.home = m["home"].toString();
        params.look_at = m["look_at"].toString();
        params.start_location = m["start_location"].toString();
        params.region_x = m["region_x"].toInt();
        params.region_y = m["region_y"].toInt();

        return params;
    }

    static LLStreamParameters parse_stream_params (const QVariantMap &m)
    {
        LLStreamParameters params;

        params.circuit_code = m["circuit_code"].toInt();
        params.agent_id.FromString (m["agent_id"].toString().toStdString());

        //;if (!m["session_id"].toString().isNull())
            params.session_id.FromString (m["session_id"].toString().toStdString());
        //else
        //    params.session_id.FromString (m["secure_session_id"].toString().toStdString());

        return params;
    }

    static LLSessionParameters parse_session_params (const QVariantMap &m)
    {
        LLSessionParameters params;

        params.seed_capabilities = m["seed_capability"].toString();
        params.sim_ip = m["sim_ip"].toString();
        params.sim_port = m["sim_port"].toInt();
        params.message = m["message"].toString();

        return params;
    }

    static LLBuddyList parse_buddies (const QVariantMap &m)
    {
        return parse_buddy_list (m["buddy-list"].toList());
    }

    static LLBuddyList parse_buddy_list (const QVariantList &l)
    {
        LLBuddyList b;
        foreach (QVariant v, l)
            b.list.push_back (parse_buddy (v.toMap()));
        return b;
    }

    static LLBuddy parse_buddy (const QVariantMap &m)
    {
        LLBuddy b;
        b.buddy_id = m["buddy_id"].toString();
        b.buddy_rights_given = m["buddy_rights_given"].toInt();
        b.buddy_rights_has = m["buddy_rights_has"].toInt();
        return b;
    }

    static LLInventorySkeleton parse_inventory (const QVariantMap &m)
    {
        return parse_inventory_skeleton (m);
        // skip "opensim library"
    }

    static LLInventorySkeleton parse_inventory_skeleton (const QVariantMap &m)
    {
        LLInventorySkeleton is;

        is.root = parse_root_folder (m ["inventory-root"].toList());
        foreach (QVariant v, m["inventory-skeleton"].toList())
        {
            is.folders.push_back (parse_folder_skeleton (v.toMap()));

            if (is.folders.back().folder_id == is.root.folder_id) 
                is.root = is.folders.back();
        }

        return is;
    }

    static LLFolderSkeleton parse_root_folder (const QVariantList &l)
    {
        LLFolderSkeleton f;
        f.folder_id = l.front().toMap()["folder_id"].toString();
        return f;
    }

    static LLFolderSkeleton parse_folder_skeleton (const QVariantMap &m)
    {
        LLFolderSkeleton f;
        f.name = m["name"].toString();
        f.folder_id = m["folder_id"].toString();
        f.parent_id = m["parent_id"].toString();
        f.type_default = m["type_default"].toInt();
        f.version = m["version"].toInt();
        return f;
    }

    static LLLoginParameters parse_login_params (const Session::LoginParameters &p)
    {
        // TODO
        LLLoginParameters params;
        params.first = "d"; params.last = "d"; params.pass = "d";
        params.service = "http://home.hulkko.net:9007";
        return params;
    }

    //=========================================================================
    // Login object for LLSession

    LLLogin::LLLogin (LLSession *session) : 
        session_ (session), login_ (0), caps_ (0),
        http_ (new QNetworkAccessManager (this)) 
    {}

    bool LLLogin::operator() (const LLLoginParameters &params)
    {
        // compose formal arguments from parameters
        QVariantMap args;
        args["first"] = params.first;
        args["last"] = params.last;
        args["passwd"] = QString::fromAscii
            ("$1$"+QCryptographicHash::hash
             (params.pass.toAscii(), QCryptographicHash::Md5).toHex());

        args["mac"] = "00:00:00:00:00:00"; // TODO
        args["id0"] = "00:00:00:00:00:00"; // TODO
        args["options"] = (QStringList()
                << "inventory-root"
                << "inventory-skeleton"
                << "inventory-lib-root"
                << "inventory-lib-owner"
                << "inventory-skel-lib"
                << "initial-outfit"
                << "gestures"
                << "event_categories"
                << "event_notifications"
                << "classified_categories"
                << "buddy-list"
                << "ui-config"
                << "tutorial_settings"
                << "login-flags"
                << "global-textures");
        args["read_critical"] = true;
        args["viewer_digest"] = "";
        args["channel"] = "channel"; // TODO
        args["version"] = "version"; // TODO
        args["platform"] = "platform"; // TODO
        args["agree_to_tos"] = true;
        args["last_exec_event"] = 0;
        args["start"] = "last";

        std::cout << "server: " << qPrintable (params.service.toString()) << std::endl;

        // make login call
        XmlRpc::Client client (params.service, http_);
        login_ = client.call ("login_to_simulator", (QVariantList() << args));

        connect (login_-> reply, SIGNAL (finished()), 
                this, SLOT (on_login_result()));

        return session_;
    }

    void LLLogin::on_login_result ()
    {
        if (login_-> reply-> error() == QNetworkReply::NoError)
        {
            QVariantMap response (login_-> result.toMap ());

            variant_print (response, " ");

            if (response["login"] == "true")
            {
                // parse login return parameters
                session_-> streamparam_ = parse_stream_params (response);
                session_-> sessionparam_ = parse_session_params (response);
                session_-> agentparam_ = parse_agent_params (response);

                // sanity checking
                if ((session_-> streamparam_.circuit_code == 0) ||
                    (session_-> streamparam_.session_id.IsNull()) ||
                    (session_-> streamparam_.agent_id.IsNull()))
                    std::cout << "session login error: stream parameters incorrect" << std::endl;

                // set up world stream 
                session_-> stream_.SetParameters (session_-> streamparam_);
                session_-> stream_.Connect 
                    (session_-> sessionparam_.sim_ip.toStdString(), 
                     session_-> sessionparam_.sim_port);

                // get capabilities
                QUrl seedcap = session_-> sessionparam_.seed_capabilities;
                Capabilities::Client client (seedcap, http_);
                caps_ = client.request ();

                connect (caps_-> reply, SIGNAL (finished()), 
                        this, SLOT (on_caps_result()));
            }
        }
        else
            std::cout << "xmlrpc login error: " << login_-> reply-> error() << std::endl;

        delete login_; login_ = 0;
    }

    void LLLogin::on_caps_result ()
    {
        if (caps_-> reply-> error() == QNetworkReply::NoError)
        {
            // get caps
            session_-> sessionparam_.capabilities = caps_-> result;

            // update future
            session_-> connected_ = true;
        }
        else
            std::cout << "seed caps error: " << caps_-> reply-> error() << std::endl;

        delete caps_; caps_ = 0;
    }

    //=========================================================================
    // Logout object for LLSession

    LLLogout::LLLogout (LLSession *session) :
        session_ (session)
    {
    }

    bool LLLogout::operator() ()
    {
        session_-> stream_.Disconnect();
        return true;
    }

    //=========================================================================
    // LLSession

    LLSession::LLSession (int type) : 
        connected_ (false), type_ (type),
        login_ (this), logout_ (this)
    {}

    LLSession::~LLSession ()
    {
        stream_.Disconnect();
    }

    bool LLSession::Login (const LoginParameters &params) 
    { 
        return login_ (parse_login_params (params)); 
    }

    bool LLSession::Logout () 
    { 
        return logout_ (); 
    }

    Foundation::Stream& LLSession::GetStream ()
    {
        return stream_;
    }

    //=========================================================================
    // Handler for LLSession

    LLSessionHandler::LLSessionHandler () :
        session_ (0)
    {}

    LLSessionHandler::~LLSessionHandler ()
    {
        delete session_;
    }

    Session *LLSessionHandler::GetSession ()
    {
        if (!type) return 0;
        if (!session_) session_ = new LLSession (type);
        return session_;
    }

    bool LLSessionHandler::Accepts (const Session::LoginParameters &params) 
    { 
        return true; // TODO: sniff login params
    } 

    Session *LLSessionHandler::Login (const Session::LoginParameters &params) 
    { 
        return (GetSession()-> Login (params))? session_ : 0;
    }

    bool LLSessionHandler::Owns (const Session *session)
    {
        return (GetSession() == session);
    }

    bool LLSessionHandler::Logout () 
    { 
        return GetSession()-> Logout (); 
    }

    void LLSessionHandler::SetStreamHandlers (const LLStream::MessageHandlerMap &map)
    {
        static_cast <LLStream &> (GetSession()-> GetStream()).SetHandlers (map);
    }
}
