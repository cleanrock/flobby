// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

class UnitSync {
public:
    UnitSync(std::string const & path); // throws on failure
    virtual ~UnitSync();

    // UnitSync API
    const char*  (*GetNextError)();
    const char*  (*GetSpringVersion)();
    const char*  (*GetSpringVersionPatchset)();
    bool         (*IsSpringReleaseVersion)();
    int          (*Init)(bool isServer, int id);
    void         (*UnInit)();
    const char*  (*GetWritableDataDirectory)();
    int          (*GetDataDirectoryCount)();
    const char*  (*GetDataDirectory)(int index);
    int          (*ProcessUnits)();
    int          (*ProcessUnitsNoChecksum)();
    int          (*GetUnitCount)();
    const char*  (*GetUnitName)(int unit);
    const char*  (*GetFullUnitName)(int unit);
    void         (*AddArchive)(const char* archiveName);
    void         (*AddAllArchives)(const char* rootArchiveName);
    void         (*RemoveAllArchives)();
    unsigned int (*GetArchiveChecksum)(const char* archiveName);
    const char*  (*GetArchivePath)(const char* archiveName);
//  int          (*GetMapInfoEx)(const char* mapName, MapInfo* outInfo, int version);
//  int          (*GetMapInfo)(const char* mapName, MapInfo* outInfo);
    int          (*GetMapCount)();
    const char*  (*GetMapName)(int index);
    const char*  (*GetMapFileName)(int index);
    const char*  (*GetMapDescription)(int index);
    const char*  (*GetMapAuthor)(int index);
    int          (*GetMapWidth)(int index);
    int          (*GetMapHeight)(int index);
    int          (*GetMapTidalStrength)(int index);
    int          (*GetMapWindMin)(int index);
    int          (*GetMapWindMax)(int index);
    int          (*GetMapGravity)(int index);
    int          (*GetMapResourceCount)(int index);
    const char*  (*GetMapResourceName)(int index, int resourceIndex);
    float        (*GetMapResourceMax)(int index, int resourceIndex);
    int          (*GetMapResourceExtractorRadius)(int index, int resourceIndex);
    int          (*GetMapPosCount)(int index);
    float        (*GetMapPosX)(int index, int posIndex);
    float        (*GetMapPosZ)(int index, int posIndex);
    float        (*GetMapMinHeight)(const char* mapName);
    float        (*GetMapMaxHeight)(const char* mapName);
    int          (*GetMapArchiveCount)(const char* mapName);
    const char*  (*GetMapArchiveName)(int index);
    unsigned int (*GetMapChecksum)(int index);
    unsigned int (*GetMapChecksumFromName)(const char* mapName);
    unsigned short* (*GetMinimap)(const char* fileName, int mipLevel);
    int          (*GetInfoMapSize)(const char* mapName, const char* name, int* width, int* height);
    int          (*GetInfoMap)(const char* mapName, const char* name, unsigned char* data, int typeHint);
    int          (*GetSkirmishAICount)();
    int          (*GetSkirmishAIInfoCount)(int index);
    const char*  (*GetInfoKey)(int index);
    const char*  (*GetInfoType)(int index);
    const char*  (*GetInfoValue)(int index);
    const char*  (*GetInfoValueString)(int index);
    int          (*GetInfoValueInteger)(int index);
    float        (*GetInfoValueFloat)(int index);
    bool         (*GetInfoValueBool)(int index);
    const char*  (*GetInfoDescription)(int index);
    int          (*GetSkirmishAIOptionCount)(int index);
    int          (*GetPrimaryModCount)();
    int          (*GetPrimaryModInfoCount)(int index);
    const char*  (*GetPrimaryModName)(int index);
    const char*  (*GetPrimaryModShortName)(int index);
    const char*  (*GetPrimaryModVersion)(int index);
    const char*  (*GetPrimaryModMutator)(int index);
    const char*  (*GetPrimaryModGame)(int index);
    const char*  (*GetPrimaryModShortGame)(int index);
    const char*  (*GetPrimaryModDescription)(int index);
    const char*  (*GetPrimaryModArchive)(int index);
    int          (*GetPrimaryModArchiveCount)(int index);
    const char*  (*GetPrimaryModArchiveList)(int archive);
    int          (*GetPrimaryModIndex)(const char* name);
    unsigned int (*GetPrimaryModChecksum)(int index);
    unsigned int (*GetPrimaryModChecksumFromName)(const char* name);
    int          (*GetSideCount)();
    const char*  (*GetSideName)(int side);
    const char*  (*GetSideStartUnit)(int side);
    int          (*GetMapOptionCount)(const char* mapName);
    int          (*GetModOptionCount)();
    int          (*GetCustomOptionCount)(const char* fileName);
    const char*  (*GetOptionKey)(int optIndex);
    const char*  (*GetOptionScope)(int optIndex);
    const char*  (*GetOptionName)(int optIndex);
    const char*  (*GetOptionSection)(int optIndex);
    const char*  (*GetOptionStyle)(int optIndex);
    const char*  (*GetOptionDesc)(int optIndex);
    int          (*GetOptionType)(int optIndex);
    int          (*GetOptionBoolDef)(int optIndex);
    float        (*GetOptionNumberDef)(int optIndex);
    float        (*GetOptionNumberMin)(int optIndex);
    float        (*GetOptionNumberMax)(int optIndex);
    float        (*GetOptionNumberStep)(int optIndex);
    const char*  (*GetOptionStringDef)(int optIndex);
    int          (*GetOptionStringMaxLen)(int optIndex);
    int          (*GetOptionListCount)(int optIndex);
    const char*  (*GetOptionListDef)(int optIndex);
    const char*  (*GetOptionListItemKey)(int optIndex, int itemIndex);
    const char*  (*GetOptionListItemName)(int optIndex, int itemIndex);
    const char*  (*GetOptionListItemDesc)(int optIndex, int itemIndex);
    int          (*GetModValidMapCount)();
    const char*  (*GetModValidMap)(int index);
    int          (*OpenFileVFS)(const char* name);
    void         (*CloseFileVFS)(int file);
    int          (*ReadFileVFS)(int file, unsigned char* buf, int numBytes);
    int          (*FileSizeVFS)(int file);
    int          (*InitFindVFS)(const char* pattern);
    int          (*InitDirListVFS)(const char* path, const char* pattern, const char* modes);
    int          (*InitSubDirsVFS)(const char* path, const char* pattern, const char* modes);
    int          (*FindFilesVFS)(int file, char* nameBuf, int size);
    int          (*OpenArchive)(const char* name);
    int          (*OpenArchiveType)(const char* name, const char* type);
    void         (*CloseArchive)(int archive);
    int          (*FindFilesArchive)(int archive, int file, char* nameBuf, int* size);
    int          (*OpenArchiveFile)(int archive, const char* name);
    int          (*ReadArchiveFile)(int archive, int file, unsigned char* buffer, int numBytes);
    void         (*CloseArchiveFile)(int archive, int file);
    int          (*SizeArchiveFile)(int archive, int file);
    void         (*SetSpringConfigFile)(const char* fileNameAsAbsolutePath);
    const char*  (*GetSpringConfigFile)();
    const char*  (*GetSpringConfigString)(const char* name, const char* defValue);
    int          (*GetSpringConfigInt)(const char* name, const int defValue);
    float        (*GetSpringConfigFloat)(const char* name, const float defValue);
    void         (*SetSpringConfigString)(const char* name, const char* value);
    void         (*SetSpringConfigInt)(const char* name, const int value);
    void         (*SetSpringConfigFloat)(const char* name, const float value);

