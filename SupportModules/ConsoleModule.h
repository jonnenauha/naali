// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleModule_h
#define incl_ConsoleModule_h

#include "ModuleInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Console
{
    //! interface for modules
    class VIEWER_API ConsoleModule : public Foundation::ModuleInterface_Impl
    {
    public:
        ConsoleModule();
        virtual ~ConsoleModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

        virtual void Update();

        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }

        //! Returns default console
        ConsolePtr GetConsole() const { return manager_; };


        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Console;

    private:
        //! debug console manager
        ConsolePtr manager_;

        Foundation::Framework *framework_;
    };
}

#endif
