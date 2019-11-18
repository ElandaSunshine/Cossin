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
 * File: thememanager.h
 * Time: 19, August 2019
 *
 * ===============================================================
 */

#pragma once

#include <jaut/expo.h>

namespace jaut
{

class IThemeDefinition;
class IMetadata;
class IMetaReader;

class JAUT_API ThemeManager final
{
public:
    struct JAUT_API Options final
    {
        String themeMetaId;
        String themePrefix;
        bool cacheThemes;

        //==============================================================================================================
        Options()
            : themeMetaId("packdata.json"),
              cacheThemes(true)
        {}
    };

    class JAUT_API ThemePointer final
    {
    public:
        using p_ptr_ref = std::shared_ptr<IThemeDefinition>;

        ThemePointer() noexcept;
        ThemePointer(std::nullptr_t) noexcept;
        ThemePointer(const String &name, IThemeDefinition *theme) noexcept;
        ThemePointer(const ThemePointer &other) noexcept;
        ThemePointer(ThemePointer &&other) noexcept;
        ~ThemePointer();

        //==============================================================================================================
        ThemePointer &operator=(const ThemePointer &right) noexcept;
        ThemePointer &operator=(ThemePointer &&right) noexcept;

        //==============================================================================================================
        IThemeDefinition &operator*() const;
        IThemeDefinition *operator->() const;
        operator const bool() const noexcept;

        //==============================================================================================================
        IThemeDefinition *get() const noexcept;

        //==============================================================================================================
        const bool isCached() const noexcept;

        //==============================================================================================================
        const String getName() const noexcept;

        //==============================================================================================================
        friend void swap(ThemePointer &left, ThemePointer &right) noexcept
        {
            std::swap(left.cached, right.cached);
            std::swap(left.manager, right.manager);
            std::swap(left.name, right.name);
            left.pointer.swap(right.pointer);
        }

    private:
        friend class ThemeManager;

        mutable bool cached;
        mutable ThemeManager *manager;
        String name;
        p_ptr_ref pointer;

        //==============================================================================================================
        void setThemeManager(ThemeManager *manager) noexcept;
    };

    using p_theme_ptr  = std::shared_ptr<IThemeDefinition>;
    using p_mread_ptr  = std::unique_ptr<IMetaReader>;
    using p_theme_sptr = std::shared_ptr<IThemeDefinition>;
    using f_pack_init  = std::function<IThemeDefinition*(const juce::File&, IMetadata *meta)>;
    using t_pack_map   = std::map<juce::String, ThemePointer>;

    //==================================================================================================================
    ThemeManager(const File &themeRoot, f_pack_init initializationCallback, std::unique_ptr<IMetaReader> metadataReader,
                 const Options &options = Options());
    ThemeManager(ThemeManager &&other) noexcept;
    ~ThemeManager();

    //==================================================================================================================
    ThemeManager &operator=(ThemeManager &&right) noexcept;

    //==================================================================================================================
    void reloadThemePacks();
    void clearThemePacks();
    void reloadThemePack(const String &themeId);

    //==================================================================================================================
    ThemePointer loadThemePack(const String &themeId) const;
    ThemePointer loadExternalThemePack(const File &themeFolder) const;
    ThemePointer loadExternalThemePack(const File &themeFolder, bool override);

    //==================================================================================================================
    ThemePointer getThemePack(const String &themeId) const;
    std::vector<ThemePointer> getAllThemePacks() const;
    const Options &getOptions() const noexcept;

    //==================================================================================================================
    friend void swap(ThemeManager &left, ThemeManager &right) noexcept
    {
        std::swap(left.initFunc, right.initFunc);
        std::swap(left.themeRoot, right.themeRoot);
        std::swap(left.options, right.options);
        left.metadataReader.swap(right.metadataReader);
        left.themeCache.swap(right.themeCache);
    }

private:
    friend class ThemeManager::ThemePointer;

    f_pack_init initFunc;
    p_mread_ptr metadataReader;
    Options options;
    t_pack_map themeCache;
    File themeRoot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};
}
