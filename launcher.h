/* This file is part of qfreerdp.
 *
 * qfreerdp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * qfreerdp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with qfreerdp; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _QFREERDP_LAUNCHER_H
#define _QFREERDP_LAUNCHER_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QSlider;

class Launcher : public QDialog
{
    Q_OBJECT

public:
    enum ResolutionType
    {
        RT_Standard,
        RT_Custom,
        RT_Fullscreen
    };

    enum CompressionType
    {
        CT_Disabled,
        CT_Default,
        CT_Level
    };

    enum PerformancePreset
    {
        PP_Minimum,
        PP_Low,
        PP_Mid,
        PP_High,
        PP_Custom
    };

    Launcher();

    void saveConfig();
    void restoreConfig();

private slots:
    void startXFreeRDP();
    void perfPresetChanged(int index);
    void perfItemChanged(bool);

private:
    // General
    QComboBox *m_server;
    QLineEdit *m_username;
    QLineEdit *m_password;

    // Display
    QComboBox *m_resolutionType;
    QSlider *m_resolution;
    QLineEdit *m_customWidth;
    QLineEdit *m_customHeight;
    QComboBox *m_depth;
    QList<QSize> m_availableResolutions;

    QComboBox *m_compression;
    QCheckBox *m_jpeg;
    QSlider *m_jpegLevel;

    // Devices
    QComboBox *m_audioMode;
    QCheckBox *m_clipboard;
    QCheckBox *m_redirectDrives;
    QCheckBox *m_redirectHome;

    // Experience
    QComboBox *m_performancePreset;
    QCheckBox *m_wallpaper;
    QCheckBox *m_fontSmoothing;
    QCheckBox *m_aero;
    QCheckBox *m_windowDrag;
    QCheckBox *m_menuAnims;
    QCheckBox *m_themes;

    QCheckBox *m_bitmapCache;
    QCheckBox *m_offscreenCache;
    QCheckBox *m_glyphCache;

    // Advanced
    QLineEdit *m_gateServer;
    QLineEdit *m_gateUsername;
    QLineEdit *m_gatePassword;
};

#endif