    // LUA API ???
    void        (*lpClose)();
    int         (*lpOpenFile)(const char* fileName, const char* fileModes, const char* accessModes);
    int         (*lpOpenSource)(const char* source, const char* accessModes);
    int         (*lpExecute)();
    const char* (*lpErrorLog)();
    void        (*lpAddTableInt)(int key, int override);
    void        (*lpAddTableStr)(const char* key, int override);
    void        (*lpEndTable)();
    void        (*lpAddIntKeyIntVal)(int key, int value);
    void        (*lpAddStrKeyIntVal)(const char* key, int value);
    void        (*lpAddIntKeyBoolVal)(int key, int value);
    void        (*lpAddStrKeyBoolVal)(const char* key, int value);
    void        (*lpAddIntKeyFloatVal)(int key, float value);
    void        (*lpAddStrKeyFloatVal)(const char* key, float value);
    void        (*lpAddIntKeyStrVal)(int key, const char* value);
    void        (*lpAddStrKeyStrVal)(const char* key, const char* value);
    int         (*lpRootTable)();
    int         (*lpRootTableExpr)(const char* expr);
    int         (*lpSubTableInt)(int key);
    int         (*lpSubTableStr)(const char* key);
    int         (*lpSubTableExpr)(const char* expr);
    void        (*lpPopTable)();
    int         (*lpGetKeyExistsInt)(int key);
    int         (*lpGetKeyExistsStr)(const char* key);
    int         (*lpGetIntKeyType)(int key);
    int         (*lpGetStrKeyType)(const char* key);
    int         (*lpGetIntKeyListCount)();
    int         (*lpGetIntKeyListEntry)(int index);
    int         (*lpGetStrKeyListCount)();
    const char* (*lpGetStrKeyListEntry)(int index);
    int         (*lpGetIntKeyIntVal)(int key, int defValue);
    int         (*lpGetStrKeyIntVal)(const char* key, int defValue);
    int         (*lpGetIntKeyBoolVal)(int key, int defValue);
    int         (*lpGetStrKeyBoolVal)(const char* key, int defValue);
    float       (*lpGetIntKeyFloatVal)(int key, float defValue);
    float       (*lpGetStrKeyFloatVal)(const char* key, float defValue);
    const char* (*lpGetIntKeyStrVal)(int key, const char* defValue);
    const char* (*lpGetStrKeyStrVal)(const char* key, const char* defValue);

private:
    void * lib_;

    template <typename T>
    void bind(std::string const & name, T & fp);
};

