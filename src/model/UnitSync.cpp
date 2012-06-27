#include "UnitSync.h"

#include <stdexcept>
#include <cassert>
#include <dlfcn.h>

#define BIND( NAME ) bind( #NAME , NAME )

UnitSync::UnitSync(std::string const & path)
{
    lib_ = ::dlopen(path.c_str(), RTLD_LAZY);
    if (lib_ == NULL)
    {
        throw std::invalid_argument("dlopen failed:" + path);
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
    BIND(ProcessUnitsNoChecksum);
    BIND(GetUnitCount);
    BIND(GetUnitName);
    BIND(GetFullUnitName);
    BIND(AddArchive);
    BIND(AddAllArchives);
    BIND(RemoveAllArchives);
    BIND(GetArchiveChecksum);
    BIND(GetArchivePath);
//  BIND(GetMapInfoEx);
//  BIND(GetMapInfo);
    BIND(GetMapCount);
    BIND(GetMapName);
    BIND(GetMapFileName);
    BIND(GetMapDescription);
    BIND(GetMapAuthor);
    BIND(GetMapWidth);
    BIND(GetMapHeight);
    BIND(GetMapTidalStrength);
    BIND(GetMapWindMin);
    BIND(GetMapWindMax);
    BIND(GetMapGravity);
    BIND(GetMapResourceCount);
    BIND(GetMapResourceName);
    BIND(GetMapResourceMax);
    BIND(GetMapResourceExtractorRadius);
    BIND(GetMapPosCount);
    BIND(GetMapPosX);
    BIND(GetMapPosZ);
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
    BIND(GetInfoValue);
    BIND(GetInfoValueString);
    BIND(GetInfoValueInteger);
    BIND(GetInfoValueFloat);
    BIND(GetInfoValueBool);
    BIND(GetInfoDescription);
    BIND(GetSkirmishAIOptionCount);
    BIND(GetPrimaryModCount);
    BIND(GetPrimaryModInfoCount);
    BIND(GetPrimaryModName);
    BIND(GetPrimaryModShortName);
    BIND(GetPrimaryModVersion);
    BIND(GetPrimaryModMutator);
    BIND(GetPrimaryModGame);
    BIND(GetPrimaryModShortGame);
    BIND(GetPrimaryModDescription);
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
    BIND(GetOptionStyle);
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
    BIND(OpenArchiveType);
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
