// Written: fmckenna

/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written: fmckenna

#include "EDP_WindSelection.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QStackedWidget>
#include <QComboBox>


#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>

#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <sectiontitle.h>

#include <StandardWindEDP.h>
#include <UserDefinedEDP.h>

EDP_WindSelection::EDP_WindSelection(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theCurrentEDP(0), theRandomVariablesContainer(theRandomVariableIW)
{
    QVBoxLayout *layout = new QVBoxLayout();

    //
    // the selection part
    //

    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText(QString("Engineering Demand Paramaters"));
    edpSelection = new QComboBox();
    edpSelection->addItem(tr("Standard Wind"));
    edpSelection->addItem(tr("User Defined"));
    edpSelection->setObjectName("EDPSelectionComboBox");

    edpSelection->setItemData(1, "A Seismic event using Seismic Hazard Analysis and Record Selection/Scaling", Qt::ToolTipRole);

    theSelectionLayout->addWidget(label);
    theSelectionLayout->addWidget(edpSelection);
    theSelectionLayout->addStretch();
    layout->addLayout(theSelectionLayout);

    //
    // create the stacked widget
    //
    theStackedWidget = new QStackedWidget();

    //
    // create the individual widgets add to stacked widget
    //

    theStandardWindEDPs = new StandardWindEDP(theRandomVariablesContainer);
    theStackedWidget->addWidget(theStandardWindEDPs);

    theUserDefinedEDPs = new UserDefinedEDP(theRandomVariablesContainer);
    theStackedWidget->addWidget(theUserDefinedEDPs);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    theCurrentEDP=theStandardWindEDPs;

    connect(edpSelection, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(edpSelectionChanged(QString)));
}

EDP_WindSelection::~EDP_WindSelection()
{

}


bool
EDP_WindSelection::outputToJSON(QJsonObject &jsonObject)
{
    theCurrentEDP->outputToJSON(jsonObject);
    return true;
}


bool
EDP_WindSelection::inputFromJSON(QJsonObject &jsonObject) {

    qDebug() << "EDP_SELECTION - " << jsonObject;

    if (theCurrentEDP != 0) {
        return theCurrentEDP->inputFromJSON(jsonObject);
    }

    return false;
}

void EDP_WindSelection::edpSelectionChanged(const QString &arg1)
{
    //
    // switch stacked widgets depending on text
    // note type output in json and name in pull down are not the same and hence the ||
    //

    if (arg1 == "Standard Earthquake") {
        theStackedWidget->setCurrentIndex(0);
        theCurrentEDP = theStandardWindEDPs;
    }

    else if(arg1 == "User Defined") {
        theStackedWidget->setCurrentIndex(1);
        theCurrentEDP = theUserDefinedEDPs;
    }

    else {
        qDebug() << "ERROR .. EDP_WindSelection selection .. type unknown: " << arg1;
    }
}

bool
EDP_WindSelection::outputAppDataToJSON(QJsonObject &jsonObject)
{
    theCurrentEDP->outputAppDataToJSON(jsonObject);
    return true;
}


bool
EDP_WindSelection::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // get name from "Application" key

    QString type;
    if (jsonObject.contains("Application")) {
        QJsonValue theName = jsonObject["Application"];
        type = theName.toString();
    } else {
        emit sendErrorMessage("EDP_WindSelection - no Application key found");
        return false;
    }


    // based on application name value set edp type
    int index = 0;
    if ((type == QString("Standard Wind EDPs")) ||
            (type == QString("StandardWindEDP"))) {
        index = 0;
    } else if ((type == QString("UserDefinedEDP")) ||
               (type == QString("User Defined EDPs"))) {
        index = 1;
    } else {
        emit sendErrorMessage("EDP_WindSelection - no valid type found");
        return false;
    }

    edpSelection->setCurrentIndex(index);

    // invoke inputAppDataFromJSON on new type

    if (theCurrentEDP != 0) {
        return theCurrentEDP->inputAppDataFromJSON(jsonObject);
    }
}

bool
EDP_WindSelection::copyFiles(QString &destDir) {

    if (theCurrentEDP != 0) {
        return  theCurrentEDP->copyFiles(destDir);
    }

    return false;
}
