//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include "utils/log.h"
#include "audio/audioplaybackinterface.h"

namespace doozy
{

#ifdef __APPLE__

static void sigsegv(int signo, siginfo_t* pInfo, void* pContext)
{
    void* array[50];
    size_t size     = backtrace(array, 50);
    char** messages = backtrace_symbols(array, size);
    
    // skip first stack frame (points here)
    for (auto i = 1; i < size && messages != nullptr; ++i)
    {
        utils::log::critical("[bt]: (%d) %s", i, messages[i]);
    }
    
    free(messages);
    
    exit(1);
}

#else

struct sig_ucontext_t
{
   unsigned long     uc_flags;
   struct ucontext   *uc_link;
   stack_t           uc_stack;
   struct sigcontext uc_mcontext;
   sigset_t          uc_sigmask;
};

static void sigsegv(int signo, siginfo_t* pInfo, void* pContext)
{
    void* array[50];
    auto pSigContext = reinterpret_cast<sig_ucontext_t*>(pContext);
    
    /* Get the address at the time the signal was raised from the EIP (x86) */
#ifdef __arm__
    void* pCallerAddress = (void*) uc->uc_mcontext.arm_pc;
#else
    void* pCallerAddress = (void*) uc->uc_mcontext.eip;
#endif
    
    size_t size     = backtrace(array, 50);
    array[1]        = pCallerAddress;
    char** messages = backtrace_symbols(array, size);
    
    // skip first stack frame (points here)
    for (auto i = 1; i < size && messages != nullptr; ++i)
    {
        log::critical("[bt]: (%d) %s", i, messages[i]);
    }
    
    free(messages);
    
    exit(1);
}

#endif

}

#endif