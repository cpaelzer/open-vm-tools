/*********************************************************
 * Copyright (C) 2009 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 and no later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *********************************************************/


#include "vmware.h"
#include "str.h"
#include "util.h"
#include "userlock.h"
#include "ulInt.h"


/*
 * Return an invalid thread ID until lib/thread is initialized.
 *
 * XXX
 *
 * VThread_CurID cannot be called before VThread_Init is called; doing so
 * causes assertion failures in some programs. This will go away when
 * lib/nothread goes away - we'll assign "dense", rationalized VMware
 * thread IDs without the distinction of lib/thread and lib/nothread.
 */

static VThreadID
MXUserDummyCurID(void)
{
   return VTHREAD_INVALID_ID;
}

VThreadID (*MXUserThreadCurID)(void) = MXUserDummyCurID;

void
MXUserIDHack(void)
{
   MXUserThreadCurID = VThread_CurID;
}

/*
 *-----------------------------------------------------------------------------
 *
 * MXUserDumpAndPanic
 *
 *      Dump a lock, print a message and die
 *
 * Results:
 *      A panic.
 *
 * Side effects:
 *      Manifold.
 *
 *-----------------------------------------------------------------------------
 */

void
MXUserDumpAndPanic(MXRecLock *lock,   // IN:
                   const char *fmt,   // IN:
                   ...)               // IN:
{
   char *msg;
   va_list ap;

   Warning("%s: Lock @ %p\n",    __FUNCTION__, lock);
   Warning("%s: signature %X\n", __FUNCTION__, lock->lockHeader.lockSignature);
   Warning("%s: count %u\n",     __FUNCTION__, lock->lockHeader.lockCount);
   Warning("%s: name %s\n",      __FUNCTION__, lock->lockHeader.lockName);
   Warning("%s: caller %p\n",    __FUNCTION__, lock->lockHeader.lockCaller);
   Warning("%s: rank %d\n",      __FUNCTION__, lock->lockHeader.lockRank);

   Warning("%s: VThreadID %d\n", __FUNCTION__,
           (int) lock->lockHeader.lockVThreadID);

   va_start(ap, fmt);
   msg = Str_SafeVasprintf(NULL, fmt, ap);
   va_end(ap);

   Panic(msg);
}


/*
 *-----------------------------------------------------------------------------
 *
 * MXUserIsAllUnlocked --
 *
 *      Is the lock currently completely unlocked?
 *
 * Results:
 *      The lock is acquired.
 *
 * Side effects:
 *      None
 *
 *-----------------------------------------------------------------------------
 */

Bool
MXUserIsAllUnlocked(const MXUserRWLock *lock)  // IN:
{
   uint32 i;

   for (i = 0; i < VTHREAD_MAX_THREADS; i++) {
      if (lock->lockTaken[i] != RW_UNLOCKED) {
         return FALSE;
      }
   }

   return TRUE;
}
