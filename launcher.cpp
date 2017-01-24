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

#include "launcher.h"

#include "qfreerdp.h"
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

static QList<QSize> s_standardResolutions {
    { 640,  480},
    { 800,  600},
    {1024,  768},
    {1280,  720},
    {1280,  768},
    {1280,  800},
    {1280,  960},
    {1280, 1024},
    {1360,  768},
    {1440,  900},
    {1400, 1050},
    {1600,  900},
    {1600,  960},   /* Not actually very common, but I like it */
    {1600, 1200},
    {1680, 1050},
    {1920, 1080},
    {1920, 1200},
    {1920, 1280},
    {1920, 1440},
    {2048, 1152},
    {2048, 1536},
    {2560, 1440},
    {2560, 1600},
    {2560, 1920},
    {3200, 1800},
    {3840, 2160},
    {4096, 2304},
    {4096, 3072},
    {5120, 2160},
    {5120, 2880},
    {6400, 4800},
    {7680, 4320},
    {8192, 4608}
};

static QList<QSize> getUsableResolutions()
{
    QList<QRect> screens;
    QDesktopWidget *desktop = QApplication::desktop();
    int maxScreens = desktop->screenCount();
    for (int i = 0; i < maxScreens; ++i)
        screens.append(desktop->screenGeometry(i));

    QList<QSize> result;
    for (const auto &res : s_standardResolutions) {
        for (int i = 0; i < maxScreens; ++i) {
            if (screens[i].width() >= res.width()
                    && screens[i].height() >= res.height()) {
                result.append(res);
                break;
            }
        }
    }

    return result;
}

static const QList<int> s_depths { 15, 16, 24, 32 };

