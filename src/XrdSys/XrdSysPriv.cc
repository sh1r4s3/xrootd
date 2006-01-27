/******************************************************************************/
/*                                                                            */
/*                     X r d S y s P r i v . c c                              */
/*                                                                            */
/* (c) 2006 G. Ganis (CERN)                                                   */
/*     All Rights Reserved. See XrdInfo.cc for complete License Terms         */
/******************************************************************************/
// $Id$

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdSysPriv                                                           //
//                                                                      //
// Author: G. Ganis, CERN, 2006                                         //
//                                                                      //
// Implementation of a privileges handling API following the paper      //
//   "Setuid Demystified" by H.Chen, D.Wagner, D.Dean                   //
// also quoted in "Secure programming Cookbook" by J.Viega & M.Messier. //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "XrdSys/XrdSysPriv.hh"

#if !defined(WINDOWS)
#include <stdio.h>
#include <iostream.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#define NOUC ((uid_t)(-1))
#define NOGC ((gid_t)(-1))

// Some machine specific stuff
#if defined(__sgi) && !defined(__GNUG__) && (SGI_REL<62)
extern "C" {
   int seteuid(int euid);
   int setegid(int egid);
   int geteuid();
   int getegid();
}
#endif

#if defined(_AIX)
extern "C" {
   int seteuid(uid_t euid);
   int setegid(gid_t egid);
   uid_t geteuid();
   gid_t getegid();
}
#endif

#if !defined(__hpux) && !defined(linux) && !defined(__FreeBSD__) && \
    !defined(__OpenBSD__) || defined(cygwingcc)
static int setresgid(gid_t r, gid_t e, gid_t)
{
   if (setgid(r) == -1)
      return -1;
   return setegid(e);
}

static int setresuid(uid_t r, uid_t e, uid_t)
{
   if (setuid(r) == -1)
      return -1;
   return seteuid(e);
}

static int getresgid(gid_t *r, gid_t *e, gid_t *)
{
  *r = getgid();
  *e = getegid();
  return 0;
}

static int getresuid(uid_t *r, uid_t *e, uid_t *)
{
  *r = getuid();
  *e = geteuid();
  return 0;
}

#else
#if (defined(__linux__) || \
    (defined(__CYGWIN__) && defined(__GNUC__))) && !defined(linux)
#   define linux
#endif
#if defined(linux) && !defined(HAS_SETRESUID)
extern "C" {
   int setresgid(gid_t r, gid_t e, gid_t s);
   int setresuid(uid_t r, uid_t e, uid_t s);
   int getresgid(gid_t *r, gid_t *e, gid_t *s);
   int getresuid(uid_t *r, uid_t *e, uid_t *s);
}
#endif
#endif
#endif // not WINDOWS

//______________________________________________________________________________
int XrdSysPriv::Restore(bool saved)
{
   // Restore the 'saved' (saved = TRUE) or 'real' entity as effective.
   // Return 0 on success, !=0 if any error occurs.

#if !defined(WINDOWS)
   // Get the UIDs
   uid_t ruid = 0, euid = 0, suid = 0;
   if (getresuid(&ruid, &euid, &suid) != 0)
      return -1;

   // Set the wanted value
   uid_t uid = saved ? suid : ruid;

   // Set uid as effective
   if (setresuid(NOUC, uid, NOUC) != 0)
      return -1;

   // Make sure the new effective UID is the one wanted
   if (geteuid() != uid)
      return -1;

   // Get the GIDs
   uid_t rgid = 0, egid = 0, sgid = 0;
   if (getresgid(&rgid, &egid, &sgid) != 0)
      return -1;

   // Set the wanted value
   gid_t gid = saved ? sgid : rgid;

   // Set newuid as effective, saving the current effective GID
   if (setresgid(NOGC, gid, NOGC) != 0)
      return -1;

   // Make sure the new effective GID is the one wanted
   if (getegid() != gid)
      return -1;
#endif
   // Done
   return 0;
}

