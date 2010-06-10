// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ClassicalLoginWidget_h
#define incl_UiModule_ClassicalLoginWidget_h

#include <QWidget>
#include <QMap>
#include <QHideEvent>

#include "Ether/EtherLoginNotifier.h"
#include "ui_ClassicalLoginWidget.h"

namespace CoreUi
{
    namespace Classical
    {
        class TraditionalLoginWidget;
        class WebLoginWidget;

        class ClassicalLoginWidget : public QWidget, private Ui::ClassicalLoginWidget
        {
            Q_OBJECT

        public:
            ClassicalLoginWidget(Ether::Logic::EtherLoginNotifier *login_notifier, QMap<QString,QString> stored_login_data);
            ~ClassicalLoginWidget();

        public slots:
            void RemoveEtherButton();
            QMap<QString, QString> GetLoginInfo();
            void AppExitRequest();
            void StatusUpdate(bool connecting, QString message);

        protected:
            void hideEvent(QHideEvent *hide_event);

        private:
            Ether::Logic::EtherLoginNotifier *login_notifier_;
            TraditionalLoginWidget *traditional_widget_;
            WebLoginWidget *web_login_;

        signals:
            void AppExitRequested();
            void StatusUpdateToUi(QString);
            void ClassicLoginHidden();
        };
    }
}

#endif