Launcher::Launcher()
    : QDialog(Q_NULLPTR)
{
    QTabWidget *tabs = new QTabWidget(this);

    QWidget *generalTab = new QWidget(this);
    QGroupBox *loginGroup = new QGroupBox(tr("Login settings"), this);
    QLabel *serverLabel = new QLabel(tr("&Server:"), this);
    m_server = new QComboBox(this);
    m_server->setEditable(true);
    serverLabel->setBuddy(m_server);
    QLabel *usernameLabel = new QLabel(tr("&Username:"), this);
    m_username = new QLineEdit(this);
    usernameLabel->setBuddy(m_username);
    QLabel *usernameHelp = new QLabel(tr("NOTE: To set a domain, use the format DOMAIN\\username or username@DOMAIN"), this);
    usernameHelp->setWordWrap(true);
    QLabel *passwordLabel = new QLabel(tr("&Password:"), this);
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);
    passwordLabel->setBuddy(m_password);
    QLabel *passwordHelp = new QLabel(tr("Password will not be saved to disk"), this);
    passwordHelp->setWordWrap(true);
    QGridLayout *loginGrid = new QGridLayout(loginGroup);
    loginGrid->addWidget(serverLabel, 0, 0);
    loginGrid->addWidget(m_server, 0, 1);
    loginGrid->addWidget(usernameLabel, 1, 0);
    loginGrid->addWidget(m_username, 1, 1);
    loginGrid->addWidget(usernameHelp, 2, 1);
    loginGrid->addWidget(passwordLabel, 3, 0);
    loginGrid->addWidget(m_password, 3, 1);
    loginGrid->addWidget(passwordHelp, 4, 1);

    QVBoxLayout *generalLayout = new QVBoxLayout(generalTab);
    generalLayout->addWidget(loginGroup);
    generalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tabs->addTab(generalTab, tr("General"));

    QWidget *displayTab = new QWidget(this);
    QGroupBox *displayGroup = new QGroupBox(tr("Display settings"), this);
    QLabel *resolutionLabel = new QLabel("&Resolution:", this);
    m_resolutionType = new QComboBox(this);
    m_resolutionType->addItems(QStringList { tr("Standard Resolution"),
                                             tr("Custom Resolution"),
                                             tr("Full Screen") });
    m_resolution = new QSlider(Qt::Horizontal, this);
    m_resolution->setTickPosition(QSlider::TicksBelow);
    QLabel *resolutionHint = new QLabel(this);
    QWidget *customResolution = new QWidget(this);
    QValidator *resolutionValidator = new QIntValidator(100, 65535, this);
    m_customWidth = new QLineEdit(this);
    m_customWidth->setValidator(resolutionValidator);
    m_customHeight = new QLineEdit(this);
    m_customHeight->setValidator(resolutionValidator);
    QHBoxLayout *customResolutionLayout = new QHBoxLayout(customResolution);
    customResolutionLayout->setContentsMargins(0, 0, 0, 0);
    customResolutionLayout->addWidget(new QLabel(tr("Width:"), this));
    customResolutionLayout->addWidget(m_customWidth);
    customResolutionLayout->addWidget(new QLabel(tr(" x Height:"), this));
    customResolutionLayout->addWidget(m_customHeight);
    QLabel *depthLabel = new QLabel(tr("Color &Depth:"), this);
    m_depth = new QComboBox(this);
    m_depth->addItems(QStringList { tr("High Color (15 bpp)"),
                                    tr("High Color (16 bpp)"),
                                    tr("True Color (24 bpp)"),
                                    tr("True Color (32 bpp)") });
    depthLabel->setBuddy(m_depth);
    resolutionLabel->setBuddy(m_resolution);
    QGridLayout *displayGrid = new QGridLayout(displayGroup);
    displayGrid->addWidget(resolutionLabel, 0, 0);
    displayGrid->addWidget(m_resolutionType, 0, 1);
    displayGrid->addWidget(m_resolution, 1, 1);
    displayGrid->addWidget(resolutionHint, 2, 1, 1, 1, Qt::AlignCenter);
    displayGrid->addWidget(customResolution, 3, 1);
    displayGrid->addWidget(depthLabel, 4, 0);
    displayGrid->addWidget(m_depth, 4, 1);

    connect(m_resolutionType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, customResolution, resolutionHint](int type)
    {
        switch (static_cast<ResolutionType>(type)) {
        case RT_Standard:
            m_resolution->setVisible(true);
            resolutionHint->setVisible(true);
            customResolution->setVisible(false);
            break;
        case RT_Custom:
            m_resolution->setVisible(false);
            resolutionHint->setVisible(false);
            customResolution->setVisible(true);
            break;
        case RT_Fullscreen:
            m_resolution->setVisible(false);
            resolutionHint->setVisible(false);
            customResolution->setVisible(false);
            break;
        }
    });
    customResolution->setVisible(false);

    m_availableResolutions = getUsableResolutions();
    m_resolution->setMaximum(m_availableResolutions.size() - 1);
    connect(m_resolution, &QSlider::valueChanged,
            [this, resolutionHint](int value)
    {
        QSize selectedResolution = m_availableResolutions[value];
        resolutionHint->setText(QString("%1x%2").arg(selectedResolution.width())
                                                .arg(selectedResolution.height()));
    });

    QGroupBox *compressionGroup = new QGroupBox(tr("Compression"), this);
    QLabel *compressionLabel = new QLabel(tr("Network &Compression:"), this);
    m_compression = new QComboBox(this);
    m_compression->addItems(QStringList { tr("Disabled"),
                                          tr("Default (Enabled)"),
                                          tr("Level 0"),
                                          tr("Level 1"),
                                          tr("Level 2") });
    compressionLabel->setBuddy(m_compression);
    m_jpeg = new QCheckBox(tr("&JPEG Compression:"), this);
    m_jpegLevel = new QSlider(Qt::Horizontal, this);
    m_jpegLevel->setMinimum(10);
    m_jpegLevel->setMaximum(100);
    m_jpegLevel->setTickPosition(QSlider::TicksBelow);
    m_jpegLevel->setEnabled(false);
    QLabel *jpegHint = new QLabel(this);
    jpegHint->setEnabled(false);
    QGridLayout *compressionGrid = new QGridLayout(compressionGroup);
    compressionGrid->addWidget(compressionLabel, 0, 0);
    compressionGrid->addWidget(m_compression, 0, 1, 1, 2);
    compressionGrid->addWidget(m_jpeg, 1, 0);
    compressionGrid->addWidget(m_jpegLevel, 1, 1);
    compressionGrid->addWidget(jpegHint, 1, 2);

    connect(m_jpegLevel, &QSlider::valueChanged, [jpegHint](int value)
    {
        jpegHint->setText(QString("%1%").arg(value));
    });
    connect(m_jpeg, &QCheckBox::toggled, [this, jpegHint](bool checked)
    {
        m_jpegLevel->setEnabled(checked);
        jpegHint->setEnabled(checked);
    });

    QVBoxLayout *displayLayout = new QVBoxLayout(displayTab);
    displayLayout->addWidget(displayGroup);
    displayLayout->addWidget(compressionGroup);
    displayLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tabs->addTab(displayTab, tr("Display"));

    QWidget *deviceTab = new QWidget(this);
    QGroupBox *audioGroup = new QGroupBox(tr("Audio"), this);
    QLabel *audioModeLabel = new QLabel(tr("&Audio Mode:"), this);
    m_audioMode = new QComboBox(this);
    m_audioMode->addItems(QStringList { tr("Redirect to local"),
                                        tr("Play on remote"),
                                        tr("Disable audio") });
    audioModeLabel->setBuddy(m_audioMode);
    QGridLayout *audioGrid = new QGridLayout(audioGroup);
    audioGrid->addWidget(audioModeLabel, 0, 0);
    audioGrid->addWidget(m_audioMode, 0, 1);

    QGroupBox *shareGroup = new QGroupBox(tr("Share devices"), this);
    QLabel *shareLabel = new QLabel(tr("Share with remote:"), this);
    m_clipboard = new QCheckBox(tr("&Clipboard"), this);
    m_redirectDrives = new QCheckBox(tr("All &drives"), this);
    m_redirectHome = new QCheckBox(tr("&Home drive"), this);
    QLabel *devicesHint = new QLabel(tr("More device support to be added later..."), this);
    devicesHint->setWordWrap(true);
    QGridLayout *shareGrid = new QGridLayout(shareGroup);
    shareGrid->addWidget(shareLabel, 0, 0);
    shareGrid->addWidget(m_clipboard, 0, 1);
    shareGrid->addWidget(m_redirectDrives, 1, 1);
    shareGrid->addWidget(m_redirectHome, 2, 1);
    shareGrid->addWidget(devicesHint, 3, 0, 1, 2);

    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceTab);
    deviceLayout->addWidget(audioGroup);
    deviceLayout->addWidget(shareGroup);
    deviceLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tabs->addTab(deviceTab, tr("Devices"));

    QWidget *experienceTab = new QWidget(this);
    QGroupBox *performanceGroup = new QGroupBox(tr("Performance"), this);
    QLabel *presetLabel = new QLabel(tr("&Preset:"), this);
    m_performancePreset = new QComboBox(this);
    m_performancePreset->addItems(QStringList { tr("Minimum"),
                                                tr("Low-speed (<1 Mbps)"),
                                                tr("Medium (2-10 Mbps)"),
                                                tr("High-speed (LAN)"),
                                                tr("Custom") });
    presetLabel->setBuddy(m_performancePreset);
    m_wallpaper = new QCheckBox(tr("Desktop &Wallpaper"), this);
    m_fontSmoothing = new QCheckBox(tr("&Font Smoothing"), this);
    m_aero = new QCheckBox(tr("Desktop &composition (Aero)"), this);
    m_windowDrag = new QCheckBox(tr("Show window contents while &dragging"), this);
    m_menuAnims = new QCheckBox(tr("Menu &Animation"), this);
    m_themes = new QCheckBox(tr("Windows &Themes"), this);
    QGridLayout *performanceGrid = new QGridLayout(performanceGroup);
    performanceGrid->addWidget(presetLabel, 0, 0);
    performanceGrid->addWidget(m_performancePreset, 0, 1);
    performanceGrid->addWidget(m_wallpaper, 1, 1);
    performanceGrid->addWidget(m_fontSmoothing, 2, 1);
    performanceGrid->addWidget(m_aero, 3, 1);
    performanceGrid->addWidget(m_windowDrag, 4, 1);
    performanceGrid->addWidget(m_menuAnims, 5, 1);
    performanceGrid->addWidget(m_themes, 6, 1);

    connect(m_performancePreset, SIGNAL(currentIndexChanged(int)),
            this, SLOT(perfPresetChanged(int)));
    for (QCheckBox *cb : { m_wallpaper, m_fontSmoothing, m_aero, m_windowDrag,
                           m_menuAnims, m_themes }) {
        connect(cb, SIGNAL(toggled(bool)), this, SLOT(perfItemChanged(bool)));
    }

    QGroupBox *cacheGroup = new QGroupBox(tr("Caching"), this);
    m_bitmapCache = new QCheckBox(tr("&Bitmap caching"), this);
    m_offscreenCache = new QCheckBox(tr("&Offscreen bitmap caching"), this);
    m_glyphCache = new QCheckBox(tr("&Glyph caching"), this);
    QGridLayout *cacheGrid = new QGridLayout(cacheGroup);
    cacheGrid->addWidget(m_bitmapCache, 0, 1);
    cacheGrid->addWidget(m_offscreenCache, 1, 1);
    cacheGrid->addWidget(m_glyphCache, 2, 1);

    QVBoxLayout *experienceLayout = new QVBoxLayout(experienceTab);
    experienceLayout->addWidget(performanceGroup);
    experienceLayout->addWidget(cacheGroup);
    experienceLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tabs->addTab(experienceTab, tr("Experience"));

    QWidget *advancedTab = new QWidget(this);
    QGroupBox *gatewayGroup = new QGroupBox(tr("Gateway settings"), this);
    QLabel *gatewayHelp = new QLabel(tr("Leave blank if you don't require a gateway"), this);
    gatewayHelp->setWordWrap(true);
    QLabel *gateServerLabel = new QLabel(tr("Gateway Ser&ver:"), this);
    m_gateServer = new QLineEdit(this);
    gateServerLabel->setBuddy(m_gateServer);
    QLabel *gateUsernameLabel = new QLabel(tr("Gateway Us&ername:"), this);
    m_gateUsername = new QLineEdit(this);
    gateUsernameLabel->setBuddy(m_gateUsername);
    QLabel *gatePasswordLabel = new QLabel(tr("Gateway Pass&word:"), this);
    m_gatePassword = new QLineEdit(this);
    m_gatePassword->setEchoMode(QLineEdit::Password);
    gatePasswordLabel->setBuddy(m_gatePassword);
    QGridLayout *gatewayGrid = new QGridLayout(gatewayGroup);
    gatewayGrid->addWidget(gatewayHelp, 0, 0, 1, 2);
    gatewayGrid->addWidget(gateServerLabel, 1, 0);
    gatewayGrid->addWidget(m_gateServer, 1, 1);
    gatewayGrid->addWidget(gateUsernameLabel, 2, 0);
    gatewayGrid->addWidget(m_gateUsername, 2, 1);
    gatewayGrid->addWidget(gatePasswordLabel, 3, 0);
    gatewayGrid->addWidget(m_gatePassword, 3, 1);

    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->addWidget(gatewayGroup);
    advancedLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tabs->addTab(advancedTab, tr("Advanced"));

    QPushButton *connectButton = new QPushButton(tr("&Connect"), this);
    connectButton->setDefault(true);
    connect(connectButton, &QPushButton::clicked, [this](bool)
    {
        startXFreeRDP();
    });

    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QWidget *buttonBox = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBox);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttonBox);
}

