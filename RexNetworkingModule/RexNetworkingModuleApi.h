// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworkingModuleApi_h
#define incl_RexNetworkingModuleApi_h

#if defined (_WINDOWS)
#if defined(REXNETWORKING_MODULE_EXPORTS)
#define REXNETWORKING_MODULE_API __declspec(dllexport)
#else
#define REXNETWORKING_MODULE_API __declspec(dllimport)
#endif
#else
#define REXNETWORKING_MODULE_API
#endif

#endif
