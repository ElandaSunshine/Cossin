/**
 * ===============================================================
 * This file is part of the Esac-Jaut library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2019 ElandaSunshine
 * ===============================================================
 *
 * Author: Elanda
 * File: AppData.h
 * Time: 4, Mai 2019
 *
 * ===============================================================
 */

#include <jaut/appdata.h>
#include <jaut/exception.h>

namespace jaut
{
namespace
{
    static Directory invalidDirectory;

    //=================================================================================================================
    void generateSubDirList(StringArray &strArray, const String &dirName) noexcept
    {
        String::CharPointerType charptr = dirName.getCharPointer();
        juce_wchar character;
        String text = "";

        for (;;)
        {
            character = charptr.getAndAdvance();

            if (character == 0)
            {
                break;
            }

            if (character == '\\' || character == '/')
            {
                juce_wchar nextchar = *charptr;
                if (nextchar != 0)
                {
                    if (nextchar != '/' && nextchar != '\\' && !text.isEmpty())
                    {
                        strArray.add(text);
                        text = "";
                    }

                    ++charptr;
                }
                else
                {
                    break;
                }
            }
            else
            {
                text += character;
            }
        }

        if (!text.isEmpty())
        {
            strArray.add(text);
        }
    }

    Directory &recursivelySearchForDesiredDirectory(Directory &dir, StringArray &strArray, int level) noexcept
    {
        Directory *thisdir = &dir;

        if (level < strArray.size())
        {
            String key = strArray[level++];

            if (dir.hasSubDirectory(key))
            {
                Directory nextdir = dir[key];
                thisdir = &recursivelySearchForDesiredDirectory(nextdir, strArray, level);
            }
        }

        return *thisdir;
    }

    void searchOnFileSystemFor(Directory::t_file_type type, File &searchRoot,
                               const Directory::t_search_set &searchPatternList, bool recursive,
                               Directory::t_file_vec &files) noexcept
    {
        File basedirfile = searchRoot;

        if (basedirfile.exists())
        {
            String searchpattern = "";
            bool first = true;

            for (auto it = searchPatternList.begin(); it != searchPatternList.end(); ++it)
            {
                searchpattern += (first ? "*." : ";*.") + it->trim();
                first = false;
            }

            DirectoryIterator diriterator(basedirfile, recursive, searchpattern, type);

            while (diriterator.next())
            {
                (void)files.push_back(diriterator.getFile());
            }
        }
    }
}

/* ==================================================================================
 * ================================= SharedObject ===================================
 * ================================================================================== */

struct Directory::SharedObject final
{
    String baseDir;
    String name;
    std::weak_ptr<SharedObject> parent;
    t_sub_map subDirectories;