void Launcher::saveConfig()
{
    QSettings settings(QStringLiteral("qfreerdp"), QStringLiteral("qfreerdp"));

    // General
    settings.setValue(QStringLiteral("CurrentServer"), m_server->currentText());
    QStringList allServers;
    allServers.append(m_server->currentText());
    for (int i = 0; i < m_server->count(); ++i) {
        QString server = m_server->itemText(i);
        if (!allServers.contains(server))
            allServers.append(server);
    }
    settings.setValue(QStringLiteral("AllServers"), allServers);
    settings.setValue(QStringLiteral("Username"), m_username->text());

    // Display
    settings.setValue(QStringLiteral("ResolutionType"), m_resolutionType->currentIndex());
    settings.setValue(QStringLiteral("StandardResolution"),
                      m_availableResolutions[m_resolution->value()]);
    settings.setValue(QStringLiteral("CustomResolution"),
                      QSize(m_customWidth->text().toInt(), m_customHeight->text().toInt()));
    settings.setValue(QStringLiteral("BitDepth"), s_depths[m_depth->currentIndex()]);

    settings.setValue(QStringLiteral("CompressionType"), m_compression->currentIndex());
    settings.setValue(QStringLiteral("Jpeg"), m_jpeg->isChecked());
    settings.setValue(QStringLiteral("JpegLevel"), m_jpegLevel->value());

    // Devices
    settings.setValue(QStringLiteral("AudioMode"), m_audioMode->currentIndex());
    settings.setValue(QStringLiteral("Clipboard"), m_clipboard->isChecked());
    settings.setValue(QStringLiteral("RedirectDrives"), m_redirectDrives->isChecked());
    settings.setValue(QStringLiteral("RedirectHome"), m_redirectHome->isChecked());

    // Experience
    settings.setValue(QStringLiteral("Wallpaper"), m_wallpaper->isChecked());
    settings.setValue(QStringLiteral("FontSmoothing"), m_fontSmoothing->isChecked());
    settings.setValue(QStringLiteral("Aero"), m_aero->isChecked());
    settings.setValue(QStringLiteral("WindowDrag"), m_windowDrag->isChecked());
    settings.setValue(QStringLiteral("MenuAnims"), m_menuAnims->isChecked());
    settings.setValue(QStringLiteral("Themes"), m_themes->isChecked());

    settings.setValue(QStringLiteral("BitmapCache"), m_bitmapCache->isChecked());
    settings.setValue(QStringLiteral("OffscreenCache"), m_offscreenCache->isChecked());
    settings.setValue(QStringLiteral("GlyphCache"), m_glyphCache->isChecked());

    // Advanced
    settings.setValue(QStringLiteral("Gateway"), m_gateServer->text());
    settings.setValue(QStringLiteral("GatewayUsername"), m_gateUsername->text());
}

