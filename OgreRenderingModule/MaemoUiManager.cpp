// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MaemoUiManager.h"

#include "EventManager.h"
#include "RendererEvents.h"

#include <QTimer>
#include <QDebug>

namespace OgreRenderer
{
    MaemoUiManager::MaemoUiManager(Foundation::Framework *framework, QMainWindow *main_window) :
        QObject(0),
        framework_(framework),
        main_window_(main_window)
    {
        renderer_category_ = framework_->GetEventManager()->QueryEventCategory("Renderer");        
        Initialize();
    }
    
    void MaemoUiManager::Initialize()
    {
        main_window_->setAttribute(Qt::WA_Maemo5StackedWindow);

        viewmode_button_ = new QPushButton("viewmode", main_window_);
        viewmode_button_->resize(125, 40);
        viewmode_button_->move(0,40);
        viewmode_button_->hide();
        
        connect_button_ = new QPushButton("connect", main_window_);
        connect_button_->resize(125, 40);
        connect_button_->move(0,0);
        connect_button_->show();

        QAction *fullscreen = new QAction("Full Screen", main_window_);
        QAction *exit = new QAction("Exit", main_window_);

        main_window_->menuBar()->addAction(fullscreen);
        main_window_->menuBar()->addAction(exit);
       
        connect(connect_button_, SIGNAL(clicked()), SLOT(TryConnect()));
        connect(viewmode_button_, SIGNAL(clicked()), SLOT(ToggleViewMode()));
        connect(fullscreen, SIGNAL(triggered()), SLOT(ToggleViewMode()));
        connect(exit, SIGNAL(triggered()), main_window_, SLOT(close()));
        
        qDebug() << "MaemoUiManager Initialized";
    }

    void MaemoUiManager::ToggleViewMode()
    {
        connect_button_->show();
        if (main_window_->isFullScreen())
        {
            main_window_->showNormal();
            viewmode_button_->hide();
        }
        else
        {
            main_window_->showFullScreen();
            viewmode_button_->show();
        }
        
        qDebug() << "Viewmode changed to fullscreen = " << main_window_->isFullScreen();
    }

    void MaemoUiManager::TryConnect()
    {
        QMap<QString, QString> cred_map;
        cred_map["WorldAddress"] = "http://home.hulkko.net:9007";
        cred_map["Username"] = "N900 Pforce";
        cred_map["Password"] = "noauth";
        
        qDebug() << "Connecting with: " << cred_map;
        
        Events::MaemoLoginRequest data(cred_map);
        framework_->GetEventManager()->SendEvent(renderer_category_, Events::MAEMO_LOGIN_REQUEST, &data);
    }
}
