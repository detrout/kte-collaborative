/* This file is part of the Kobby plugin
 * Copyright (C) 2013 Sven Brauch <svenbrauch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kcm_kte_collaborative.h"
#include <kobbyplugin.h>

#include <KDebug>
#include <QHBoxLayout>
#include <QLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>

KCMKTECollaborative::KCMKTECollaborative(QWidget* parent, const QVariantList& args)
    : KCModule(KobbyPluginFactory::componentData(), parent, args)
{
    kDebug() << "creating kte_collaborative kcmodule";
    // Set up config groups
    KConfig* config = new KConfig("ktecollaborative");
    m_colorsGroup = config->group("colors");
    m_notifyGroup = config->group("notifications");

    // Create config group boxes
    QGroupBox* notificationsGroupBox = new QGroupBox();
    notificationsGroupBox->setTitle(i18n("Highlights and Notifications"));
    QFormLayout* notificationsLayout = new QFormLayout();
    notificationsGroupBox->setLayout(notificationsLayout);
    m_highlightBackground = new QCheckBox();
    m_displayWidgets = new QCheckBox();
    notificationsLayout->addRow(i18n("Display popup widgets"), m_displayWidgets);
    notificationsLayout->addRow(i18n("Colorize text background"), m_highlightBackground);

    QGroupBox* colorsGroupBox = new QGroupBox();
    colorsGroupBox->setTitle(i18n("Colours"));
    QFormLayout* colorsLayout = new QFormLayout();
    colorsGroupBox->setLayout(colorsLayout);
    m_saturationSilder = new QSlider(Qt::Horizontal);
    m_saturationSilder->setRange(30, 255);
    colorsLayout->addRow(i18n("Highlight saturation"), m_saturationSilder);

    // Assemble the UI
    setLayout(new QVBoxLayout());
    layout()->addWidget(new QLabel(i18n("Some changes might only be applied for newly opened documents.")));
    layout()->addWidget(notificationsGroupBox);
    layout()->addWidget(colorsGroupBox);
    // Add a spacer to top-align the widgets
    layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

    // Set up connections for changed signals
    connect(m_saturationSilder, SIGNAL(sliderMoved(int)), SLOT(changed()));
    connect(m_highlightBackground, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(m_displayWidgets, SIGNAL(toggled(bool)), SLOT(changed()));
}

KCMKTECollaborative::~KCMKTECollaborative()
{
    delete m_colorsGroup.config();
}

void KCMKTECollaborative::load()
{
    m_saturationSilder->setValue(m_colorsGroup.readEntry("saturation", 185));
    m_highlightBackground->setChecked(m_notifyGroup.readEntry("highlightBackground", true));
    m_displayWidgets->setChecked(m_notifyGroup.readEntry("displayWidgets", true));
}

void KCMKTECollaborative::save()
{
    m_colorsGroup.writeEntry("saturation", m_saturationSilder->value());
    m_notifyGroup.writeEntry("highlightBackground", m_highlightBackground->isChecked());
    m_notifyGroup.writeEntry("displayWidgets", m_displayWidgets->isChecked());
}