void Launcher::restoreConfig()
{
    QSettings settings(QStringLiteral("qfreerdp"), QStringLiteral("qfreerdp"));

    // General
    m_server->addItems(settings.value(QStringLiteral("AllServers"), QStringList{}).toStringList());
    m_server->setCurrentText(settings.value(QStringLiteral("CurrentServer")).toString());
    m_username->setText(settings.value(QStringLiteral("Username")).toString());

    // Display
    m_resolutionType->setCurrentIndex(settings.value(QStringLiteral("ResolutionType"),
                                                     QStringLiteral("0")).toInt());
    QSize stdResolution = settings.value(QStringLiteral("StandardResolution"),
                                         m_availableResolutions.last()).toSize();
    int stdResolutionIndex = m_availableResolutions.size() - 1;
    for (int i = 0; i < m_availableResolutions.size(); ++i) {
        if (m_availableResolutions[i] == stdResolution) {
            stdResolutionIndex = i;
            break;
        }
    }
    m_resolution->setValue(stdResolutionIndex);
    QSize customResolution = settings.value(QStringLiteral("CustomResolution"),
                                            QSize(0, 0)).toSize();
    m_customWidth->setText(QString::number(customResolution.width()));
    m_customHeight->setText(QString::number(customResolution.height()));
    int bitDepth = settings.value(QStringLiteral("BitDepth"),
                                  QString::number(s_depths.last())).toInt();
    int bitDepthIndex = s_depths.size() - 1;
    for (int i = 0; i < s_depths.size(); ++i) {
        if (s_depths[i] == bitDepth) {
            bitDepthIndex = i;
            break;
        }
    }
    m_depth->setCurrentIndex(bitDepthIndex);

    m_compression->setCurrentIndex(settings.value(QStringLiteral("Compression"),
                                                  QStringLiteral("1")).toInt());
    m_jpeg->setChecked(settings.value(QStringLiteral("Jpeg"),
                                      QStringLiteral("false")).toBool());
    m_jpegLevel->setValue(settings.value(QStringLiteral("JpegLevel"),
                                         QStringLiteral("95")).toInt());

    // Devices
    m_audioMode->setCurrentIndex(settings.value(QStringLiteral("AudioMode"),
                                                QStringLiteral("0")).toInt());
    m_clipboard->setChecked(settings.value(QStringLiteral("Clipboard"),
                                           QStringLiteral("true")).toBool());
    m_redirectDrives->setChecked(settings.value(QStringLiteral("RedirectDrives"),
                                                QStringLiteral("false")).toBool());
    m_redirectHome->setChecked(settings.value(QStringLiteral("RedirectHome"),
                                              QStringLiteral("false")).toBool());

    // Experience
    m_wallpaper->setChecked(settings.value(QStringLiteral("Wallpaper"),
                                           QStringLiteral("true")).toBool());
    m_fontSmoothing->setChecked(settings.value(QStringLiteral("FontSmoothing"),
                                               QStringLiteral("true")).toBool());
    m_aero->setChecked(settings.value(QStringLiteral("Aero"),
                                      QStringLiteral("true")).toBool());
    m_windowDrag->setChecked(settings.value(QStringLiteral("WindowDrag"),
                                            QStringLiteral("true")).toBool());
    m_menuAnims->setChecked(settings.value(QStringLiteral("MenuAnims"),
                                           QStringLiteral("true")).toBool());
    m_themes->setChecked(settings.value(QStringLiteral("Themes"),
                                        QStringLiteral("true")).toBool());

    m_bitmapCache->setChecked(settings.value(QStringLiteral("BitmapCache"),
                                             QStringLiteral("true")).toBool());
    m_offscreenCache->setChecked(settings.value(QStringLiteral("OffscreenCache"),
                                                QStringLiteral("true")).toBool());
    m_glyphCache->setChecked(settings.value(QStringLiteral("GlyphCache"),
                                            QStringLiteral("true")).toBool());

    // Advanced
    m_gateServer->setText(settings.value(QStringLiteral("Gateway")).toString());
    m_gateUsername->setText(settings.value(QStringLiteral("GatewayUsername")).toString());
}

