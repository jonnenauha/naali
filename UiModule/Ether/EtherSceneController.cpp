// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EtherSceneController.h"
#include "EtherLoginNotifier.h"
#include "Data/DataManager.h"
#include "Data/AvatarInfo.h"

#include "View/EtherScene.h"
#include "View/InfoCard.h"
#include "View/EtherMenu.h"
#include "View/ActionProxyWidget.h"
#include "View/ControlProxyWidget.h"
#include "View/Classical/ClassicalLoginWidget.h"

#include "Common/AnchorLayoutManager.h"

#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QImage>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Logic
    {
        EtherSceneController::EtherSceneController(QObject *parent, Data::DataManager *data_manager, View::EtherScene *scene, 
                                                   QPair<View::EtherMenu*, View::EtherMenu*> menus, QRectF card_size, int top_items, int bottom_items)
            : QObject(parent),
              data_manager_(data_manager),
              scene_(scene),
              active_menu_(0),
              top_menu_(menus.first),
              bottom_menu_(menus.second),
              top_menu_visible_items_(top_items),
              bottom_menu_visible_items_(bottom_items),
              menu_cap_size_(10),
              login_animations_(new QParallelAnimationGroup(this)),
              card_size_(card_size),
              last_active_top_card_(0),
              last_active_bottom_card_(0),
              connected_world_(0),
              layout_manager_(new CoreUi::AnchorLayoutManager(this, scene)),
              info_hide_timer_(new QTimer(this)),
              login_in_progress_(false),
              classical_login_widget_(0)
        {
            // Connect key press signals from scene
            connect(scene_, SIGNAL( UpPressed() ), SLOT( UpPressed() ));
            connect(scene_, SIGNAL( DownPressed() ), SLOT( DownPressed() ));
            connect(scene_, SIGNAL( RightPressed() ), SLOT( RightPressed() ));
            connect(scene_, SIGNAL( LeftPressed() ), SLOT( LeftPressed() ));
            connect(scene_, SIGNAL( EnterPressed() ), SLOT( TryStartLogin() ));

            // Hide timer for status widget
            info_hide_timer_->setSingleShot(true);
            connect(info_hide_timer_, SIGNAL(timeout()), SLOT(HideStatusWidget()));

            // Connect item clicked signal
            connect(scene_, SIGNAL( ItemClicked(View::InfoCard*) ),
                    this, SLOT( ItemActivatedWithMouse(View::InfoCard*) ));

            // Connect resize signal from scene
            connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ),
                    this, SLOT( SceneRectChanged(const QRectF &) ));

            // Connect highlight signals from menus
            connect(top_menu_, SIGNAL( ItemHighlighted(View::InfoCard*) ),
                    this, SLOT( ActiveItemChanged(View::InfoCard*) ));
            connect(bottom_menu_, SIGNAL( ItemHighlighted(View::InfoCard*) ),
                    this, SLOT( ActiveItemChanged(View::InfoCard*) ));

            // Connect login animations finished signal
            connect(login_animations_, SIGNAL( finished() ),
                    this, SLOT( LoginAnimationFinished() ));

            // Init widget pointers to 0 for null cheching on startup
            avatar_info_widget_ = 0;
            world_info_widget_ = 0;
            control_widget_ = 0;
            action_proxy_widget_ = 0;
            status_widget_ = 0;
        }

        EtherSceneController::EtherSceneController(QObject *parent, Data::DataManager *data_manager, View::EtherScene *scene)
            : QObject(parent),
              data_manager_(data_manager),
              scene_(scene),
              layout_manager_(new CoreUi::AnchorLayoutManager(this, scene)),
              info_hide_timer_(new QTimer(this)),
              login_in_progress_(false),
              last_active_top_card_(0),
              last_active_bottom_card_(0),
              connected_world_(0),
              status_widget_(0),
              top_menu_(0),
              bottom_menu_(0),
              login_animations_(0)
        {
            // Hide timer for status widget
            info_hide_timer_->setSingleShot(true);
            connect(info_hide_timer_, SIGNAL(timeout()), SLOT(HideStatusWidget()));
        }

        EtherSceneController::~EtherSceneController()
        {
            SAFE_DELETE(top_menu_);
            SAFE_DELETE(bottom_menu_);
        }

        void EtherSceneController::LoadStaticEther(EtherLoginNotifier *login_notifier, QMap<QString,QString> stored_login_data)
        {
            // Static login space widgets
            classical_login_widget_ = new CoreUi::Classical::ClassicalLoginWidget(login_notifier, stored_login_data);
            classical_login_widget_->RemoveEtherButton();

            classic_login_proxy_ = new QGraphicsProxyWidget(0, Qt::Widget);
            classic_login_proxy_->setZValue(1);
            classic_login_proxy_->setWidget(classical_login_widget_);

            layout_manager_->AddFullscreenWidget(classic_login_proxy_);

            // Connect static login signals/slots
            connect(classical_login_widget_, SIGNAL( AppExitRequested() ), SLOT( TryExitApplication() ));
        }

        void EtherSceneController::LoadStartUpCardsToScene(QVector<View::InfoCard*> avatar_ordered_vector, int visible_top_items, QVector<View::InfoCard*> world_ordered_vector, int visible_bottom_items)
        {
            top_menu_visible_items_ = visible_top_items;
            bottom_menu_visible_items_ = visible_bottom_items;

            // Add to scene
            foreach (View::InfoCard *avatar_card, avatar_ordered_vector)
                scene_->addItem(avatar_card);
            foreach (View::InfoCard *world_card, world_ordered_vector)
                scene_->addItem(world_card);

            // Calculate rects
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            // Start menus
            top_menu_->Initialize(top_rect, avatar_ordered_vector, card_size_, 0.95, top_menu_visible_items_, 0.05, menu_cap_size_);
            bottom_menu_->Initialize(bottom_rect, world_ordered_vector, card_size_, 0.95, bottom_menu_visible_items_, 0.05, menu_cap_size_);
        }

        void EtherSceneController::LoadAvatarCardsToScene(QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items, bool add_to_scene)
        {
            top_menu_visible_items_ = visible_top_items;

            // Add cards to scene if requested
            if (add_to_scene)
            {
                foreach (View::InfoCard *card, avatar_map.values())
                    scene_->addItem(card);
            }

            // Adjust menu rect to proper size
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            // Start menu with rect and cards, set top as active
            top_menu_->Initialize(top_rect, avatar_map.values().toVector(), card_size_, 0.95, top_menu_visible_items_, 0.05, menu_cap_size_);
            
            // Recalculate the positions if this was a update, 
            // not adding widgets to scene
            if (!add_to_scene)
                RecalculateMenus();

            // For the first run we set top as the active menu
            if (!active_menu_)
                active_menu_ = top_menu_;
        }

        void EtherSceneController::NewAvatarToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items)
        {
            top_menu_visible_items_ = visible_top_items;
            scene_->addItem(new_card);

            // Adjust menu rect to proper size
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            avatar_map.remove(new_card->id());
            QVector<View::InfoCard*> avatar_vector = avatar_map.values().toVector();
            avatar_vector.push_front(new_card);
            
            top_menu_->Initialize(top_rect, avatar_vector, card_size_, 0.95, top_menu_visible_items_, 0.05, menu_cap_size_);
            active_menu_ = top_menu_;

            RecalculateMenus();
        }

        void EtherSceneController::LoadWorldCardsToScene(QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items, bool add_to_scene)
        {
            bottom_menu_visible_items_ = visible_bottom_items;

            // Add cards to scene if requested
            if (add_to_scene)
            {
                foreach (View::InfoCard *card, world_map.values())
                    scene_->addItem(card);
            }

            // Adjust menu rect to proper size
            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            // Start menu with rect and cards
            bottom_menu_->Initialize(bottom_rect, world_map.values().toVector(), card_size_, 0.95, bottom_menu_visible_items_, 0.05, menu_cap_size_);
            
            // Recalculate the positions if this was a update, 
            // not adding widgets to scene
            if (!add_to_scene)
                RecalculateMenus();
        }

        void EtherSceneController::NewWorldToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items)
        {
            bottom_menu_visible_items_ = visible_bottom_items;
            scene_->addItem(new_card);

            // Adjust menu rect to proper size
            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            world_map.remove(new_card->id());
            QVector<View::InfoCard*> world_vector = world_map.values().toVector();
            world_vector.push_front(new_card);

            // Start menu with rect and cards
            bottom_menu_->Initialize(bottom_rect, world_vector, card_size_, 0.95, bottom_menu_visible_items_, 0.05, menu_cap_size_);
            active_menu_ = bottom_menu_;

            RecalculateMenus();
        }

        void EtherSceneController::LoadActionWidgets()
        {
            // Action widget to show info and edit cards info
            action_proxy_widget_ = new View::ActionProxyWidget(data_manager_);
            connect(action_proxy_widget_, SIGNAL( ActionInProgress(bool) ), SLOT( ActionWidgetInProgress(bool) ));
            connect(action_proxy_widget_, SIGNAL( RemoveAvatar(Data::AvatarInfo *) ), SLOT ( RemoveAvatar(Data::AvatarInfo *) ));
            connect(action_proxy_widget_, SIGNAL( RemoveWorld(Data::WorldInfo *) ), SLOT ( RemoveWorld(Data::WorldInfo *) ));
            scene_->addItem(action_proxy_widget_);

            // Avatar info frame
            avatar_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::BottomToTop);
            avatar_info_widget_->SetActionWidget(action_proxy_widget_);
            scene_->addItem(avatar_info_widget_);

            // Add/remove for avatar
            avatar_addremove_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::AddRemoveControl, View::ControlProxyWidget::BottomToTop);
            avatar_addremove_widget_->SetActionWidget(action_proxy_widget_);
            avatar_addremove_widget_->SetOverlayWidget(avatar_info_widget_);
            scene_->addItem(avatar_addremove_widget_);

            // World info frame
            world_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::TopToBottom);
            world_info_widget_->SetActionWidget(action_proxy_widget_);
            scene_->addItem(world_info_widget_);

            // Add/remove for world
            world_addremove_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::AddRemoveControl, View::ControlProxyWidget::TopToBottom);
            world_addremove_widget_->SetActionWidget(action_proxy_widget_);
            world_addremove_widget_->SetOverlayWidget(world_info_widget_);
            scene_->addItem(world_addremove_widget_);

            // Bottom controls
            control_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::ActionControl, View::ControlProxyWidget::TopToBottom);
            connect(control_widget_, SIGNAL( ActionRequest(QString) ), SLOT( ControlsWidgetHandler(QString) ));
            scene_->addItem(control_widget_);

            // Notify widget
            status_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::StatusWidget, View::ControlProxyWidget::NoneDirection);
            connect(control_widget_, SIGNAL( ActionRequest(QString) ), SLOT( ControlsWidgetHandler(QString) ));
            layout_manager_->AddCornerAnchor(status_widget_, Qt::TopLeftCorner, Qt::TopLeftCorner);
            layout_manager_->AddCornerAnchor(status_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
            status_widget_->hide();
        }

        void EtherSceneController::LoadClassicLoginWidget(EtherLoginNotifier *login_notifier, bool default_view, QMap<QString,QString> stored_login_data)
        {
            // Classical login widgets
            classical_login_widget_ = new CoreUi::Classical::ClassicalLoginWidget(login_notifier, stored_login_data);
            classic_login_proxy_ = new QGraphicsProxyWidget(0, Qt::Widget);
            classic_login_proxy_->setZValue(100);
            classic_login_proxy_->setWidget(classical_login_widget_);

            // Default to correct view
            if (default_view)
            {
                classic_login_proxy_->show();
                scene_->SupressKeyEvents(true);
            }
            else
                classic_login_proxy_->hide();
            scene_->addItem(classic_login_proxy_);

            // Classical login widget control button
            QPushButton *classical_login_button = new QPushButton("Classic Mode");
            classical_login_button->setStyleSheet("border: 1px solid rgba(255,255,255,100); color: white; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(18, 18, 18, 255), stop:1 rgba(54, 54, 54, 255)); padding: 10px;");
            classical_login_button->setFont(QFont("Narkisim", 12));

            QGraphicsProxyWidget *classic_login_button_proxy_ = new QGraphicsProxyWidget(0, Qt::Widget);
            classic_login_button_proxy_->setWidget(classical_login_button);
            
            layout_manager_->AddCornerAnchor(classic_login_button_proxy_, Qt::TopRightCorner, Qt::TopRightCorner);

            // Connect classic login signals/slots
            connect(classical_login_button, SIGNAL( clicked() ), classical_login_widget_, SLOT( show() ));
            connect(classical_login_button, SIGNAL( clicked() ), SLOT( StopActiveItemAnimations() ));
            connect(classical_login_widget_, SIGNAL(ClassicLoginHidden()), scene_, SLOT(SupressKeyEvents()));
            connect(classical_login_widget_, SIGNAL( AppExitRequested() ), SLOT( TryExitApplication() ));
        }

        void EtherSceneController::RecalculateMenus()
        {
            SceneRectChanged(scene_->sceneRect());
        }

        void EtherSceneController::SceneRectChanged(const QRectF &new_rect)
        {
            if (!top_menu_ || !bottom_menu_)
                return;

            QRectF controls_rect, left_over_rect, top_rect, bottom_rect;
            View::InfoCard *hightlight_top = top_menu_->GetHighlighted();
            View::InfoCard *hightlight_bottom = bottom_menu_->GetHighlighted();
            qreal present_scale = hightlight_top->scale();
            qreal scaled_margin = 58.00 * present_scale;
            bool do_fade = true;
            
            // Controls rect
            controls_rect.setTopLeft(QPointF(0, new_rect.height() - control_widget_->rect().height()));
            controls_rect.setHeight(control_widget_->rect().height());
            controls_rect.setWidth(new_rect.width());

            left_over_rect = new_rect;
            left_over_rect.setY(left_over_rect.y() + scaled_margin + 25);
            left_over_rect.setHeight(left_over_rect.height() - scaled_margin - 20 - control_widget_->rect().height());

            // New rects to item menus
            top_rect = left_over_rect;
            top_rect.setHeight(left_over_rect.height()/2 - scaled_margin);
            bottom_rect = left_over_rect;
            bottom_rect.setY(top_rect.bottom() + scaled_margin * 2);
            bottom_rect.setHeight(left_over_rect.height()/2 - scaled_margin);

            top_menu_->RectChanged(top_rect);
            bottom_menu_->RectChanged(bottom_rect);
            
            QRectF top_scene_rect = hightlight_top->mapRectToScene(hightlight_top->boundingRect());
            QRectF bottom_scene_rect = hightlight_bottom->mapRectToScene(hightlight_bottom->boundingRect());
            // Update avatar info geometry (highlighted card size and pos)
            avatar_info_widget_->UpdateGeometry(top_scene_rect, present_scale, do_fade);
            // Update world info geometry (highlighted card size and pos)
            world_info_widget_->UpdateGeometry(bottom_scene_rect, present_scale, do_fade);
            // Update avatar add/remove widget
            avatar_addremove_widget_->UpdateGeometry(top_scene_rect, present_scale, do_fade);
            // Update world add/remove widget
            world_addremove_widget_->UpdateGeometry(bottom_scene_rect, present_scale, do_fade);
            // Update control widget position (bottom of screen, pos calculated inside)
            control_widget_->UpdateGeometry(controls_rect, present_scale, !do_fade);
            // Update action widgets geometry (full screen)
            action_proxy_widget_->UpdateGeometry(new_rect);

            // Start crazy, but neccessary hack
            connect(hightlight_top->GetMoveAnimationPointer(), SIGNAL( finished()), SLOT( TheResizer() ));
            last_scale_ = present_scale * 10;

            // Classical login widget (full screen)
            classic_login_proxy_->setGeometry(new_rect);
        }

        void EtherSceneController::TheResizer()
        {
            // End crazy, but neccessary hack
            if (last_scale_ != (int)(top_menu_->GetHighlighted()->scale()*10) && top_menu_->GetHighlighted()->scale() < 0.95)
                SceneRectChanged(scene_->sceneRect());
            disconnect(this, SLOT( TheResizer() ));
        }

        void EtherSceneController::UpPressed()
        {
            active_menu_ = top_menu_;
            if (last_active_top_card_)
                ActiveItemChanged(last_active_top_card_);
        }

        void EtherSceneController::DownPressed()
        {
            active_menu_ = bottom_menu_;
            if (last_active_bottom_card_)
                ActiveItemChanged(last_active_bottom_card_);
        }

        void EtherSceneController::LeftPressed() { active_menu_->moveLeft(); }
        void EtherSceneController::RightPressed() { active_menu_->moveRight(); }

        void EtherSceneController::ItemActivatedWithMouse(View::InfoCard *clicked_item)
        {
            if (clicked_item->arragementType() == View::InfoCard::TopToBottom)
                top_menu_->SetFocusToCard(clicked_item);
            else
                bottom_menu_->SetFocusToCard(clicked_item);
        }

        void EtherSceneController::ActiveItemChanged(View::InfoCard *card)
        {
            // Clear all active item animations
            if (last_active_top_card_)
                last_active_top_card_->IsActiveItem(false);
            if (last_active_bottom_card_)
                last_active_bottom_card_->IsActiveItem(false);

            if (!card)
                return;

            card->IsActiveItem(true);

            if (card->arragementType() == View::InfoCard::BottomToTop)
            {
                last_active_bottom_card_ = card;
                world_info_widget_->UpdateContollerCard(last_active_bottom_card_);
                world_addremove_widget_->UpdateContollerCard(last_active_bottom_card_);
            }
            else if (card->arragementType() == View::InfoCard::TopToBottom)
            {
                last_active_top_card_ = card;
                avatar_info_widget_->UpdateContollerCard(last_active_top_card_);
                avatar_addremove_widget_->UpdateContollerCard(last_active_top_card_);
            }
        }

        void EtherSceneController::ActionWidgetInProgress(bool action_ongoing)
        {
            scene_->SupressKeyEvents(action_ongoing);
        }

        void EtherSceneController::UpdateAvatarInfoWidget()
        {
            if (last_active_top_card_)
                avatar_info_widget_->UpdateContollerCard(last_active_top_card_);
        }

        void EtherSceneController::RemoveAvatar(Data::AvatarInfo *avatar_info)
        {
            bool removed = data_manager_->RemoveAvatar(avatar_info);
            if (removed && last_active_top_card_)
            {
                last_active_top_card_->close();
                scene_->removeItem(last_active_top_card_);
                emit ObjectRemoved(avatar_info->id());
            }
        }

        void EtherSceneController::UpdateWorldInfoWidget()
        {
            if (last_active_bottom_card_)
                world_info_widget_->UpdateContollerCard(last_active_bottom_card_);
        }

        void EtherSceneController::RemoveWorld(Data::WorldInfo *world_info)
        {
            bool removed = data_manager_->RemoveWorld(world_info);
            if (removed && last_active_bottom_card_)
            {
                last_active_bottom_card_->close();
                scene_->removeItem(last_active_bottom_card_);
                emit ObjectRemoved(world_info->id());
            }
        }

        void EtherSceneController::ControlsWidgetHandler(QString request_type)
        {
            if (request_type == "connect")
                TryStartLogin();
            else if (request_type == "hide")
                status_widget_->hide();
            else if (request_type == "help")
            {
                ShowStatusInformation("Naalis help system is still under development, sorry.");
                return;
            }
            else if (request_type == "exit")
                TryExitApplication();
            else if (request_type == "cancel" || request_type == "disconnect")
            {
                login_in_progress_ = false;

                if (request_type == "cancel")
                {
                    if (!bottom_menu_)
                        return;
                    View::InfoCard* world_card = bottom_menu_->GetHighlighted();
                    if (!world_card)
                        return;
                    ShowStatusInformation(QString("Login to %1 canceled").arg(world_card->title()));
                }
                else if (request_type == "disconnect" && connected_world_)
                    ShowStatusInformation(QString("Disconnected from %1").arg(connected_world_->title()));

                emit DisconnectCurrentConnection();
                SuppressControlWidgets(false);
                scene_->SupressKeyEvents(false);
                SceneRectChanged(scene_->sceneRect());
            }
        }

        void EtherSceneController::TryStartLogin()
        {
            QPair<View::InfoCard*, View::InfoCard*> selected_cards;
            selected_cards.first = top_menu_->GetHighlighted();
            selected_cards.second = bottom_menu_->GetHighlighted();
            
            scene_->SupressKeyEvents(true);
            SuppressControlWidgets(true);

            StartLoginAnimation();
            if (!login_in_progress_)
            {
                emit LoginRequest(selected_cards);
                login_in_progress_ = !login_in_progress_;
            }

            ShowStatusInformation(QString("Connecting to %1 with %2").arg(selected_cards.second->title(), selected_cards.first->title()));
        }

        void EtherSceneController::StartLoginAnimation()
        {
            if (!login_animations_)
                return;

            login_animations_->clear();
            login_animations_->setDirection(QAbstractAnimation::Forward);

            // Get all Infocards
            QVector<View::InfoCard*> t_obj = top_menu_->GetObjects();
            t_obj += bottom_menu_->GetObjects();

            QPropertyAnimation *anim;
            foreach (View::InfoCard* card, t_obj)
            {
                // Animate opacity from current to 0 for every card except the selected cards
                if (card != last_active_bottom_card_ && card != last_active_top_card_)
                {
                    anim = new QPropertyAnimation(card, "opacity", login_animations_);
                    anim->setStartValue(card->opacity());
                    anim->setEndValue(0);
                    anim->setDuration(2000);
                    anim->setEasingCurve(QEasingCurve::InOutSine);
                    login_animations_->addAnimation(anim);
                }
            }

            // Fade animation for add remove widgets
            anim = new QPropertyAnimation(avatar_addremove_widget_, "opacity", login_animations_);
            anim->setStartValue(avatar_addremove_widget_->opacity());
            anim->setEndValue(0);
            anim->setDuration(2000);
            anim->setEasingCurve(QEasingCurve::InOutSine);
            login_animations_->addAnimation(anim);

            anim = new QPropertyAnimation(world_addremove_widget_, "opacity", login_animations_);
            anim->setStartValue(world_addremove_widget_->opacity());
            anim->setEndValue(0);
            anim->setDuration(2000);
            anim->setEasingCurve(QEasingCurve::InOutSine);
            login_animations_->addAnimation(anim);

            // Animate selected cards position
            QPropertyAnimation* anim1 = new QPropertyAnimation(last_active_top_card_, "pos", login_animations_);
            QPropertyAnimation* anim2 = new QPropertyAnimation(avatar_info_widget_, "pos", login_animations_);

            QPropertyAnimation* anim3 = new QPropertyAnimation(last_active_bottom_card_, "pos", login_animations_);
            QPropertyAnimation* anim4 = new QPropertyAnimation(world_info_widget_, "pos", login_animations_);

            qreal diff = avatar_info_widget_->mapRectToScene(avatar_info_widget_->rect()).bottom() - world_info_widget_->mapRectToScene(world_info_widget_->rect()).top();
            diff /=2;

            if (diff < 0)
                diff *= -1;

            // Calculate positions
            QPointF pos1 = last_active_top_card_->pos();
            pos1.setY(pos1.y() + diff + 3);

            QPointF pos2 = avatar_info_widget_->pos();
            pos2.setY(pos2.y() + diff + 3);

            QPointF pos3 = last_active_bottom_card_->pos();
            pos3.setY(pos3.y() - diff - 3);

            QPointF pos4 = world_info_widget_->pos();
            pos4.setY(pos4.y() - diff - 3);

            anim1->setStartValue(last_active_top_card_->pos());
            anim1->setEndValue(pos1);
            anim1->setDuration(2000);
            anim1->setEasingCurve(QEasingCurve::InOutSine);

            anim2->setStartValue(avatar_info_widget_->pos());
            anim2->setEndValue(pos2);
            anim2->setDuration(2000);
            anim2->setEasingCurve(QEasingCurve::InOutSine);

            anim3->setStartValue(last_active_bottom_card_->pos());
            anim3->setEndValue(pos3);
            anim3->setDuration(2000);
            anim3->setEasingCurve(QEasingCurve::InOutSine);

            anim4->setStartValue(world_info_widget_->pos());
            anim4->setEndValue(pos4);
            anim4->setDuration(2000);
            anim4->setEasingCurve(QEasingCurve::InOutSine);

            // Add animations to the group
            login_animations_->addAnimation(anim1);
            login_animations_->addAnimation(anim2);
            login_animations_->addAnimation(anim3);
            login_animations_->addAnimation(anim4);

            // Start animations
            login_animations_->start();
        }

        void EtherSceneController::RevertLoginAnimation(bool change_scene_after_anims_finish)
        {
            if (!login_animations_)
                return;

            change_scene_after_anims_finish_ = change_scene_after_anims_finish; 
            if (login_animations_->state() == QAbstractAnimation::Running)
                login_animations_->pause();
            login_animations_->setDirection(QAbstractAnimation::Backward);
            login_animations_->start();
        }

        void EtherSceneController::LoginAnimationFinished()
        {
            if (!login_animations_)
                return;

            if (login_animations_->direction() == QAbstractAnimation::Backward && change_scene_after_anims_finish_)
               scene_->EmitSwitchSignal();
            if (login_animations_->direction() == QAbstractAnimation::Backward && !change_scene_after_anims_finish_)
                HideStatusWidget();
            if (login_animations_->direction() == QAbstractAnimation::Backward)
                SuppressControlWidgets(false);
        }

        void EtherSceneController::SetConnected(bool connected)
        {
            control_widget_->SetConnected(connected);
            if (connected)
                connected_world_ = bottom_menu_->GetHighlighted();
            else
                connected_world_ = 0;
        }

        void EtherSceneController::SuppressControlWidgets(bool suppress)
        {
            control_widget_->SuppressButtons(suppress);
        }

        void EtherSceneController::ShowStatusInformation(QString text)
        {
            if (status_widget_)
            {
                status_widget_->UpdateStatusText(text);
                status_widget_->show();
                info_hide_timer_->start(7500);
            }

            classical_login_widget_->StatusUpdate(login_in_progress_, text);
        }

        void EtherSceneController::HideStatusWidget()
        {
            status_widget_->hide();
        }

        void EtherSceneController::TryExitApplication()
        {
            ShowStatusInformation("Exiting");
            emit ApplicationExitRequested();
        }

        void EtherSceneController::StoreConfigs()
        {
            // Store ether data
            if (last_active_top_card_ && last_active_bottom_card_)
                data_manager_->StoreSelectedCards(last_active_top_card_->id(), last_active_bottom_card_->id());
           
            // Store classic login data
            if (classical_login_widget_)
            {
                QMap<QString, QString> classic_login_info = classical_login_widget_->GetLoginInfo();
                data_manager_->StoreClassicLoginInfo(classic_login_info);
            }

            // Store default ether view
            if (classical_login_widget_->isVisible())
                data_manager_->StoreDefaultView("classic");
            else
                data_manager_->StoreDefaultView("ether");
        }

        void EtherSceneController::StopActiveItemAnimations()
        {
            ActiveItemChanged(0);
            scene_->SupressKeyEvents(true);
        }
    }
}
