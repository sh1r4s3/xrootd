#include <sys/errno.h>
#include <sys/socket.h>  // for sockaddr
// Return Values for Integer Returning XrdSfs Interface
#define SFS_STALL         1 // ErrInfo code -> Seconds to stall client
     XrdSfsFileExistIsDirectory
struct XrdSfsFSctl // SFS_FSCTL_PLUGIN/PLUGIO parameters
 const char            *Arg1;      // PLUGIO & PLUGIN
       int              Arg1Len;
       int              Arg2Len;
 const char            *Arg2;      // PLUGIN opaque string
struct XrdSfsPrep  // Prepare parameters
       char            *reqid;     // Request ID
       char            *notify;    // Notification path or 0
       int              opts;      // Prep_xxx
       XrdOucTList     *paths;     // List of paths
       XrdOucTList     *oinfo;     // 1-to-1 correspondence of opaque info
// The following two methods allocate a directory or file object
//
// The following are filesystem related methods
//
                              const char             *Path,
                                    XrdOucErrInfo    &out_error,
                              {out_error.setErrInfo(ENOTSUP, "Not supported.");
virtual int            chmod(const char             *Name,
                                   XrdSfsMode        Mode,
                                   XrdOucErrInfo    &out_error,
                                   XrdOucErrInfo    &out_error,
                                   XrdOucErrInfo    &out_error,
virtual int            exists(const char                *fileName,
                                    XrdSfsFileExistence &exists_flag,
                                    XrdOucErrInfo       &out_error,
virtual int            mkdir(const char             *dirName,
                                   XrdSfsMode         Mode,
                                   XrdOucErrInfo     &out_error,
                                     XrdOucErrInfo   &out_error,
                                 XrdOucErrInfo       &out_error,
virtual int            remdir(const char             *dirName,
                                    XrdOucErrInfo    &out_error,
virtual int            rename(const char             *oldFileName,
                              const char             *newFileName,
                                    XrdOucErrInfo    &out_error,
                                  XrdOucErrInfo      &out_error,
virtual int            stat(const char               *Name,
                                  XrdOucErrInfo      &out_error,
virtual int            truncate(const char             *Name,
                                      XrdSfsFileOffset fileOffset,
                                      XrdOucErrInfo    &out_error,

protected:
/* When building a shared library plugin, the following "C" entry point must
   exist in the library:

//! Specify the compilation version.
//!
//! Additionally, you *should* declare the xrootd version you used to compile
//! your plug-in. While not currently required, it is highly recommended to
//! avoid execution issues should the class definition change. Declare it as:
//------------------------------------------------------------------------------
/*! #include "XrdVersion.hh"
                                  XrdOucErrInfo    &out_error) = 0;
virtual int            read(XrdSfsFileOffset   fileOffset,
                            XrdSfsXferSize     preread_sz) = 0;
virtual XrdSfsXferSize read(XrdSfsFileOffset   fileOffset,
                            XrdSfsXferSize     buffer_size) = 0;
virtual int            read(XrdSfsAio *aioparm) = 0;
virtual XrdSfsXferSize write(XrdSfsFileOffset  fileOffset,
                             XrdSfsXferSize    buffer_size) = 0;
virtual int            truncate(XrdSfsFileOffset fileOffset) = 0;
virtual int         open(const char              *dirName,