void Launcher::startXFreeRDP()
{
    QStringList params;
    if (m_server->currentText().isEmpty()) {
        QMessageBox::critical(this, tr("Missing server"), tr("Server name must not be empty."));
        return;
    }
    params.append(QStringLiteral("/v:%1").arg(m_server->currentText()));
    if (m_username->text().isEmpty()) {
        QMessageBox::critical(this, tr("Missing username"), tr("Username must not be empty."));
        return;
    }
    params.append(QStringLiteral("/u:%1").arg(m_username->text()));

    // xfreerdp will mask this out for us in the running process
    if (m_password->text().isEmpty()) {
        QMessageBox::critical(this, tr("Missing password"), tr("Password must not be empty."));
        return;
    }
    params.append(QStringLiteral("/p:%1").arg(m_password->text()));

    switch (static_cast<ResolutionType>(m_resolutionType->currentIndex())) {
    case RT_Standard:
        {
            int stdSize = m_resolution->value();
            QSize size = m_availableResolutions[stdSize];
            params.append(QStringLiteral("/size:%1x%2").arg(size.width()).arg(size.height()));
        }
        break;
    case RT_Custom:
        for (QLineEdit *le : { m_customWidth, m_customHeight }) {
            const QValidator *validator = le->validator();
            QString text = le->text();
            int cursor = le->cursorPosition();
            if (validator->validate(text, cursor) != QValidator::Acceptable) {
                QMessageBox::critical(this, tr("Invalid input"),
                                      tr("Invalid custom resolution specified"));
                return;
            }
        }
        params.append(QStringLiteral("/size:%1x%2").arg(m_customWidth->text())
                                                   .arg(m_customHeight->text()));
        break;
    case RT_Fullscreen:
        params.append(QStringLiteral("/f"));
        break;
    }

    int bppSel = m_depth->currentIndex();
    params.append(QStringLiteral("/bpp:%1").arg(s_depths[bppSel]));

    int compression = m_compression->currentIndex();
    if (compression == CT_Disabled)
        params.append(QStringLiteral("-compression"));
    else if (compression == CT_Default)
        params.append(QStringLiteral("+compression"));
    else
        params.append(QStringLiteral("/compression-level:%1").arg(compression - CT_Level));

    if (m_jpeg->isChecked()) {
        params.append(QStringLiteral("/jpeg"));
        params.append(QStringLiteral("/jpeg-quality:%1").arg(m_jpegLevel->value()));
    }

    params.append(QStringLiteral("/audio-mode:%1").arg(m_audioMode->currentIndex()));

    params.append(QStringLiteral("%1clipboard").arg(m_clipboard->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1drives").arg(m_redirectDrives->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1home-drive").arg(m_redirectHome->isChecked() ? "+" : "-"));

    params.append(QStringLiteral("%1fonts").arg(m_fontSmoothing->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1aero").arg(m_aero->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1window-drag").arg(m_windowDrag->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1menu-anims").arg(m_menuAnims->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1themes").arg(m_themes->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1wallpaper").arg(m_wallpaper->isChecked() ? "+" : "-"));

    params.append(QStringLiteral("%1bitmap-cache").arg(m_bitmapCache->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1offscreen-cache").arg(m_offscreenCache->isChecked() ? "+" : "-"));
    params.append(QStringLiteral("%1glyph-cache").arg(m_glyphCache->isChecked() ? "+" : "-"));

    if (!m_gateServer->text().isEmpty())
        params.append(QStringLiteral("/g:%1").arg(m_gateServer->text()));
    if (!m_gateUsername->text().isEmpty()) {
        params.append(QStringLiteral("/gu:%1").arg(m_gateUsername->text()));
        params.append(QStringLiteral("/gp:%1").arg(m_gatePassword->text()));
    }

    QProcess proc;
    if (!proc.startDetached(QStringLiteral("xfreerdp"), params)) {
        QMessageBox::critical(this, tr("Error starting xfreerdp"),
                              tr("Could not start xfreerdp.  Is it in your PATH?"));
        return;
    }
    saveConfig();
    close();
}

void Launcher::perfPresetChanged(int index)
{
    switch (static_cast<PerformancePreset>(index)) {
    case PP_Minimum:
        m_wallpaper->setChecked(false);
        m_fontSmoothing->setChecked(false);
        m_aero->setChecked(false);
        m_windowDrag->setChecked(false);
        m_menuAnims->setChecked(false);
        m_themes->setChecked(false);
        break;
    case PP_Low:
        m_wallpaper->setChecked(false);
        m_fontSmoothing->setChecked(false);
        m_aero->setChecked(false);
        m_windowDrag->setChecked(false);
        m_menuAnims->setChecked(false);
        m_themes->setChecked(true);
        break;
    case PP_Mid:
        m_wallpaper->setChecked(false);
        m_fontSmoothing->setChecked(false);
        m_aero->setChecked(true);
        m_windowDrag->setChecked(false);
        m_menuAnims->setChecked(false);
        m_themes->setChecked(true);
        break;
    case PP_High:
        m_wallpaper->setChecked(true);
        m_fontSmoothing->setChecked(true);
        m_aero->setChecked(true);
        m_windowDrag->setChecked(true);
        m_menuAnims->setChecked(true);
        m_themes->setChecked(true);
        break;
    default:
        /* Don't change settings */
        break;
    }
}

void Launcher::perfItemChanged(bool)
{
    uint selector = 0;
    if (m_wallpaper->isChecked())
        selector |= (1<<0);
    if (m_fontSmoothing->isChecked())
        selector |= (1<<1);
    if (m_aero->isChecked())
        selector |= (1<<2);
    if (m_windowDrag->isChecked())
        selector |= (1<<3);
    if (m_menuAnims->isChecked())
        selector |= (1<<4);
    if (m_themes->isChecked())
        selector |= (1<<5);

    // Don't cycle between here and perfPresetChanged()
    disconnect(m_performancePreset, SIGNAL(currentIndexChanged(int)),
               this, SLOT(perfPresetChanged(int)));
    switch (selector) {
    case 0:
        m_performancePreset->setCurrentIndex(PP_Minimum);
        break;
    case 0x20:
        m_performancePreset->setCurrentIndex(PP_Low);
        break;
    case 0x24:
        m_performancePreset->setCurrentIndex(PP_Mid);
        break;
    case 0x3F:
        m_performancePreset->setCurrentIndex(PP_High);
        break;
    default:
        m_performancePreset->setCurrentIndex(PP_Custom);
        break;
    }
    connect(m_performancePreset, SIGNAL(currentIndexChanged(int)),
            this, SLOT(perfPresetChanged(int)));
}
