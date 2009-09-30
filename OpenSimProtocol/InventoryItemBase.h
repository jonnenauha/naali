// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryItemBase.h
 *  @brief Base class for inventory items.
 */
 
#ifndef InventoryItemBase_h
#define InventoryItemBase_h

#include "OpenSimProtocolModuleApi.h"
#include "RexUUID.h"

#include <QModelIndex>

namespace OpenSimProtocol
{
    enum InventoryItemType
    {
        Type_Folder = 0,
        Type_Asset,
        Type_Unknown
    };

    class InventoryFolder;

    class OSPROTO_MODULE_API InventoryItemBase
    {
    public:
        /// Default constructor.
        InventoryItemBase();

        /// Constructor.
        /// @param type Type (folder or item)
        /// @param id ID.
        /// @param type name Name.
        /// @param type parent Parent folder.
        /// @param type editable Is this item editable.
        /// @param type index Tree model index.
        InventoryItemBase(
            const InventoryItemType &type,
            const RexTypes::RexUUID &id,
            const std::string &name,
            const bool &editable = true,
            InventoryFolder *parent = 0,
            const QModelIndex &index = QModelIndex());

        /// Destructor.
        virtual ~InventoryItemBase() {}

        /// Copy constructor.
        InventoryItemBase(const InventoryItemBase &rhs);

        /// Assign operator.
        InventoryItemBase &operator =(const InventoryItemBase &rhs);

        /// Sets the name.
        /// @param name Name.
        void SetName(const std::string name) { name_ = name; }

        /// @return Name.
        const std::string &GetName() const { return name_; }

        /// Sets the ID.
        /// @param id ID.
        void SetID(const RexTypes::RexUUID &id) { id_ = id; }

        /// @return ID.
        const RexTypes::RexUUID &GetID() const { return id_; }

        /// @return Type.
        const InventoryItemType &GetInventoryItemType() const { return itemType_; }

        /// @return Parent.
        void SetParent(InventoryFolder *folder) { parent_ = folder; }

        /// @return Parent.
        InventoryFolder *GetParent() const { return parent_; }

        /// @param editable Can this item be edited or not.
        void SetEditable(const bool &editable) { editable_ = editable; }

        /// @return Can user edit this item.
        const bool IsEditable() const { return editable_; }

    protected:
        /// Type of the tree item.
        InventoryItemType itemType_;

        /// ID.
        RexTypes::RexUUID id_;

        /// Name.
        std::string name_;

        /// Parent.
        InventoryFolder *parent_;

        ///
        QModelIndex index_;

        /// Is this item editable.
        bool editable_;

       /// 
    };
}

#endif