    SharedObject(const String &name, const String &baseDir)
        : baseDir(baseDir),
          name(name),
          subDirectories(10)
    {}
};



/* ==================================================================================
 * ================================== Directory =====================================
 * ================================================================================== */

Directory::Directory(const String &name, const String &baseDir) noexcept
    : data(new SharedObject(name, baseDir), [](SharedObject *so) { delete so; })
{
    /**
       The file base can't be created therefor it has to exists already.
       Make sure your filesystem is prepared when you are using this class!
    */
    jassert(File(baseDir).exists());
}

//==============================================================================
Directory Directory::operator[](const String &dirName) noexcept
{
    if (!data)
    {
        return invalidDirectory;
    }

    if (dirName.containsAnyOf("/\\"))
    {
        StringArray dirlist;
        generateSubDirList(dirlist, dirName);

        if (dirlist.size() > 0)
        {
            if (hasSubDirectory(dirName))
            {
                return recursivelySearchForDesiredDirectory(data->subDirectories.getReference(dirName), dirlist, 1);
            }
        }

        return invalidDirectory;
    }

    if (!hasSubDirectory(dirName))
    {
        (void)withSubFolder(dirName);
    }

    return data->subDirectories.getReference(dirName);
}

const bool Directory::operator==(const Directory &dir) const noexcept
{
    return data && data.get() == dir.data.get();
}

const bool Directory::operator!=(const Directory &dir) const noexcept
{
    return !data || data.get() != dir.data.get();
}

/*
const bool Directory::operator<(const Directory &dir) const noexcept
{
    if (!isValidDirectory())
        return false;

    Directory *directory = pParent;

    do
    {
        if (&dir == directory)
            return true;
    }
    while(directory != 0 && (directory = directory->pParent) != 0);

    return false;
}

const bool Directory::operator<=(const Directory &dir) const noexcept
{
    if (!isValidDirectory())
        return false;

    if(&dir == this)
        return true;

    Directory *directory = pParent;

    do
    {
        if (&dir == directory)
            return true;
    }
    while(directory != 0 && (directory = directory->pParent) != 0);

    return false;
}
// */

const bool Directory::operator>(const Directory &dir) const noexcept
{
    if (!data || !dir.data)
    {
        return false;
    }

    p_shared_object dirparent(dir.data->parent.lock());

    if(dirparent)
    {
        do
        {
            if (dirparent.get() == data.get())
            {
                return true;
            }
        }
        while(dirparent = dirparent->parent.lock());
    }

    return false;
}

const bool Directory::operator>=(const Directory &dir) const noexcept
{
    return *this > dir || *this == dir;
}

const bool Directory::operator+(const Directory &dir) const noexcept
{
    if(!data || !dir.data)
    {
        return false;
    }

    p_shared_object dirparent(dir.data->parent.lock());

    return dirparent && dirparent.get() == data.get();
}

const bool Directory::operator+=(const Directory &dir) const noexcept
{
    return *this + dir || &dir == this;
}

const bool Directory::operator*(const Directory &dir) const noexcept
{
    return dir > *this || *this > dir;
}

const bool Directory::operator*=(const Directory &dir) const noexcept
{
    return *this * dir || &dir == this;
}

const bool Directory::operator/(const Directory &dir) const noexcept
{
    if(!data || !dir.data)
    {
        return false;
    }

    p_shared_object thisparent(data->parent.lock());
    p_shared_object dirparent(dir.data->parent.lock());

    return thisparent && thisparent.get() == dirparent.get();
}

const bool Directory::operator/=(const Directory &dir) const noexcept
{
    return *this / dir || &dir == this;
}

//==============================================================================
Directory Directory::withSubFolder(const String &directoryName) noexcept
{
    if(data)
    {
        bool flag = directoryName.containsAnyOf("\\/");

        // '\' and '/' characters are not allowed in the path with
        // this function call.
        jassert(!flag);

        if (!flag && !hasSubDirectory(directoryName))
        {
            Directory directory(directoryName, getBaseDir());
            directory.data->parent  = data;
            directory.data->baseDir = data->baseDir;
            data->subDirectories.set(directoryName, directory);
        }
    }

    return invalidDirectory;
}

Directory Directory::makeDir(const String &directoryName)
{
    if (directoryName.containsAnyOf("\\/") || hasSubDirectory(directoryName))
    {
        throw std::invalid_argument(String("Directory '" + directoryName + "' cannot be added to '"
                                           + data->name + "").toStdString());
    }

    (void) withSubFolder(directoryName);

    return (*this)[directoryName];
}

const Directory Directory::getDir(const String &directoryName) const noexcept
{
    if (!hasSubDirectory(directoryName))
    {
        return Directory();
    }

    return data->subDirectories[directoryName];
}

//==============================================================================
const bool Directory::isValidDirectory() const noexcept
{
    return data != nullptr;
}

const bool Directory::isRootDirectory() const noexcept
{
    return data && !data->parent.lock();
}

const bool Directory::exists() const noexcept
{
    return toFile().exists();
}

const bool Directory::hasSubDirectories() const noexcept
{
    return numSubDirectories() > 0;
}

const bool Directory::hasSubDirectory(const String &name) const noexcept
{
    if(!data)
    {
        return false;
    }

    return data->subDirectories.contains(name.trim().toLowerCase());
}

const bool Directory::hasParentDirectory() const noexcept
{
    return data && data->parent.lock();
}

const size_t Directory::numSubDirectories() const noexcept
{
    if(!data)
    {
        return false;
    }

    return data->subDirectories.size();
}

const String Directory::getBaseDir() const noexcept
{
    if(!data)
    {
        return "";
    }

    return data->baseDir;
}

const String Directory::getDirectoryName() const noexcept
{
    if(!data)
    {
        return "";
    }

    return data->name;
}

File Directory::getFile(const String &fileName) const noexcept
{
    if (!data)
    {
        return File();
    }

    return File(toString(true) + "/" + fileName);
}

Directory Directory::getParentDirectory() const noexcept
{
    if(!data)
    {
        return invalidDirectory;
    }

    Directory dir;
    dir.data = data->parent.lock();

    return !dir.data ? invalidDirectory : dir;
}

//==============================================================================
Directory::t_file_vec Directory::listFiles(const t_search_set &fileNames, bool recursiveSearch) const noexcept
{
    if (!data)
    {
        return t_file_vec();
    }

    t_file_vec files;
    File directory = toFile();
    searchOnFileSystemFor(t_file_type::findFiles, directory, fileNames, recursiveSearch, files);

    return files;
}

Directory::t_file_vec Directory::listDirectories(const t_search_set &fileNames, bool recursiveSearch) const noexcept
{
    if (!data)
    {
        return t_file_vec();
    }

    t_file_vec files;
    File directory = toFile();
    searchOnFileSystemFor(t_file_type::findDirectories, directory, fileNames, recursiveSearch, files);

    return files;
}

Directory::t_iterator Directory::begin() noexcept
{
    if(!data)
    {
        return t_sub_map(0).begin();
    }

    return data->subDirectories.begin();
}

Directory::t_iterator Directory::end() noexcept
{
    if(!data)
    {
        return t_sub_map(0).end();
    }

    return data->subDirectories.end();
}

File Directory::toFile() const noexcept
{
    if (!data)
    {
        return File();
    }

    return File(toString(true));
}

const String Directory::toString(bool absolute) const noexcept
{
    if (!data)
    {
        return "";
    }

    String fullpath;
    p_shared_object sharedobject(data);

    do
    {
        fullpath = sharedobject->name + (fullpath.isEmpty() ? "" : "/" + fullpath);
    }
    while(sharedobject = sharedobject->parent.lock());

    return absolute ? (data->baseDir + fullpath) : fullpath;
}

void Directory::createFolderHierarchy(bool recursive) const noexcept
{
    if (!data)
    {
        return;
    }

    for (auto it = data->subDirectories.begin(); it != data->subDirectories.end(); ++it)
    {
        Directory dir = data->subDirectories[it.getKey()];
        File newdir   = dir.toFile();

        if (!newdir.exists())
        {
            (void) newdir.createDirectory();
        }

        if (recursive && dir.hasSubDirectories())
        {
            dir.createFolderHierarchy();
        }
    }
}

/* ==================================================================================
 * =================================== AppData ======================================
 * ================================================================================== */

AppData::AppData(const File &parentDirectory, const String &rootName, const t_sub_vec &subDirectories)
    : Directory(rootName, parentDirectory.getFullPathName())
{
    data->baseDir = parentDirectory.exists() ? parentDirectory.getFullPathName()
                    : File::getSpecialLocation(t_sp_loc::userDocumentsDirectory).getFullPathName();

    if (!data->baseDir.endsWith("/") && !rootName.startsWith("/"))
    {
        data->baseDir += "/";
    }

    dataDir = File(data->baseDir + rootName);

    if (!dataDir.exists() && !dataDir.createDirectory().wasOk())
    {
        throw exception::AppDataRootCreation();
    }

    for (auto name : subDirectories)
    {
        (void) withSubFolder(name);
    }
}

AppData::AppData(const File &parentDirectory, const t_sub_vec &subDirectories)
    : AppData(parentDirectory.getParentDirectory(), parentDirectory.getFileName(), subDirectories)
{}

AppData::AppData(const String &baseDirectoryName, const t_sub_vec &subDirectories)
    : AppData(File::getSpecialLocation(t_sp_loc::userDocumentsDirectory), baseDirectoryName, subDirectories)
{}

AppData::AppData(const File &parentDirectory, const String &baseDirectoryName)
    : AppData(parentDirectory, baseDirectoryName, t_sub_vec())
{}

AppData::AppData(const File &parentDirectory)
    : AppData(parentDirectory.getParentDirectory(), parentDirectory.getFileName(), t_sub_vec())
{}

AppData::AppData(const String &baseDirectoryName)
    : AppData(File::getSpecialLocation(t_sp_loc::userDocumentsDirectory), baseDirectoryName, t_sub_vec())
{}

//==============================================================================
AppData &AppData::withSubFolder(const String &directoryName) noexcept
{
    if(data)
    {
        bool flag = directoryName.containsAnyOf("\\/");

        /**
            '\' and '/' characters are not allowed in the path with
            this function call.
            */
        jassert(!flag);

        if (!flag && !hasSubDirectory(directoryName))
        {
            Directory directory(directoryName, data->baseDir);
            directory.data->parent  = data;
            directory.data->baseDir = data->baseDir;
            (void) data->subDirectories.set(directoryName, directory);
        }
    }

    return *this;
}

Directory AppData::getSubDirOrPossibleRewrite(const String &directoryName) noexcept
{
    String lname = directoryName.trim().toLowerCase();

    if (rewrites.find(lname) != rewrites.end())
    {
        return rewrites.at(lname);
    }

    return invalidDirectory;
}

void AppData::addRewrite(const String &rewriteName, Directory &dir) noexcept
{
    String lname = rewriteName.trim().toLowerCase();

    // checking if directory is a direct child of this AppData object
    if (*this + dir && rewrites.find(lname) == rewrites.end())
    {
        (void) rewrites.emplace(lname, dir);
    }
}
}