//______________________________________________________________________________
int XrdSysPriv::ChangeTo(uid_t newuid)
{
   // Change effective to entity newuid. Current entity is saved.
   // Real entity is not touched. Use RestoreSaved to go back to
   // previous settings.
   // Return 0 on success, !=0 if any error occurs.

#if !defined(WINDOWS)
   // Make sure the entity is consistent
   struct passwd *pw = getpwuid(newuid);
   if (!pw)
      return -1;

   // Current UGID 
   uid_t oeuid = geteuid();
   gid_t oegid = getegid();

   // Restore privileges, if needed
   if (oeuid && XrdSysPriv::Restore(0) != 0)
      return -1;

   // New GID 
   gid_t newgid = pw->pw_gid;

   // Set newgid as effective, saving the current effective GID
   if (setresgid(NOGC, newgid, oegid) != 0)
      return -1;

   // Get the GIDs
   uid_t rgid = 0, egid = 0, sgid = 0;
   if (getresgid(&rgid, &egid, &sgid) != 0)
      return -1;

   // Make sure the new effective GID is the one wanted
   if (egid != newgid || sgid != oegid)
      return -1;

   // Set newuid as effective, saving the current effective UID
   if (setresuid(NOUC, newuid, oeuid) != 0)
      return -1;

   // Get the UIDs
   uid_t ruid = 0, euid = 0, suid = 0;
   if (getresuid(&ruid, &euid, &suid) != 0)
      return -1;

   // Make sure the new effective UID is the one wanted
   if (euid != newuid || suid != oeuid)
      return -1;
#endif
   // Done
   return 0;
}

//______________________________________________________________________________
int XrdSysPriv::ChangePerm(uid_t newuid)
{
   // Change permanently to entity newuid. Requires super-userprivileges.
   // Provides a way to drop permanently su privileges.
   // Return 0 on success, !=0 if any error occurs.

#if !defined(WINDOWS)
   // Make sure the entity is consistent
   struct passwd *pw = getpwuid(newuid);
   if (!pw)
      return -1;

   // Restore privileges, if needed
   if (geteuid() && XrdSysPriv::Restore(0) != 0)
      return -1;

   // New GID 
   gid_t newgid = pw->pw_gid;

   // Set newgid as GID, alle levels
   if (setresgid(newgid, newgid, newgid) != 0)
      return -1;

   // Get GID
   uid_t rgid = 0, egid = 0, sgid = 0;
   if (getresgid(&rgid, &egid, &sgid) != 0)
      return -1;

   // Make sure the new GIDs are all equal to the one asked
   if (rgid != newgid || egid != newgid || sgid != newgid)
      return -1;

   // Set newuid as UID, all levels
   if (setresuid(newuid, newuid, newuid) != 0)
      return -1;

   // Get UID
   uid_t ruid = 0, euid = 0, suid = 0;
   if (getresuid(&ruid, &euid, &suid) != 0)
      return -1;

   // Make sure the new UIDs are all equal to the one asked 
   if (ruid != newuid || euid != newuid || suid != newuid)
      return -1;
#endif
   // Done
   return 0;
}

//______________________________________________________________________________
void XrdSysPriv::DumpUGID(const char *msg)
{
   // Dump current entity

#if !defined(WINDOWS)
   // Get the UIDs
   uid_t ruid = 0, euid = 0, suid = 0;
   if (getresuid(&ruid, &euid, &suid) != 0)
      return;

   // Get the GIDs
   uid_t rgid = 0, egid = 0, sgid = 0;
   if (getresgid(&rgid, &egid, &sgid) != 0)
      return;

   cout << "XrdSysPriv: "  << endl; 
   cout << "XrdSysPriv: dump values: " << (msg ? msg : "") << endl; 
   cout << "XrdSysPriv: "  << endl; 
   cout << "XrdSysPriv: real       = (" << ruid <<","<< rgid <<")" << endl; 
   cout << "XrdSysPriv: effective  = (" << euid <<","<< egid <<")" << endl; 
   cout << "XrdSysPriv: saved      = (" << suid <<","<< sgid <<")" << endl; 
   cout << "XrdSysPriv: "  << endl; 
#endif
}

//
// Guard class
//______________________________________________________________________________
XrdSysPrivGuard::XrdSysPrivGuard(uid_t uid)
{
   // Constructor. We act only if 'superuser'.

   dum = 1;
   valid = 1;
#if !defined(WINDOWS)
   uid_t ruid = 0, euid = 0, suid = 0;
   if (getresuid(&ruid, &euid, &suid) == 0 && (euid != uid) && !ruid) {
      // Change temporarly identity
      if (XrdSysPriv::ChangeTo(uid) != 0)
         valid = 0;
      dum = 0;
   }
#endif
}
