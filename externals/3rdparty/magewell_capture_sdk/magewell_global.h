#ifndef MAGEWELL_GLOBAL_H
#define MAGEWELL_GLOBAL_H

#include <QMetaType>


#ifdef __linux__
;
#else

#define MWWaitEvent WaitForSingleObject
#define MWCreateEvent() CreateEvent(NULL, FALSE, FALSE, NULL)
#define MWCloseEvent CloseHandle

#endif


typedef void* MGHCHANNEL;

Q_DECLARE_METATYPE(MGHCHANNEL)


#endif // MAGEWELL_GLOBAL_H
