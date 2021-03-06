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
    @file   SCLabel.cpp
    @date   22, June 2020
    
    ===============================================================
 */
 
#pragma once

#include <jaut_gui/jaut_gui.h>

class SCLabel : public juce::Label
{
public:
    using juce::Label::Label;
    
    //==================================================================================================================
    void paint(juce::Graphics &g) override
    {
        juce::LookAndFeel &laf = getLookAndFeel();
        
        g.fillAll(findColour(backgroundColourId));
    
        if (!isBeingEdited())
        {
            float alpha = isEnabled() ? 1.0f : 0.5f;
            const juce::Font font (laf.getLabelFont(*this));
        
            g.setColour(findColour(textColourId).withMultipliedAlpha(alpha));
            g.setFont(font);
        
            auto textArea = laf.getLabelBorderSize(*this).subtractedFrom(getLocalBounds());
            jaut::FontFormat::drawSmallCaps(g, getText(), textArea.toFloat(), getJustificationType());
            
            g.setColour(findColour(outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (isEnabled())
        {
            g.setColour(findColour(outlineColourId));
        }
    
        g.drawRect(getLocalBounds());
    }
};
