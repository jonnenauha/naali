/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.h
 *  @brief  EC_ChatBubble 
*/

#ifndef incl_EC_ChatBubble_ECBillBoard_h
#define incl_EC_ChatBubble_ECBillBoard_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QStringList>
#include <QFont>

/*
namespace Scene
{
    class Entity;
}

namespace Ogre
{
    class SceneManager;
}
*/

class EC_ChatBubble : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_ChatBubble);

private:
    /// Constuctor.
    /// @param module Module.
    explicit EC_ChatBubble(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_ChatBubble();

    /// @param offset Offset position for the chat bubble. Entity's positon is used as the basis.
    /// @param imageName Name of the image file. Note the image must be located in the OGRE resource group.
    void Show(const Vector3df &offset, float timeToShow, const char *imageName);

    ///
    /// @param
    void SetFont(const QFont &font);

public slots:
    /// @param msg Message to be shown.
    void ShowMessage(const QString &msg);

private slots:
    /// Removes the last message.
    void RemoveLastMessage();

    /// Removes all the messages.
    void RemoveAllMessages();

    ///
    void Refresh();

private:
    /// Test code for getting pixmap with chat bubble image and text
    /// @param filename Filename for the image used as the basis for the chat bubble.
    /// @param text Text to be shown in the chat bubble.
    /// @return the rendered pixmap with image and embedded text or null if something goes wrong.
    QPixmap GetPixmap(const QString &image_name, const QString &text);

    ///
    QPixmap GetPixmap(const QString &text);

    /// For used for the chat bubble text.
    QFont font_;

    /// List of visible chat messages.
    QStringList messages_;
};

#endif