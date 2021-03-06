/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any internal version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda
    @file   PluginProcessor.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CossinDef.h"
#include "SharedData.h"
#include "Resources.h"

#include <jaut_provider/jaut_provider.h>

namespace
{
//======================================================================================================================
inline constexpr float Const_Pi                        = 3.14159f;
inline constexpr float Const_LinearPanningCompensation = 2.0f;
inline constexpr float Const_SquarePanningCompensation = 1.41421356238f;
inline constexpr float Const_SinePanningCompensation   = 1.41421356238f;

inline constexpr int Resolution_LookupTable = 200;

//======================================================================================================================
template<class Member, class ...Args>
std::unique_ptr<Member> newParameter(Member *&member, Args &&...args)
{
    return std::unique_ptr<Member>((member = new Member(std::forward<Args>(args)...)));
}

auto createSineTable() noexcept
{
    std::array<float, Resolution_LookupTable + 1> table {};
    constexpr int size = static_cast<std::size_t>(table.size());
    
    for (int i = 0; i < size; ++i)
    {
        table[static_cast<std::size_t>(i)] = std::sqrt(static_cast<float>(i) / 100.0f) *
                                             Const_SquarePanningCompensation;
    }
    
    return table;
}

auto createSquareTable() noexcept
{
    std::array<float, ::Resolution_LookupTable + 1> table {};
    constexpr int size = static_cast<std::size_t>(table.size());
    
    for (int i = 0; i < size; ++i)
    {
        table[static_cast<std::size_t>(i)] = std::sin((static_cast<float>(i) / 100.0f) * (Const_Pi / 2.0f)) *
                                             Const_SinePanningCompensation;
    }
    
    return table;
}
}

//======================================================================================================================
CossinAudioProcessor::CossinAudioProcessor()
     : AudioProcessor(getDefaultBusesLayout()),
       parameters(*this, nullptr, "CossinState", getParameters())
{
    initialize();
}

CossinAudioProcessor::~CossinAudioProcessor() = default;

//======================================================================================================================
const juce::String CossinAudioProcessor::getName() const
{
    return "Cossin";
}

//======================================================================================================================
void CossinAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const float gain   = parGain->get();
    const int pan_mode = parPanMode->get();
    previousGain[0]    = gain * calculatePanningGain(pan_mode, 0);
    previousGain[1]    = gain * calculatePanningGain(pan_mode, 1);
    
    metreSource.resize(getMainBusNumOutputChannels(), static_cast<int>(0.02f * sampleRate / samplesPerBlock));
}

void CossinAudioProcessor::releaseResources()
{
}

bool CossinAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    const juce::AudioChannelSet main_bus = layouts.getMainOutputChannelSet();

    if((main_bus != juce::AudioChannelSet::mono() && main_bus != juce::AudioChannelSet::stereo())
       || main_bus == juce::AudioChannelSet::disabled())
    {
        return false;
    }

    return main_bus == layouts.getMainInputChannelSet();
}

void CossinAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals denormals;

    const float gain   = parGain->get();
    const int pan_mode = parPanMode->get();
    
    for (auto i = 0; i < buffer.getNumChannels(); ++i)
    {
        const float current_gain = gain * calculatePanningGain(pan_mode, i);
        
        if (current_gain == previousGain[i])
        {
            buffer.applyGain(i, 0, buffer.getNumSamples(), current_gain);
        }
        else
        {
            buffer.applyGainRamp(i, 0, buffer.getNumSamples(), previousGain[i], current_gain);
            previousGain[i] = current_gain;
        }
    }

    metreSource.measureBlock(buffer);
}

//======================================================================================================================
bool CossinAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CossinAudioProcessor::createEditor()
{
    return new CossinMainEditorWindow(*this, parameters, metreSource);
}

//======================================================================================================================
void CossinAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ValueTree state = parameters.copyState();

    if (state.isValid())
    {
        COSSIN_IS_STANDALONE({})
        COSSIN_STANDALONE_ELSE
        (
            // Gui data
            state.setProperty("GuiWidth",  windowBounds.getWidth(), nullptr);
            state.setProperty("GuiHeight", windowBounds.getHeight(), nullptr);
        )

        // Property data
        juce::ValueTree property_tree = state.getOrCreateChildWithName("Properties", nullptr);

        // Dump data
        std::unique_ptr<juce::XmlElement> state_xml(state.createXml());
        copyXmlToBinary(*state_xml, destData);

        // Store output debug information
      #ifdef JUCE_DEBUG
        state_xml->writeTo(juce::File::getCurrentWorkingDirectory().getChildFile("../../plugin_state.xml"));
      #endif
    }
}

void CossinAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> state_xml(getXmlFromBinary(data, sizeInBytes));

    if (state_xml.get() && state_xml->hasTagName(parameters.state.getType()))
    {
        const juce::ValueTree state = juce::ValueTree::fromXml(*state_xml);

        if (state.isValid())
        {
            COSSIN_IS_STANDALONE()
            COSSIN_STANDALONE_ELSE
            (
                // Gui data
                windowBounds.setBounds(0, 0, state.getProperty("GuiWidth", -1), state.getProperty("GuiHeight", -1));
            )
            
            // Contain data
            parameters.replaceState(state);
        }
    }
}

//======================================================================================================================
juce::Rectangle<int> &CossinAudioProcessor::getWindowSize() noexcept
{
    return windowBounds;
}

//======================================================================================================================
void CossinAudioProcessor::initialize()
{
    SharedData::ReadLock lock(*sharedData);

    // Default init properties
    const jaut::Config &config = sharedData->Configuration();
    auto property_window_size  = config.getProperty("size", res::Cfg_Defaults);

    COSSIN_IS_STANDALONE({})
    COSSIN_STANDALONE_ELSE
    (
        windowBounds.setBounds(0, 0, property_window_size->getProperty("width") ->getValue(),
                                     property_window_size->getProperty("height")->getValue());
    )

    // Misc
    metreSource.setMaxHoldMS(50);
}

float CossinAudioProcessor::calculatePanningGain(int panMode, int channel) const noexcept
{
    static auto SineLookupTable   = ::createSineTable();
    static auto SquareLookupTable = ::createSquareTable();
    
    jassert(jaut::fit<int>(panMode, 0, res::List_PanningModes.size()));

    if (panMode == 0) // linear
    {
        const float panning_p    = parPanning->get() / 2.0f + 0.5f;
        const float channel_mod  = channel == 0 ? 1.0f - panning_p : panning_p;
        return channel_mod * Const_LinearPanningCompensation;
    }
    else
    {
        const int panning_p   = juce::roundToInt(parPanning->get() * 100.0f) + 100;
        const int table_index = channel == 0 ? 200 - panning_p : panning_p;
        return panMode == 1 ? SquareLookupTable[static_cast<std::size_t>(table_index)]
                            : SineLookupTable  [static_cast<std::size_t>(table_index)];
    }
}

//======================================================================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CossinAudioProcessor::getParameters()
{
    using Range = juce::NormalisableRange<float>;
    
    const int last_panning_mode = res::List_PanningModes.size() - 1;
    const int last_process_mode = res::List_ProcessModes.size() - 1;
    
    const jaut::Config &config  = sharedData->Configuration();
    const int default_pan_mode  = std::clamp<int>(config.getProperty(res::Prop_DefaultsPanningMode, res::Cfg_Defaults)
                                                  ->getValue(), 0, last_panning_mode);
    //const int default_processor = std::clamp<int>(config.getProperty(res::Prop_DefaultsProcessMode, res::Cfg_Defaults)
    //                                              ->getValue(), 0, last_process_mode);
    
    return {
        // Volume parameter
        ::newParameter(parGain, ParameterIds::MasterLevel, "Global level", Range(0.0f, 1.0f, 0.0f, 0.5f), 1.0f, "",
                       juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength)
                       {
                           const juce::String db(std::round(juce::Decibels::gainToDecibels(value) * 100.0f) / 100.0f);
                           return ((value > 0 ? db : "-INF") + "dB").substring(maximumStringLength);
                       }),
        
        // Mix parameter
        ::newParameter(parMix, ParameterIds::MasterMix, "Global mix", Range(0.0f, 1.0f), 1.0f, "",
                       juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength)
                       {
                            return (juce::String(static_cast<int>(value * 100)) + "%")
                                    .substring(maximumStringLength);
                       }),
        
        // Panning parameter
        ::newParameter(parPanning, ParameterIds::MasterPan, "Global pan", Range(-1.0f, 1.0f, 0.01f, 0.5f, true), 0.0f,
                       "", juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength) -> juce::String
                       {
                           if (std::fmod(value, 1.0f) == 0.0f)
                           {
                               return value == 0.0f ? "Center" : (value == 1.0f ? "Right" : "Left");
                           }
    
                           const int mod = static_cast<int>(value * 100.0f);
                           return (juce::String(100 - mod) + "% Left, " + juce::String(100 + mod) + "% Right")
                                   .substring(maximumStringLength);
                       }),
                       
        // Panning law parameter
        ::newParameter(parPanMode, ParameterIds::PropertyPanningMode, "Pan mode", 0, last_panning_mode,
                       default_pan_mode, "",
                       [](int value, int maximumStringLength)
                       {
                           return juce::String(res::List_PanningModes[static_cast<std::size_t>(value)])
                                        .substring(maximumStringLength);
                       })
    
        // TODO Processor mode parameter
        /*::newParameter(parProcMode, ParameterIds::PropertyProcessMode, "Process mode", 0, last_process_mode,
                       default_processor, "",
                       [](int value, int maximumStringLength)
                       {
                           return juce::String(res::List_PanModes[value]).substring(maximumStringLength);
                       })*/
    };
}

//======================================================================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CossinAudioProcessor();
}
