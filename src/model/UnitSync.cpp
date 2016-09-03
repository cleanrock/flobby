// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "UnitSync.h"
#include "log/Log.h"

#include <stdexcept>
#include <cassert>

#define BIND( NAME ) bind( #NAME , NAME )

std::map<std::string, Lmid_t> UnitSync::dlmNamespaces_;

UnitSync::UnitSync(std::string const & path)
{
    Lmid_t dlmNamespace = LM_ID_NEWLM;

    auto ns = dlmNamespaces_.find(path);
    if (ns != dlmNamespaces_.end())
    {
        dlmNamespace = ns->second;
    }

    lib_ = ::dlmopen(dlmNamespace, path.c_str(), RTLD_LAZY);
    if (lib_ == NULL)
    {
        throw std::invalid_argument("dlopen failed: " + path);
    }

    // store new namespace for reuse
    if (dlmNamespace == LM_ID_NEWLM)
    {
        Lmid_t lmid;
        int const res = ::dlinfo(lib_, RTLD_DI_LMID, &lmid);
        if (res != 0)
        {
            LOG(WARNING)<< "dlinfo(RTLD_DI_LMID) failed: " << path;
        }
        else
        {
            dlmNamespaces_[path] = lmid;
        }
    }

    BIND(GetNextError);
    BIND(GetSpringVersion);
    BIND(GetSpringVersionPatchset);
    BIND(IsSpringReleaseVersion);
    BIND(Init);
    BIND(UnInit);
    BIND(GetWritableDataDirectory);
    BIND(GetDataDirectoryCount);
    BIND(GetDataDirectory);
    BIND(ProcessUnits);
    BIND(GetUnitCount);
    BIND(GetUnitName);
    BIND(GetFullUnitName);
    BIND(AddArchive);
    BIND(AddAllArchives);
    BIND(RemoveAllArchives);
    BIND(GetArchiveChecksum);
    BIND(GetArchivePath);
    BIND(GetMapCount);
    BIND(GetMapName);
    BIND(GetMapInfoCount);
    BIND(GetMapFileName);
    BIND(GetMapMinHeight);
    BIND(GetMapMaxHeight);
    BIND(GetMapArchiveCount);
    BIND(GetMapArchiveName);
    BIND(GetMapChecksum);
    BIND(GetMapChecksumFromName);
    BIND(GetMinimap);
    BIND(GetInfoMapSize);
    BIND(GetInfoMap);
    BIND(GetSkirmishAICount);
    BIND(GetSkirmishAIInfoCount);
    BIND(GetInfoKey);
    BIND(GetInfoType);
    BIND(GetInfoValueString);
    BIND(GetInfoValueInteger);
    BIND(GetInfoValueFloat);
    BIND(GetInfoValueBool);
    BIND(GetInfoDescription);
    BIND(GetSkirmishAIOptionCount);
    BIND(GetPrimaryModCount);
    BIND(GetPrimaryModInfoCount);
    BIND(GetPrimaryModArchive);
    BIND(GetPrimaryModArchiveCount);
    BIND(GetPrimaryModArchiveList);
    BIND(GetPrimaryModIndex);
    BIND(GetPrimaryModChecksum);
    BIND(GetPrimaryModChecksumFromName);
    BIND(GetSideCount);
    BIND(GetSideName);
    BIND(GetSideStartUnit);
    BIND(GetMapOptionCount);
    BIND(GetModOptionCount);
    BIND(GetCustomOptionCount);
    BIND(GetOptionKey);
    BIND(GetOptionScope);
    BIND(GetOptionName);
    BIND(GetOptionSection);
    BIND(GetOptionDesc);
    BIND(GetOptionType);
    BIND(GetOptionBoolDef);
    BIND(GetOptionNumberDef);
    BIND(GetOptionNumberMin);
    BIND(GetOptionNumberMax);
    BIND(GetOptionNumberStep);
    BIND(GetOptionStringDef);
    BIND(GetOptionStringMaxLen);
    BIND(GetOptionListCount);
    BIND(GetOptionListDef);
    BIND(GetOptionListItemKey);
    BIND(GetOptionListItemName);
    BIND(GetOptionListItemDesc);
    BIND(GetModValidMapCount);
    BIND(GetModValidMap);
    BIND(OpenFileVFS);
    BIND(CloseFileVFS);
    BIND(ReadFileVFS);
    BIND(FileSizeVFS);
    BIND(InitFindVFS);
    BIND(InitDirListVFS);
    BIND(InitSubDirsVFS);
    BIND(FindFilesVFS);
    BIND(OpenArchive);
    BIND(CloseArchive);
    BIND(FindFilesArchive);
    BIND(OpenArchiveFile);
    BIND(ReadArchiveFile);
    BIND(CloseArchiveFile);
    BIND(SizeArchiveFile);
    BIND(SetSpringConfigFile);
    BIND(GetSpringConfigFile);
    BIND(GetSpringConfigString);
    BIND(GetSpringConfigInt);
    BIND(GetSpringConfigFloat);
    BIND(SetSpringConfigString);
    BIND(SetSpringConfigInt);
    BIND(SetSpringConfigFloat);
    BIND(lpClose);
    BIND(lpOpenFile);
    BIND(lpOpenSource);
    BIND(lpExecute);
    BIND(lpErrorLog);
    BIND(lpAddTableInt);
    BIND(lpAddTableStr);
    BIND(lpEndTable);
    BIND(lpAddIntKeyIntVal);
    BIND(lpAddStrKeyIntVal);
    BIND(lpAddIntKeyBoolVal);
    BIND(lpAddStrKeyBoolVal);
    BIND(lpAddIntKeyFloatVal);
    BIND(lpAddStrKeyFloatVal);
    BIND(lpAddIntKeyStrVal);
    BIND(lpAddStrKeyStrVal);
    BIND(lpRootTable);
    BIND(lpRootTableExpr);
    BIND(lpSubTableInt);
    BIND(lpSubTableStr);
    BIND(lpSubTableExpr);
    BIND(lpPopTable);
    BIND(lpGetKeyExistsInt);
    BIND(lpGetKeyExistsStr);
    BIND(lpGetIntKeyType);
    BIND(lpGetStrKeyType);
    BIND(lpGetIntKeyListCount);
    BIND(lpGetIntKeyListEntry);
    BIND(lpGetStrKeyListCount);
    BIND(lpGetStrKeyListEntry);
    BIND(lpGetIntKeyIntVal);
    BIND(lpGetStrKeyIntVal);
    BIND(lpGetIntKeyBoolVal);
    BIND(lpGetStrKeyBoolVal);
    BIND(lpGetIntKeyFloatVal);
    BIND(lpGetStrKeyFloatVal);
    BIND(lpGetIntKeyStrVal);
    BIND(lpGetStrKeyStrVal);

}

UnitSync::~UnitSync()
{
    int res = ::dlclose(lib_);
    assert(res == 0);
}

template <typename T>
void UnitSync::bind(std::string const & name, T & fp)
{
    void * f = ::dlsym(lib_, name.c_str());
    if (f == NULL)
    {
        throw std::invalid_argument("dlsym failed:" + name);
    }
    fp = reinterpret_cast<T>(f);
}
