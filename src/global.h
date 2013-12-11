#ifndef SRC_GLOBAL_H
#define SRC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(REMOTEDISPLAY_LIBRARY)
#  define REMOTEDISPLAYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define REMOTEDISPLAYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SRC_GLOBAL_H
