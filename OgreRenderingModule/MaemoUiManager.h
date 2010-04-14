// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_MaemoUiManager_h
#define incl_OgreRenderer_MaemoUiManager_h

#include "Foundation.h"

#include <QObject>
//#include <Qt/QtMaemo5>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QPushButton>

namespace OgreRenderer
{
    class MaemoUiManager : public QObject
    {

    Q_OBJECT
    
    public:
        MaemoUiManager(Foundation::Framework *framework, QMainWindow *main_window);
        
    public slots:
        void Initialize();
        
    private slots:
        void TryConnect();
        void ToggleViewMode();
        
    private:
        Foundation::Framework *framework_;
        event_category_id_t renderer_category_;
        
        QMainWindow *main_window_;
        
        QPushButton *connect_button_;
        QPushButton *viewmode_button_;
        
    };
}

#endif
