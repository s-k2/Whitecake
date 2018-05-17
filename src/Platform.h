
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#include "Win32/Platform.h"

#endif /* WIN32 */

#ifdef __linux__

#include "Gtk/Platform.h"

#endif /* __linux__ */

#endif /* PLATFORM_H */