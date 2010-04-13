/* llparameters.h -- parameters for LL protocols
 *
 */

#ifndef incl_LLParameters_h
#define incl_LLParameters_h

#include <QString>
#include <QList>
#include <QMap>
#include <QUrl>

#include "RexUUID.h"

namespace RexNetworking
{

    //=========================================================================
    // LL Buddies

    struct LLBuddy
    {
        QString buddy_id;
        int buddy_rights_given;
        int buddy_rights_has;
    };

    struct LLBuddyList
    {
        QList <LLBuddy> list;
    };

    //=========================================================================
    // LL Inventory Skeleton

    struct LLFolderSkeleton
    {
        QString name;
        QString folder_id;
        QString parent_id;
        int type_default;
        int version;
    };

    struct LLInventorySkeleton
    {
        QString owner;
        LLFolderSkeleton root;
        QList <LLFolderSkeleton> folders;
    };

    //=========================================================================
    // Parameters used in LLStream

    struct LLStreamParameters
    {
        RexUUID agent_id;
        RexUUID session_id;
        RexUUID region_id;
        int circuit_code;
    };

    //=========================================================================
    // Parameters used in LLSession

    struct LLSessionParameters
    {
        QString message;

        QString sim_name;
        QString sim_ip;
        int sim_port;

        QUrl seed_capabilities;
        QMap <QString, QUrl> capabilities;
    };


    //=========================================================================
    // Parameters used by LLAgent

    struct LLAgentParameters
    {
        QString first_name;
        QString last_name;

        LLBuddyList buddies;
        LLInventorySkeleton inventory;

        QString home;
        QString look_at;
        QString start_location;
        int region_x;
        int region_y;
    };

    //=========================================================================
    // Parameters used by LLLogin

    struct LLLoginParameters
    {
        QString first;
        QString last;
        QString pass;
        QUrl service;
    };
 
}
#endif
