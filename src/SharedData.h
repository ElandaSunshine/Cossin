/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda (elanda@elandasunshine.xyz)
    @file   SharedData.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <juce_events/juce_events.h>
#include <jaut_provider/jaut_provider.h>

class CossinAudioProcessorEditor;

class SharedData final : public juce::ActionBroadcaster
{
public:
    JAUT_CREATE_EXCEPTION_WITH_STRING(AppDataFolderCreationException, "Couldn't create plugin data folders: ");
    
    enum class LockPriority
    {
        HIGH,
        LOW,
        NONE
    };
    
    struct ApplicationData
    {
        // The users app data dir
        juce::File dirAppData { juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory) };
        
        // The plugin data root
        juce::File dirRoot    { dirRoot.getChildFile("/ElandaSunshine/Cossin") };
        
        juce::File dirLang    { dirRoot.getChildFile("Lang")    };
        juce::File dirThemes  { dirRoot.getChildFile("Themes")  };
        juce::File dirPresets { dirRoot.getChildFile("Presets") };
    
        juce::File dirData      { dirRoot.getChildFile("Data")  };
        juce::File dirDataLogs  { dirData.getChildFile("Logs")  };
        juce::File dirDataSaves { dirData.getChildFile("Saves") };
    };
    
    class ReadLock
    {
    public:
        explicit ReadLock(SharedData &sharedData, LockPriority priority = LockPriority::HIGH)
            : sharedData(sharedData), lockWasSuccessful(priority == LockPriority::HIGH)
        {
            if(juce::JUCEApplicationBase::isStandaloneApp() || priority == LockPriority::NONE)
            {
                return;
            }

            if(priority == LockPriority::HIGH)
            {
                sharedData.rwLock.enterRead();
            }
            else
            {
                lockWasSuccessful = sharedData.rwLock.tryEnterRead();
            }
        }

        ~ReadLock()
        {
            if(juce::JUCEApplicationBase::isStandaloneApp())
            {
                return;
            }

            if(lockWasSuccessful)
            {
                sharedData.rwLock.exitRead();
            }
        }

    private:
        SharedData &sharedData;
        bool lockWasSuccessful;
    };

    class WriteLock
    {
    public:
        explicit WriteLock(SharedData &sharedData, LockPriority priority = LockPriority::HIGH)
            : sharedData(sharedData), lockWasSuccessful(priority == LockPriority::HIGH)
        {
            if(juce::JUCEApplicationBase::isStandaloneApp() || priority == LockPriority::NONE)
            {
                return;
            }

            if(priority == LockPriority::HIGH)
            {
                sharedData.rwLock.enterWrite();
            }
            else
            {
                lockWasSuccessful = sharedData.rwLock.tryEnterWrite();
            }
        }

        ~WriteLock()
        {
            if(juce::JUCEApplicationBase::isStandaloneApp())
            {
                return;
            }
            
            if(lockWasSuccessful)
            {
                sharedData.rwLock.exitWrite();
            }
        }

    private:
        SharedData &sharedData;
        bool lockWasSuccessful;
    };
    
    //==================================================================================================================
    static juce::SharedResourcePointer<SharedData> getInstance();

    //==================================================================================================================
    SharedData() noexcept;
    ~SharedData() override;

    //==================================================================================================================
    const ApplicationData& AppData() const noexcept;
    jaut::Config&          Configuration() noexcept;
    jaut::ThemeManager&    ThemeManager()  noexcept;
    jaut::Localisation&    Localisation()  noexcept;

    //==================================================================================================================
    const jaut::ThemePointer& getDefaultTheme()  const noexcept;
    const jaut::Localisation& getDefaultLocale() const noexcept;

    //==================================================================================================================
    void sendChangeToAllInstancesExcept(CossinAudioProcessorEditor* = nullptr) const;

private:
    friend class ReadLock;

    // Data
    ApplicationData                     appData;
    std::unique_ptr<jaut::Config>       appConfig;
    std::unique_ptr<jaut::ThemeManager> appThemes;
    std::unique_ptr<jaut::Localisation> appLocale;

    // Defaults
    jaut::ThemePointer defaultTheme;
    std::unique_ptr<jaut::Localisation> defaultLocale;

    // Misc
    mutable juce::ReadWriteLock rwLock;
    bool initialized;

    //==================================================================================================================
    void initialize();
    void initAppdata() const;
    void initConfig();
    void initDefaults();
    void initLangs();
    void initThemeManager();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SharedData)
};
