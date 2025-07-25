/*

Pencil2D - Traditional Animation Software
Copyright (C) 2020 Jakob Gahde

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include <cmath>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <array>

#include "editor.h"
#include "elidedlabel.h"
#include "layermanager.h"
#include "scribblearea.h"
#include "toolmanager.h"
#include "viewmanager.h"

#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent)
{
    setContentsMargins(3, 0, 3, 0);

    mToolIcon = new QLabel(this);
    addWidget(mToolIcon);
    mToolLabel = new ElidedLabel(this);
    mToolLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addWidget(mToolLabel, 1);

    mModifiedLabel = new QLabel(this);
    mModifiedLabel->setPixmap(QPixmap(":/icons/themes/playful/menubar/save.svg"));
    updateModifiedStatus(false);
    addPermanentWidget(mModifiedLabel);

    QLocale locale;
    mZoomBox = new QComboBox(this);
    const int originalSizes = 13;
    std::array<double, originalSizes> zoomList = {
      10000.,
      6400.,
      1600.,
      800.,
      400.,
      200.,
      100.,
      75.,
      50.,
      33.,
      25.,
      12.,
      1.
    };
    mZoomBox->addItems(QStringList()
                           << locale.toString(zoomList[0], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[1], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[2], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[3], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[4], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[5], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[6], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[7], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[8], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[9], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[10], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[11], 'f', 1) + locale.percent()
                           << locale.toString(zoomList[12], 'f', 1) + locale.percent());
    mZoomBox->setMaxCount(mZoomBox->count() + 1);
    mZoomBox->setEditable(true);
    mZoomBox->lineEdit()->setAlignment(Qt::AlignRight);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    connect(mZoomBox, &QComboBox::textActivated, [=](const QString &currentText)
#else
    connect(mZoomBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated), [=](const QString &currentText)
#endif
    {
        if (mZoomBox->count() == mZoomBox->maxCount())
        {
            // Keep the size of the list reasonable by preventing user entries
            // insertPolicy is unsuitable as it prevents entering custom values at all
            mZoomBox->removeItem(mZoomBox->maxCount() - 1);
        }

        double currentDouble = locale.toDouble(QString(currentText).remove(locale.percent()));

        // Accurate text search
        int includedIn = mZoomBox->findText(locale.toString(currentDouble, 'f', 1) + locale.percent());
        // qDebug() << "number inserted is in place" << includedIn;

        // Needed for the removal of custom values when non-custom is inserted
        // Read the TODO below
        [[maybe_unused]]bool isInOriginal = false;

        for (int index{0}; index < originalSizes; index++)
        {
            if (currentDouble == zoomList[index])
            {
                isInOriginal = true;
                // qDebug() << "the number inserted is in the original";
            }
        }

        // Insert custom number into the ZoomBox
        if (includedIn == -1 && mZoomBox->maxCount() == originalSizes + 1)
        {
            mZoomBox->setMaxCount(originalSizes + 2);
            for(int index{0}; index < mZoomBox->count(); index++)
            {
                double compTo = locale.toDouble(QString(mZoomBox->itemText(index)).remove(locale.percent()));
                if (currentDouble > compTo)
                {
                  mZoomBox->insertItem(index,locale.toString(currentDouble, 'f', 1) + locale.percent());
                  break;
                }
            }
        }
        // Replace the existing custom value if a new one is inserted
        else if(includedIn == -1)
        {
            // Remove the old value
            int newMemberPlace = -1;
            for (int outOfPlace{0}; outOfPlace < originalSizes; outOfPlace++)
            {
                double compare = locale.toDouble(QString(mZoomBox->itemText(outOfPlace)).remove(locale.percent()));
                if (compare != zoomList[outOfPlace])
                {
                    newMemberPlace = outOfPlace;
                    // qDebug() << "this index of mZoomBox:" << newMemberPlace << "was not in the original:" << compare;
                    break;
                }
            }
            if (newMemberPlace == -1)
            {
              mZoomBox->removeItem(mZoomBox->maxCount() - 1);
            }
            else
            {
              mZoomBox->removeItem(newMemberPlace);
            }

            // Find the correct place and insert the new one
            for(int index{0}; index < mZoomBox->count(); index++)
            {
                double compTo = locale.toDouble(QString(mZoomBox->itemText(index)).remove(locale.percent()));
                if (currentDouble > compTo)
                {
                  mZoomBox->insertItem(index,locale.toString(currentDouble, 'f', 1) + locale.percent());
                  break;
                }
            }

        }
        // TODO: Figure out how to differentiate between value change and manual
        // keyboard input

        // else if(includedIn >= 0 && isInOriginal){
        //     int newMemberPlace = -1;
        //     for (int outOfPlace{0}; outOfPlace < originalSizes; outOfPlace++){
        //         double compare = locale.toDouble(QString(mZoomBox->itemText(outOfPlace)).remove(locale.percent()));
        //         if (compare != zoomList[outOfPlace]){
        //             newMemberPlace = outOfPlace;
        //             qDebug() << "this index of mZoomBox:" << newMemberPlace << "was not in the original:" << compare;
        //             break;
        //         }
        //     }
        //     if (newMemberPlace == -1){
        //       mZoomBox->removeItem(mZoomBox->maxCount() - 1);
        //     }
        //     else{
        //       mZoomBox->removeItem(newMemberPlace);
        //     }
        //     mZoomBox->setMaxCount(originalSizes + 1);
        // }

        emit zoomChanged(locale.toDouble(QString(currentText).remove(locale.percent())) / 100);
    });
    addPermanentWidget(mZoomBox);

    mZoomSlider = new QSlider(Qt::Horizontal, this);
    mZoomSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mZoomSlider->setRange(-20, 20);
    mZoomSlider->setTickPosition(QSlider::TicksBelow);
    mZoomSlider->setTickInterval(20);
    connect(mZoomSlider, &QSlider::valueChanged, [this](int value)
    {
        emit zoomChanged(std::pow(10, value / 10.));
    });
    addPermanentWidget(mZoomSlider);
}

void StatusBar::updateToolStatus(ToolType tool)
{
    Q_ASSERT(mEditor);
    switch (tool) {
        case PENCIL:
            mToolLabel->setText(tr("Click to draw. Hold Ctrl and Shift to erase or Alt to select a color from the canvas."));
            break;
        case ERASER:
            mToolLabel->setText(tr("Click to erase."));
            break;
        case SELECT:
            mToolLabel->setText(tr("Click and drag to create or modify a selection. Hold Alt to modify its contents or press Backspace to clear them."));
            break;
        case MOVE:
            mToolLabel->setText(tr("Click and drag to move an object. Hold Ctrl to rotate."));
            break;
        case CAMERA:
            mToolLabel->setText(tr("Click and drag to move the camera. While on in-between frames, drag handle to change interpolation."));
            break;
        case HAND:
            mToolLabel->setText(tr("Click and drag to pan. Hold Ctrl to zoom or Alt to rotate."));
            break;
        case SMUDGE:
            mToolLabel->setText(tr("Click to liquefy pixels or modify a vector line. Hold Alt to smooth."));
            break;
        case PEN:
            mToolLabel->setText(tr("Click to draw. Hold Ctrl and Shift to erase or Alt to select a color from the canvas."));
            break;
        case POLYLINE:
            if (mEditor->tools()->getTool(tool)->isActive())
            {
                mToolLabel->setText(tr("Click to continue the polyline. Double-click or press enter to complete the line or press Escape to discard it."));
            }
            else
            {
                mToolLabel->setText(tr("Click to create a new polyline. Hold Ctrl and Shift to erase."));
            }
            break;
        case BUCKET:
            mToolLabel->setText(tr("Click to fill an area with the current color. Hold Alt to select a color from the canvas."));
            break;
        case EYEDROPPER:
            mToolLabel->setText(tr("Click to select a color from the canvas."));
            break;
        case BRUSH:
            mToolLabel->setText(tr("Click to paint. Hold Ctrl and Shift to erase or Alt to select a color from the canvas."));
            break;
        default:
            Q_ASSERT(false);
    }

    static QPixmap toolIcons[TOOL_TYPE_COUNT]{
        {":icons/themes/playful/tools/tool-pencil.svg"},
        {":icons/themes/playful/tools/tool-eraser.svg"},
        {":icons/themes/playful/tools/tool-select.svg"},
        {":icons/themes/playful/tools/tool-move.svg"},
        {":icons/themes/playful/tools/tool-hand.svg"},
        {":icons/themes/playful/tools/tool-smudge.svg"},
        {""}, // Camera tool does not have an icon
        {":icons/themes/playful/tools/tool-pen.svg"},
        {":icons/themes/playful/tools/tool-polyline.svg"},
        {":icons/themes/playful/tools/tool-bucket.svg"},
        {":icons/themes/playful/tools/tool-eyedropper.svg"},
        {":icons/themes/playful/tools/tool-brush.svg"}
    };
    mToolIcon->setPixmap(toolIcons[tool]);
    mToolIcon->setToolTip(BaseTool::TypeName(tool));
}

void StatusBar::updateModifiedStatus(bool modified)
{
    mModifiedLabel->setDisabled(!modified);
    if (modified)
    {
        mModifiedLabel->setToolTip(tr("This file has unsaved changes"));
    }
    else
    {
        mModifiedLabel->setToolTip(tr("This file has no unsaved changes"));
    }
}

void StatusBar::updateZoomStatus()
{
    Q_ASSERT(mEditor);

    QLocale locale;
    QSignalBlocker b1(mZoomBox);
    mZoomBox->setCurrentText(locale.toString(mEditor->view()->scaling() * 100, 'f', 1) + locale.percent());

    QSignalBlocker b2(mZoomSlider);
    mZoomSlider->setValue(static_cast<int>(std::round(std::log10(mEditor->view()->scaling()) * 10)));
}
