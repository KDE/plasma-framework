/*
 *   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_PANELBACKGROUNDPROVIDER_H
#define PLASMA_PANELBACKGROUNDPROVIDER_H

namespace Plasma {
class Theme;

/**
 * A class that paints an additional background behind specific elements of a theme.
 * Construct it locally right before using it.
 */
class StandardThemeBackgroundProvider {
  public:
    /**
     * Constructs a background-provider for the given theme
     */
    StandardThemeBackgroundProvider(Theme* theme, QString imagePath);
    
    /**
     * Applies the background to the given target. The target must have correct alpha-values,
     * so the background can be painted under it. Also the clip-region must be already set correctly
     * to restrict the area where the background is painted.
     * @param target The target where the background should be painted
     * @param offset Additional offset for the rendering: The render-source is translated by this offset
     */
    void apply(QPainter& target) const;
    
    /**
     * Returns an identity that can be used for caching the result of the background rendering.
     * @return The identity string
     */
    QString identity() const;
    
    static void clearCache();
    
    /**
     * Returns true if  this background-provider will paint something
     */
    operator bool() const;
  private:
    QColor m_color;
    QString m_pattern;
    int m_patternAlpha;
    int m_offsetX;
    int m_offsetY;
    bool m_valid;
    
    //Maps file-name to (image, alpha)
    typedef QPair<QImage, int> PatternAlphaPair; //The alpha value is statically applied to the pattern
    static QMap<QString, PatternAlphaPair > m_cachedPatterns;
};

}

#endif
