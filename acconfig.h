/* Define this to the desired log facility */
#define LOGFACILITY LOG_LOCAL4

#undef BUILD_TARGET

@TOP@

@BOTTOM@

typedef unsigned char Uchar;
typedef unsigned short Ushort;
typedef unsigned int Uint;

#if defined(sun) || defined(sysv) || defined(__sysv__)
# define PIDFILE "/etc/gjdictd.pid"
#else
# define PIDFILE "/var/run/gjdictd.pid"
#endif

#if defined (SETVBUF_REVERSED)
# define SETVBUF(str,buf,mode,size) setvbuf(str,mode,buf,size) 
#else
# define SETVBUF(str,buf,mode,size) setvbuf(str,buf,mode,size)
#endif
