/// @file Water.h
/// @brief Manages Water-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Water_h
#define incl_Water_h

#include "EnvironmentModuleApi.h"
#include "EnvironmentModule.h"
#include <QObject>

namespace Environment
{
    class EC_Water;

    class ENVIRONMENT_MODULE_API Water : public QObject
    {
        Q_OBJECT 

        public:
        
            Water(EnvironmentModule *owner_);
            virtual ~Water();

            //! Looks through all the entities in RexLogic's currently active scene to find the Water
            //! entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
            void FindCurrentlyActiveWater();

            //! @return The scene entity that represents the terrain in the currently active world.
            Scene::EntityWeakPtr GetWaterEntity();
            
            //! Creates water geometry, uses a default value to water height
            void CreateWaterGeometry();
       
            //! @return The scene water height.
            //! @note If error occuers returns -1.0
            float GetWaterHeight() const;

        public slots:

            //! Sets a new water height to scene water.
            void SetWaterHeight(float height);   

        signals:
            //! Emited when water height has been changed. 
            void HeightChanged(double height);

        private:

            EnvironmentModule *owner_;
            EC_Water* waterComponent_;
            Scene::EntityWeakPtr cachedWaterEntity_;
  
    };

}

#endif