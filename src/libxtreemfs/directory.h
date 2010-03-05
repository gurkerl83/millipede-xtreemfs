// Copyright (c) 2010 Minor Gordon
// All rights reserved
// 
// This source file is part of the XtreemFS project.
// It is licensed under the New BSD license:
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the XtreemFS project nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL Minor Gordon BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef _LIBXTREEMFS_DIRECTORY_H_
#define _LIBXTREEMFS_DIRECTORY_H_

#include "xtreemfs/mrc_proxy.h"
#include "xtreemfs/path.h"
#include "xtreemfs/user_credentials_cache.h"


namespace xtreemfs
{
  class Directory : public YIELD::platform::Directory
  {
  public:
    const static uint64_t LIMIT_DIRECTORY_ENTRIES_COUNT_DEFAULT = 100;

    // YIELD::platform::Directory
    YIELD_PLATFORM_DIRECTORY_PROTOTYPES;

  private:
    friend class Volume;

    Directory
    ( 
      const org::xtreemfs::interfaces::DirectoryEntrySet& first_directory_entries,
      YIELD::platform::auto_Log log,
      auto_MRCProxy mrc_proxy,
      bool names_only,
      const Path& path,
      auto_UserCredentialsCache user_credentials_cache
    );

    ~Directory()
    { }

    org::xtreemfs::interfaces::DirectoryEntrySet directory_entries;
    YIELD::platform::auto_Log log;
    auto_MRCProxy mrc_proxy;
    bool names_only;
    Path path;
    uint16_t read_directory_entry_i;
    uint64_t seen_directory_entries_count;
    auto_UserCredentialsCache user_credentials_cache;
  };

  typedef yidl::runtime::auto_Object<Directory> auto_Directory;
};

#endif
