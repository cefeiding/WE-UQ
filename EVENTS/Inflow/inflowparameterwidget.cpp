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

// Written: Peter Mackenzie-Helnwein, University of Washington, Seattle

#include "inflowparameterwidget.h"
#include "ui_inflowparameterwidget.h"

#include <RandomVariablesContainer.h>

#include <math.h>

#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>

#ifdef ENDLN
#undef ENDLN
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#define ENDLN endl
#else
#define ENDLN Qt::endl
#endif


#include <QDebug>

InflowParameterWidget::InflowParameterWidget(RandomVariablesContainer *theRandomVariableIW, bool isRemote, QWidget *parent)
    : SimCenterAppWidget(parent),
      ui(new Ui::InflowParameterWidget),
      isRemote(isRemote),
      theRandomVariablesContainer(theRandomVariableIW)
{
    ui->setupUi(this);
    // ui->exportGroup->hide();
    updateUIsettings();
    setDefaultParameters();

    theParameters.clear();
    hasParameters = false;

    UFileHead = "";
    UFileTail = "";
    clearBoundaryMap();

    if(isRemote)
    {
        ui->sourceLocationDisplay->hide();
        ui->sourceLocateBtn->hide();
        ui->refreshButton->hide();

        connect(ui->refreshButton, &QPushButton::clicked, this, [this]()
        {
            this->uFileUpdateRequested();
        });
    }
    else {
        ui->sourceLocationDisplay->show();
        ui->sourceLocateBtn->show();
        ui->refreshButton->show();
    }
    // fmk - removed but left in .ui file in case absolutely must have
    //ui->sourceSelectionBrowser->hide();
}

InflowParameterWidget::~InflowParameterWidget()
{
    delete ui;
}

void InflowParameterWidget::updateUIsettings(void)
{

    // tool tips
    ui->PHI11->setToolTip("R<sub>11</sub>");
    ui->PHI21->setToolTip("R<sub>21</sub>");
    ui->PHI31->setToolTip("R<sub>31</sub>");
    ui->PHI12->setToolTip("R<sub>12</sub>");
    ui->PHI22->setToolTip("R<sub>22</sub>");
    ui->PHI32->setToolTip("R<sub>32</sub>");
    ui->PHI13->setToolTip("R<sub>13</sub>");
    ui->PHI23->setToolTip("R<sub>23</sub>");
    ui->PHI33->setToolTip("R<sub>33</sub>");

    ui->L11->setToolTip("L<sup>1</sup><sub>11</sub> = <sup>x</sup>L<sub>u</sub>");
    ui->L21->setToolTip("L<sup>2</sup><sub>11</sub> = <sup>y</sup>L<sub>u</sub>");
    ui->L31->setToolTip("L<sup>3</sup><sub>11</sub> = <sup>z</sup>L<sub>u</sub>");
    ui->L12->setToolTip("L<sup>1</sup><sub>22</sub> = <sup>x</sup>L<sub>v</sub>");
    ui->L22->setToolTip("L<sup>2</sup><sub>22</sub> = <sup>y</sup>L<sub>v</sub>");
    ui->L32->setToolTip("L<sup>3</sup><sub>22</sub> = <sup>z</sup>L<sub>v</sub>");
    ui->L13->setToolTip("L<sup>1</sup><sub>33</sub> = <sup>x</sup>L<sub>w</sub>");
    ui->L23->setToolTip("L<sup>2</sup><sub>33</sub> = <sup>y</sup>L<sub>w</sub>");
    ui->L33->setToolTip("L<sup>3</sup><sub>33</sub> = <sup>z</sup>L<sub>w</sub>");

    ui->alpha11->setToolTip("&alpha;<sub>11</sub>");
    ui->alpha21->setToolTip("&alpha;<sub>21</sub>");
    ui->alpha31->setToolTip("&alpha;<sub>31</sub>");
    ui->alpha12->setToolTip("&alpha;<sub>12</sub>");
    ui->alpha22->setToolTip("&alpha;<sub>22</sub>");
    ui->alpha32->setToolTip("&alpha;<sub>32</sub>");
    ui->alpha13->setToolTip("&alpha;<sub>13</sub>");
    ui->alpha23->setToolTip("&alpha;<sub>23</sub>");
    ui->alpha33->setToolTip("&alpha;<sub>33</sub>");

    ui->alpha1->setToolTip("&alpha;<sub>1</sub>");
    ui->alpha2->setToolTip("&alpha;<sub>2</sub>");
    ui->alpha3->setToolTip("&alpha;<sub>3</sub>");
}

void InflowParameterWidget::selectSourceLocation(void)
{
    QFileDialog *dlg = new QFileDialog();
    dlg->setFileMode(QFileDialog::Directory);
    if (dlg->exec())
    {
        QDir sourceFolder = dlg->directory();
        ui->sourceLocationDisplay->setText(sourceFolder.canonicalPath());

        qDebug() << sourceFolder;
    }
    delete dlg;

}

void InflowParameterWidget::setDefaultParameters()
{
    this->on_modelSelectionCBX_currentIndexChanged(0);

    theParameters.clear();

    /* for use in inflowProperties file */

    theParameters["profile"] = 0;

    theParameters["vel0"] = 1.0;
    theParameters["refAngleU"] = 0.0;
    theParameters["refDistU"] = 1.0;
    theParameters["alphaU"] = 0.0;

    theParameters["refAnglePHI"] = 0.0;
    theParameters["refDistPHI"] = 1.0;

    theParameters["alpha0"] = 0.0;
    theParameters["alpha1"] = 0.0;
    theParameters["alpha2"] = 0.0;

    theParameters["phi00"] = 1.0;
    theParameters["phi10"] = 0.0;
    theParameters["phi20"] = 0.0;
    theParameters["phi11"] = 1.0;
    theParameters["phi21"] = 0.0;
    theParameters["phi22"] = 1.0;
    
    theParameters["refAngleL"] = 0.0;
    theParameters["refDistL"] = 1.0;
    
    theParameters["L11"] = 1.0;
    theParameters["L12"] = 1.0;
    theParameters["L13"] = 1.0;
    theParameters["L21"] = 1.0;
    theParameters["L22"] = 1.0;
    theParameters["L23"] = 1.0;
    theParameters["L31"] = 1.0;
    theParameters["L32"] = 1.0;
    theParameters["L33"] = 1.0;
    
    theParameters["alpha11"] = 0.0;
    theParameters["alpha12"] = 0.0;
    theParameters["alpha13"] = 0.0;
    theParameters["alpha21"] = 0.0;
    theParameters["alpha22"] = 0.0;
    theParameters["alpha23"] = 0.0;
    theParameters["alpha31"] = 0.0;
    theParameters["alpha32"] = 0.0;
    theParameters["alpha33"] = 0.0;

    /* for use in U file */

    theParameters["FilterMethod"] = 0;

    theParameters["filterType"] = 0;
    theParameters["gridFactor"] = 1.0;
    theParameters["filterFactor"] = 2;

    theParameters["eddyType"] = 0;
    theParameters["eddyDensity"] = 0.0;

    theParameters["intersection0"] = 0.0;
    theParameters["intersection1"] = 0.0;
    theParameters["intersection2"] = 0.0;
    
    theParameters["offset0"] = 0.0;
    theParameters["offset1"] = 0.0;
    theParameters["offset2"] = 0.0;

    hasParameters = true;

    refreshDisplay();
}

void InflowParameterWidget::refreshParameterMap(void)
{
    hasParameters = this->fetchParameterMap(theParameters);
}

bool InflowParameterWidget::fetchParameterMap(QMap<QString, double> &theParams)
{

    // collect data
    theParams.clear();

    //
    // populate theParameters map
    //

    /* for use in inflowProperties file */

    theParams.insert("profile",double(ui->modelSelectionCBX->currentIndex()));

    theParams.insert("vel0",ui->vel->value());
    theParams.insert("refAngleU",ui->refAngleU->value());
    theParams.insert("refDistU",ui->refDistU->value());
    theParams.insert("alphaU",ui->alphaU->value());

    theParams.insert("refAnglePHI",ui->refAnglePHI->value());
    theParams.insert("refDistPHI",ui->refDistPHI->value());

    theParams.insert("alpha0",ui->alpha1->value());
    theParams.insert("alpha1",ui->alpha2->value());
    theParams.insert("alpha2",ui->alpha3->value());

    theParams.insert("phi00",ui->PHI11->value());
    theParams.insert("phi10",ui->PHI21->value());
    theParams.insert("phi20",ui->PHI31->value());
    theParams.insert("phi11",ui->PHI22->value());
    theParams.insert("phi21",ui->PHI23->value());
    theParams.insert("phi22",ui->PHI33->value());
    
    theParams.insert("refAngleL",ui->refAngleL->value());
    theParams.insert("refDistL",ui->refDistL->value());

    theParams.insert("alpha11",ui->alpha11->value());
    theParams.insert("alpha12",ui->alpha12->value());
    theParams.insert("alpha13",ui->alpha13->value());
    theParams.insert("alpha21",ui->alpha21->value());
    theParams.insert("alpha22",ui->alpha22->value());
    theParams.insert("alpha23",ui->alpha23->value());
    theParams.insert("alpha31",ui->alpha31->value());
    theParams.insert("alpha32",ui->alpha32->value());
    theParams.insert("alpha33",ui->alpha33->value());

    theParams.insert("L11",ui->L11->value());
    theParams.insert("L12",ui->L12->value());
    theParams.insert("L13",ui->L13->value());
    theParams.insert("L21",ui->L21->value());
    theParams.insert("L22",ui->L22->value());
    theParams.insert("L23",ui->L23->value());
    theParams.insert("L31",ui->L31->value());
    theParams.insert("L32",ui->L32->value());
    theParams.insert("L33",ui->L33->value());

    /* for use in U file */

    // there must be four options FIX IT!

    if (ui->RB_digitalFilter->isChecked())
        { theParams.insert("FilterMethod",0); }
    else if (ui->RB_syntheticEddie->isChecked())
        { theParams.insert("FilterMethod",1); }
    else if (ui->RB_divergenceFree->isChecked())
        { theParams.insert("FilterMethod",2); }
    else if (ui->RB_turbulentSpot->isChecked())
        { theParams.insert("FilterMethod",3); }
    else
        { theParams.insert("FilterMethod",0); }

    theParams.insert("filterType",ui->filterType->currentIndex());
    theParams.insert("gridFactor",ui->gridFactor->value());
    theParams.insert("filterFactor",ui->filterFactor->value());

    theParams.insert("eddyType",ui->eddyType->currentIndex());
    theParams.insert("eddyDensity",ui->eddyDensity->value());
    theParams.insert("divergenceFreeEddyDensity",ui->divEddyDensity->value());
    theParams.insert("turbulentSpotDensity",ui->turbulentSpotDensity->value());
    
    if (ui->RB_turbulentSpotTypeL->isChecked()) {
        theParams.insert("turbulentSpotType", -1.0);
    }
    else {
        theParams.insert("turbulentSpotType", 1.0);
    }

    theParams.insert("periodicY",ui->CBx_periodicY->isChecked()?1:0);
    theParams.insert("periodicZ",ui->CBx_periodicZ->isChecked()?1:0);
    theParams.insert("cleanRestart",ui->CBx_cleanRestart->isChecked()?1:0);
    theParams.insert("interpolateParameters",ui->CBx_interpolateParameters->isChecked()?1:0);

    theParams.insert("intersection0",ui->dir1->value());
    theParams.insert("intersection1",ui->dir2->value());
    theParams.insert("intersection2",ui->dir3->value());
    
    theParams.insert("offset0",ui->offset0->value());
    theParams.insert("offset1",ui->offset1->value());
    theParams.insert("offset2",ui->offset2->value());

    return true;
}


QString InflowParameterWidget::fetchBoundarySelection(void)
{
    QString theBoundarySelection = ui->boundarySelection->currentText();
    return theBoundarySelection;
}

void InflowParameterWidget::refreshDisplay(void)
{
    /* for use in inflowProperties file */

    ui->modelSelectionCBX->setCurrentIndex(int(theParameters.value("profile")));

    ui->vel->setValue(theParameters.value("vel0"));
    ui->refAngleU->setValue(theParameters.value("refAngleU"));
    ui->refDistU->setValue(theParameters.value("refDistU"));
    ui->alphaU->setValue(theParameters.value("alphaU"));

    ui->refAnglePHI->setValue(theParameters.value("refAnglePHI"));
    ui->refDistPHI->setValue(theParameters.value("refDistPHI"));

    ui->alpha1->setValue(theParameters.value("alpha0"));
    ui->alpha2->setValue(theParameters.value("alpha1"));
    ui->alpha3->setValue(theParameters.value("alpha2"));

    ui->PHI11->setValue(theParameters.value("phi00"));
    ui->PHI21->setValue(theParameters.value("phi10"));
    ui->PHI31->setValue(theParameters.value("phi20"));
    ui->PHI22->setValue(theParameters.value("phi11"));
    ui->PHI23->setValue(theParameters.value("phi21"));
    ui->PHI33->setValue(theParameters.value("phi22"));
    
    ui->refAngleL->setValue(theParameters.value("refAngleL"));
    ui->refDistL->setValue(theParameters.value("refDistL"));

    ui->alpha11->setValue(theParameters.value("alpha11"));
    ui->alpha12->setValue(theParameters.value("alpha12"));
    ui->alpha13->setValue(theParameters.value("alpha13"));
    ui->alpha21->setValue(theParameters.value("alpha21"));
    ui->alpha22->setValue(theParameters.value("alpha22"));
    ui->alpha23->setValue(theParameters.value("alpha23"));
    ui->alpha31->setValue(theParameters.value("alpha31"));
    ui->alpha32->setValue(theParameters.value("alpha32"));
    ui->alpha33->setValue(theParameters.value("alpha33"));
    
    ui->L11->setValue(theParameters.value("L11"));
    ui->L12->setValue(theParameters.value("L12"));
    ui->L13->setValue(theParameters.value("L13"));
    ui->L21->setValue(theParameters.value("L21"));
    ui->L22->setValue(theParameters.value("L22"));
    ui->L23->setValue(theParameters.value("L23"));
    ui->L31->setValue(theParameters.value("L31"));
    ui->L32->setValue(theParameters.value("L32"));
    ui->L33->setValue(theParameters.value("L33"));

    /* for use in U file */

    ui->RB_digitalFilter->setChecked(int(theParameters.value("FilterMethod"))==0?true:false);
    ui->RB_syntheticEddie->setChecked(int(theParameters.value("FilterMethod"))==1?true:false);
    ui->RB_divergenceFree->setChecked(int(theParameters.value("FilterMethod"))==2?true:false);
    ui->RB_turbulentSpot->setChecked(int(theParameters.value("FilterMethod"))==3?true:false);

    ui->filterType->setCurrentIndex(int(theParameters.value("filterType")));
    ui->gridFactor->setValue(theParameters.value("gridFactor"));
    ui->filterFactor->setValue(int(theParameters.value("filterFactor")));

    ui->eddyType->setCurrentIndex(int(theParameters.value("eddyType")));
    ui->eddyDensity->setValue(theParameters.value("eddyDensity"));

    ui->divEddyDensity->setValue(theParameters.value("divergenceFreeEddyDensity"));
    ui->turbulentSpotDensity->setValue(theParameters.value("turbulentSpotDensity"));
    
    if (theParameters.value("turbulentSpotType") > 0.0) {
        ui->RB_turbulentSpotTypeL->setChecked(false);
        ui->RB_turbulentSpotTypeR->setChecked(true);
    }
    else {
        ui->RB_turbulentSpotTypeL->setChecked(true);
        ui->RB_turbulentSpotTypeR->setChecked(false);
    }

    ui->CBx_periodicY->setChecked(theParameters.value("periodicY") > 0.1);
    ui->CBx_periodicZ->setChecked(theParameters.value("periodicZ") > 0.1);
    ui->CBx_cleanRestart->setChecked(theParameters.value("cleanRestart") > 0.1);
    ui->CBx_interpolateParameters->setChecked(theParameters.value("interpolateParameters") > 0.1);

    ui->dir1->setValue(theParameters.value("intersection0"));
    ui->dir2->setValue(theParameters.value("intersection1"));
    ui->dir3->setValue(theParameters.value("intersection2"));
    
    ui->offset0->setValue(theParameters.value("offset0"));
    ui->offset1->setValue(theParameters.value("offset1"));
    ui->offset2->setValue(theParameters.value("offset2"));
}

void InflowParameterWidget::on_PHI21_valueChanged(double arg1)
{
    ui->PHI12->setValue(arg1);
}

void InflowParameterWidget::on_PHI31_valueChanged(double arg1)
{
    ui->PHI13->setValue(arg1);
}

void InflowParameterWidget::on_PHI32_valueChanged(double arg1)
{
    ui->PHI23->setValue(arg1);
}

void InflowParameterWidget::on_resetButton_clicked()
{
    // set UI to default parameter values
    setDefaultParameters();
}

void InflowParameterWidget::on_modelSelectionCBX_currentIndexChanged(int index)
{
    // this is where we get a mode

    switch (index) {
    case 0:
        this->setUniformTurbulent();
        break;
    case 1:
        this->setExponentialTurbulent();
        break;
    default:
        qWarning() << "Unknown boundary condition type selected" ;
    }
}

void InflowParameterWidget::setUniformTurbulent(void)
{
    ui->velocityGroup->show();
    ui->phiTensorGroup->show();
    ui->LTensorGroup->show();

    // hide extension parameters
    ui->label_refDistU->hide();
    ui->label_refAngleU->hide();
    ui->label_alphaU->hide();

    ui->refDistU->hide();
    ui->refAngleU->hide();
    ui->alphaU->hide();

    ui->label_refDistPHI->hide();
    ui->label_refAnglePHI->hide();
    ui->label_alphaPHI->hide();

    ui->refDistPHI->hide();
    ui->refAnglePHI->hide();
    ui->alpha1->hide();
    ui->alpha2->hide();
    ui->alpha3->hide();
    
    ui->label_refDistL->hide();
    ui->label_refAngleL->hide();
    ui->label_alphaL->hide();

    ui->refDistL->hide();
    ui->refAngleL->hide();
    ui->alpha11->hide();
    ui->alpha12->hide();
    ui->alpha13->hide();
    ui->alpha21->hide();
    ui->alpha22->hide();
    ui->alpha23->hide();
    ui->alpha31->hide();
    ui->alpha32->hide();
    ui->alpha33->hide();

    emit rescaleRequested();
}

void InflowParameterWidget::setExponentialTurbulent(void)
{
    ui->velocityGroup->show();
    ui->phiTensorGroup->show();
    ui->LTensorGroup->show();

    // show extension parameters
    ui->label_refDistU->show();
    ui->label_refAngleU->show();
    ui->label_alphaU->show();

    ui->refDistU->show();
    ui->refAngleU->show();
    ui->alphaU->show();

    ui->label_refDistPHI->show();
    ui->label_refAnglePHI->show();
    ui->label_alphaPHI->show();

    ui->refDistPHI->show();
    ui->refAnglePHI->show();
    ui->alpha1->show();
    ui->alpha2->show();
    ui->alpha3->show();
    
    ui->label_refDistL->show();
    ui->label_refAngleL->show();
    ui->label_alphaL->show();

    ui->refDistL->show();
    ui->refAngleL->show();
    ui->alpha11->show();
    ui->alpha12->show();
    ui->alpha13->show();
    ui->alpha21->show();
    ui->alpha22->show();
    ui->alpha23->show();
    ui->alpha31->show();
    ui->alpha32->show();
    ui->alpha33->show();

    emit rescaleRequested();
}

void InflowParameterWidget::sendParameterMap(void)
{
    this->refreshParameterMap();

    // collect data
    QMap<QString, double> data(theParameters);

    // send the parameter map
    emit parametersReady(data);
}

// --- from exportWidget

void InflowParameterWidget::clearBoundaryMap(void)
{
    foreach (QString s, boundaries.keys())
    {
        if (boundaries.value(s) != nullptr) {
            delete boundaries.value(s);
        }
        boundaries.remove(s);
    }
    //qDebug() << boundaries;
}

void InflowParameterWidget::setLocationAvailable(bool status, QDir &loc)
{
    if (status) {
        hasLocation = true;
        oldLocation = loc;
        newLocation = loc;
    }
    else {
        hasLocation = false;
        oldLocation.setPath(QDir::homePath());
        newLocation.setPath(QDir::homePath());
    }
}


void InflowParameterWidget::exportInflowParameterFile(QString fileName)
{
    hasParameters = false;

    // requests parameters to be sent
    emit sendParameterMap();

    // wait for parameters to arrive
    int i = 0;
    while (!hasParameters) { i++; }

    qDebug() << "Had to wait for " << i << "cycles";

    QString profile;

    switch (int(theParameters.value("profile")))
    {
    case 0: { profile="uniform"; break; }
    case 1: { profile="exponential"; break; }
    default: { profile="uniform"; break; }
    }

    QFile theFile(fileName);
    if (theFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&theFile);

        out << "/*--------------------------------*- C++ -*----------------------------------*\\" << ENDLN;
        out << "  =========                 |" << ENDLN;
        out << "  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox" << ENDLN;
        out << "   \\    /   O peration     | Website:  https://openfoam.org" << ENDLN;
        out << "    \\  /    A nd           | Version:  6" << ENDLN;
        out << "     \\/     M anipulation  |" << ENDLN;
        out << "\\*---------------------------------------------------------------------------*/" << ENDLN;
        out << "FoamFile" << ENDLN;
        out << "{" << ENDLN;
        out << "    version     2.0;" << ENDLN;
        out << "    format      ascii;" << ENDLN;
        out << "    class       dictionary;" << ENDLN;
        out << "    location    \"constant\";" << ENDLN;
        out << "    object      inflowProperties;" << ENDLN;
        out << "}" << ENDLN;
        out << "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //" << ENDLN;
        out << "" << ENDLN;

        /* this was moved to the inflowProperties-file starting with version 1.1.0 */

        if (theParameters.value("interpolateParameters") < 0.1)   // shall we enter parameters (y) or interpolate (n)?
        {
            out << ENDLN;
            out << "Naxis       ( "
                << theParameters.value("intersection0") << " "
                << theParameters.value("intersection1") << " "
                << theParameters.value("intersection2") << " );" << ENDLN;

            out << "offset      ( "
                << theParameters.value("offset0") << " "
                << theParameters.value("offset1") << " "
                << theParameters.value("offset2") << " );" << ENDLN;

            out << ENDLN;

            /* the above section was part of the U-file prior to version 1.1.0 */

            out << "// mean velocity" << ENDLN;
            out << ENDLN;
            out << "UDict" << ENDLN;
            out << "{" << ENDLN;
            out << "    referenceValue          " << theParameters.value("vel0") << ";" << ENDLN;

            out << "    profile                 " << profile << ";" << ENDLN;

            if ( int(theParameters.value("profile")) > 0 ) {
                out << "    referenceAngl           " << theParameters.value("refAngleU") << ";" << ENDLN;
                out << "    referenceDist           " << theParameters.value("refDistU") << ";" << ENDLN;
                out << "    alpha                   " << theParameters.value("alphaU") << ";" << ENDLN;
            }
            out << "}" << ENDLN;
            out << ENDLN;

            out << "// Reynolds stress tensor (symmTensorField)" << ENDLN;
            out << ENDLN;
            out << "RDict" << ENDLN;
            out << "{" << ENDLN;
            out << "    referenceValue          ("
                << theParameters.value("phi00") << "  "
                << theParameters.value("phi10") << "  "
                << theParameters.value("phi20") << "  "
                << theParameters.value("phi11") << "  "
                << theParameters.value("phi21") << "  "
                << theParameters.value("phi22")
                << ");" << ENDLN;

            out << "    profile                 " << profile << ";" << ENDLN;

            if ( int(theParameters.value("profile")) > 0 ) {
                out << "    referenceAngl           " << theParameters.value("refAnglePHI") << ";" << ENDLN;
                out << "    referenceDist           " << theParameters.value("refDistPHI") << ";" << ENDLN;
                out << "    alpha                     ("
                    << theParameters.value("alpha0") << "  "
                    << theParameters.value("alpha1") << "  "
                    << theParameters.value("alpha2")
                    << ");" << ENDLN;
            }

            out << "}" << ENDLN;
            out << ENDLN;

            out << "// length scale tensor (tensorField)" << ENDLN;
            out << ENDLN;
            out << "LDict" << ENDLN;
            out << "{" << ENDLN;
            out << "    referenceValue          ("
                << theParameters.value("L11") << "  "
                << theParameters.value("L12") << "  "
                << theParameters.value("L13") << "  "
                << theParameters.value("L21") << "  "
                << theParameters.value("L22") << "  "
                << theParameters.value("L23") << "  "
                << theParameters.value("L31") << "  "
                << theParameters.value("L32") << "  "
                << theParameters.value("L33")
                << ");" << ENDLN;

            out << "    profile                 " << profile << ";" << ENDLN;

            if ( int(theParameters.value("profile")) > 0 ) {
                out << "    referenceAngl           " << theParameters.value("refAngleL") << ";" << ENDLN;
                out << "    referenceDist           " << theParameters.value("refDistL") << ";" << ENDLN;
                out << "    alpha                     ("
                    << theParameters.value("alpha11") << "  "
                    << theParameters.value("alpha12") << "  "
                    << theParameters.value("alpha13") << "  "
                    << theParameters.value("alpha21") << "  "
                    << theParameters.value("alpha22") << "  "
                    << theParameters.value("alpha23") << "  "
                    << theParameters.value("alpha31") << "  "
                    << theParameters.value("alpha32") << "  "
                    << theParameters.value("alpha33")
                    << ");" << ENDLN;
            }

            out << "}" << ENDLN;
            out << ENDLN;
            out << ENDLN;
            out << "// ************************************************************************* //" << ENDLN;

        }

        out.flush();
    }

    theFile.close();
}

void InflowParameterWidget::exportUFile(QString fileName)
{
    refreshParameterMap();

    // get the boundary condition to generate
    QString BCselected = ui->boundarySelection->currentText();

    // file handle for the U file
    QFile UFile(fileName);
    UFile.open(QFile::WriteOnly);
    QTextStream out(&UFile);

    out << UFileHead;

    foreach (QString key, boundaries.keys())
    {
        out << "    " << key << ENDLN;
        out << "    {" << ENDLN;

        if (key == BCselected)
        {
            QMap<QString, QString> theMap = *boundaries.value(key);

            switch (int(theParameters.value("FilterMethod"))) {
            case 0: /* digital filter */

                out << "        type               turbulentDFMInlet;" << ENDLN;
                switch (int(theParameters.value("filterType"))) {
                case 0:
                    out << "        filterType         gaussian;" << ENDLN;
                    break;
                case 1:
                    out << "        filterType         exponential;" << ENDLN;
                    break;
                default:
                    out << "        filterType         exponential;" << ENDLN;
                }
                out << "        filterFactor       " << theParameters.value("filterFactor") << ";" << ENDLN;
                out << "        gridFactor         " << theParameters.value("gridFactor") << ";" << ENDLN;

                out << "        perodicInY         " << (( theParameters.value("periodicY") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        perodicInZ         " << (( theParameters.value("periodicZ") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        cleanRestart       " << (( theParameters.value("cleanRestart") > 0.1 ) ? "true" : "false") << ";" << ENDLN;

                break;

            case 1:  /* synthetic eddy */

                out << "        type               turbulentSEMInlet;" << ENDLN;
                switch (int(theParameters.value("eddyType"))) {
                case 0:
                    out << "        eddyType        gaussian;" << ENDLN;
                    break;
                case 1:
                    out << "        eddyType        tent;" << ENDLN;
                    break;
                case 2:
                    out << "        eddyType        step;" << ENDLN;
                    break;
                default:
                    out << "        eddyType        gaussian;" << ENDLN;
                }
                out << "        density            " << theParameters.value("eddyDensity") << ";" << ENDLN;

                out << "        perodicInY         " << (( theParameters.value("periodicY") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        perodicInZ         " << (( theParameters.value("periodicZ") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        cleanRestart       " << (( theParameters.value("cleanRestart")>0.1 ) ? "true" : "false") << ";" << ENDLN;

                break;

            case 2:  /* divergence-free synthetic eddy */

                out << "        type               turbulentDFSEMInlet;" << ENDLN;
                out << "        density            " << theParameters.value("divergenceFreeEddyDensity") << ";" << ENDLN;

                out << "        perodicInY         " << (( theParameters.value("periodicY") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        perodicInZ         " << (( theParameters.value("periodicZ") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        cleanRestart       " << (( theParameters.value("cleanRestart")>0.1 ) ? "true" : "false") << ";" << ENDLN;

                break;

            case 3:  /* digital spot */

                out << "        type               turbulentATSMInlet;" << ENDLN;

                out << "        vortonType         type" << ((theParameters.value("turbulentSpotType") > 0.0) ? "R" : "L" ) << ";" << ENDLN;
                out << "        density            " << theParameters.value("divergenceFreeEddyDensity") << ";" << ENDLN;

                out << "        perodicInY         " << (( theParameters.value("periodicY") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        perodicInZ         " << (( theParameters.value("periodicZ") > 0.1 ) ? "true" : "false") << ";" << ENDLN;
                out << "        cleanRestart       " << (( theParameters.value("cleanRestart")>0.1 ) ? "true" : "false") << ";" << ENDLN;

                break;

            default:
                qWarning() << "unknown turbulent inflow boundary conditions";
            }

            if (theParameters.value("interpolateParameters") < 0.1)   // shall we enter parameters (y) or interpolate (n)?
            {
                out << "        calculateU         true;" << ENDLN;
                out << "        calculateL         true;" << ENDLN;
                out << "        calculateR         true;" << ENDLN;
            }

            /* this was moved to the inflowProperties-file starting with version 1.1.0 *
             *

            out << "        intersection       ( "
                << theParameters.value("intersection0") << " "
                << theParameters.value("intersection1") << " "
                << theParameters.value("intersection2") << " );" << ENDLN;
            out << "        yOffset            " << theParameters.value("yOffset") << ";" << ENDLN;
            out << "        zOffset            " << theParameters.value("zOffset") << ";" << ENDLN;

             *
             */

            if (theMap.contains("type"))         theMap.remove("type");
            if (theMap.contains("filterType"))   theMap.remove("filterType");
            if (theMap.contains("filterFactor")) theMap.remove("filterFactor");
            if (theMap.contains("gridFactor"))   theMap.remove("gridFactor");
            if (theMap.contains("density"))      theMap.remove("density");
            if (theMap.contains("eddyType"))     theMap.remove("eddyType");
            if (theMap.contains("vortonType"))   theMap.remove("vortonType");
            if (theMap.contains("periodicInY"))  theMap.remove("periodicInY");
            if (theMap.contains("periodicInZ"))  theMap.remove("periodicInZ");
            if (theMap.contains("cleanRestart")) theMap.remove("cleanRestart");

            foreach (QString s, theMap.keys() )
            {
                out << "        " << s << "    " << theMap.value(s) << ";" << ENDLN;
            }
        }
        else {
            foreach (QString s, (boundaries.value(key))->keys() )
            {
                out << "        " << s << "    " << (boundaries.value(key))->value(s) << ";" << ENDLN;
            }
        }
        out << "    }" << ENDLN;
        out << ENDLN;
    }

    out << UFileTail;

    UFile.close();
}

void InflowParameterWidget::exportControlDictFile(QString origFileName, QString fileName)
{
    // file handle for the controlDict file
    QFile CDictIn(origFileName);
    CDictIn.open(QFile::ReadOnly);
    CDictContents = CDictIn.readAll();
    CDictIn.close();

    QFile CDict(fileName);
    CDict.open(QFile::WriteOnly);
    QTextStream out(&CDict);

    QList<QByteArray> CDictList = CDictContents.split('\n');
    foreach (QByteArray line, CDictList)
    {
        if (line.contains("application")) {
            out << "libs" << ENDLN;
            out << "(" << ENDLN;
            out << "    \"libturbulentInflow.so\"" << ENDLN;
            out << ");" << ENDLN;
            out << ENDLN;
        }

        out << line << ENDLN;
    }

    CDict.close();
}


void InflowParameterWidget::on_RB_digitalFilter_clicked()
{
    ui->stackedMethods->setCurrentIndex(0);
}

void InflowParameterWidget::on_RB_syntheticEddie_clicked()
{
    ui->stackedMethods->setCurrentIndex(1);
}

void InflowParameterWidget::on_RB_divergenceFree_clicked()
{
    ui->stackedMethods->setCurrentIndex(2);
}

void InflowParameterWidget::on_RB_turbulentSpot_clicked()
{
    ui->stackedMethods->setCurrentIndex(3);
}



void InflowParameterWidget::on_btn_export_clicked()
{
    // time to export :)

    // we place new file into the existing file structure
    // but we do save one version of the existing file as
    // filename.orig before writing the new one

    //
    // ... inflowProperties file
    //

    // save any existing file to .orig
    newLocation = oldLocation;
    newLocation.cd("constant");

    QString newFile = newLocation.absoluteFilePath("inflowProperties");
    QString origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // write the new file
    this->exportInflowParameterFile(newFile);

    //
    // ... U file
    //

    newLocation = oldLocation;
    newLocation.cd("0");

    newFile  = newLocation.absoluteFilePath("U");
    origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // update U file
    this->exportUFile(newFile);

    //
    // ... controlDict file
    //

    newLocation = oldLocation;
    newLocation.cd("systen");

    newFile  = newLocation.absoluteFilePath("controlDict");
    origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // update controlDict file
    this->exportControlDictFile(origFile, newFile);
}

void InflowParameterWidget::on_RemoteFilesChanged(QString uFilePath, QString controlDictPath)
{
    UFilePath = uFilePath;
    UFileHead = "";
    UFileTail = "";

    if (readUfile(uFilePath))
        processUfile();

    readControlDict(controlDictPath);
}

void InflowParameterWidget::on_boundarySelection_currentIndexChanged(int index)
{
    ui->boundarySelection->setCurrentIndex(index);
}

// --- from FileWidget

void InflowParameterWidget::on_sourceLocateBtn_clicked()
{
    UFileHead = "";
    UFileTail = "";

    // select the openFOAM input source tree
    QFileDialog *dlg = new QFileDialog();
    dlg->setReadOnly(true);
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->exec();
    QDir fileTreeLocation = dlg->directory();

    ui->sourceLocationDisplay->setText(fileTreeLocation.path());

    QStringList folders = fileTreeLocation.entryList(QStringList(),QDir::Dirs);

    if (folders.contains("0") && folders.contains("constant")  && folders.contains("system")) {
        //
        // look for U file
        //

        QDir UDir = fileTreeLocation;
        UDir.cd("0");
        UFilePath = UDir.filePath("U");

        //
        // look for controlDict file
        //

        QDir CDictDir = fileTreeLocation;
        CDictDir.cd("system");
        ControlDictPath = CDictDir.filePath("controlDict");

        validSourcePresent = readUfile(UFilePath) && readControlDict(ControlDictPath);

        if (validSourcePresent)
        { ui->sourceLocationDisplay->setStyleSheet("color: #000000;"); }
        else
        { ui->sourceLocationDisplay->setStyleSheet("color: #FFA500;"); }
    }
    else {
        //
        // this is not a valid OpenFOAM folder
        //
        UFileContents = "";

        ui->sourceLocationDisplay->setStyleSheet("color: #ff0000;");
        validSourcePresent = false;
    }

    delete dlg;

    if (validSourcePresent) {
        this->processUfile();
    }
    else {
        // user not ready to proceed
        QDir thisDir(".");
        setLocationAvailable(false, thisDir);
    }
}

bool InflowParameterWidget::readUfile(QString filename)
{
    QFile UFile(filename);

    if (UFile.exists()) {
        //
        // U file exists
        //
        UFile.open(QFile::ReadOnly);
        UFileContents = UFile.readAll();
        UFile.close();

        return true;
    }
    else {
        //
        // U file missing
        //
        UFileContents = "";

        return false;
    }
}

bool InflowParameterWidget::readControlDict(QString filename)
{
    QFile CDictFile(filename);

    if (CDictFile.exists()) {
        //
        // controlDict file exists
        //
        CDictFile.open(QFile::ReadOnly);
        CDictContents = CDictFile.readAll();
        CDictFile.close();

        return true;
    }
    else {
        //
        // controlDict file missing
        //
        CDictContents = "";

        return false;
    }
}

bool InflowParameterWidget::getLine(QStringList &reply)
{
    bool hasLine = false;
    QByteArray lineString = "";

    while (UIter->hasNext() && (!hasLine))
    {
        QByteArray line = UIter->next().simplified();
        if (qstrncmp(line,"//",2) == 0) continue;
        if (qstrncmp(line, "#",1) == 0) continue;
        if (line.contains('{')) {
            hasLine = true;
            break;
        }
        lineString += line;
        if (line.contains('}')) {
            hasLine = true;
            break;
        }
        if (line.contains(';')) {
            int idx = lineString.indexOf(';');
            lineString.truncate(idx);
            hasLine = true;
            break;
        }
    }

    reply.clear();

    if (hasLine)
    {
        QByteArrayList reply0 = lineString.simplified().split(' ');

        foreach (QByteArray item, reply0)
        {
            reply.append(item);
        }
    }

    return hasLine;
}

QMap<QString, QString> *InflowParameterWidget::readParameters(void)
{
    QMap<QString, QString> *params = new QMap<QString, QString>();

    QStringList items;

    while ( this->getLine(items) ) {
        if (items[0] == '}') break;

        if (items.length() > 0 ) {
            QString key = items[0];
            items.removeFirst();
            QString value = items.join(" ");
            params->insert(key, value);
        }
    }

    return params;
}

void InflowParameterWidget::processUfile()
{
    // parse files for available boundaries
    QStringList boundaryList;

    UFileList = UFileContents.split('\n');
    UIter = new QListIterator<QByteArray>(UFileList);

    // read till boundaryField keyword
    while (UIter->hasNext())
    {
        QByteArray line = UIter->next();
        UFileHead.append(line);
        UFileHead.append('\n');
        if (line.contains("boundaryField")) {
            while ( (!line.contains('{')) && UIter->hasNext()) {
                line = UIter->next();
                UFileHead.append(line);
                UFileHead.append('\n');
            }
            break;
        }
    }

    // parse for boundary patches
    while (UIter->hasNext())
    {
        QStringList list;

        if (this->getLine(list))
        {
            // skip empty lines
            if (list.length() == 0) continue;

            // terminate if done with boundaryFields section
            if (list[0] == '}') {
                UFileTail.append("}\n");
                break;
            }

            // read and store the boundary item
            boundaryList.append(list[0]);
            boundaries.insert(list[0], this->readParameters());
        }
    }

    // collect the remainder of the file
    while (UIter->hasNext())
    {
        QByteArray line = UIter->next();
        UFileTail.append(line);
        UFileTail.append('\n');
    }

    QStandardItemModel *theModel= new QStandardItemModel();
    foreach(QString s, boundaryList)
    {
        theModel->appendRow(new QStandardItem(s));
    }
    ui->boundarySelection->setModel(theModel);
}

/* ************************************************************
 *
 *      overloading virtual methods
 *
 * ************************************************************/

bool InflowParameterWidget::outputToJSON(QJsonObject &rvObject)
{
    refreshParameterMap();

    // just need to send the class type here.. type needed in object in case user screws up
    rvObject["type"]="CFD-Inflow";

    rvObject["EventClassification"]="Wind";

    foreach (QString key, theParameters.keys())
    {
        rvObject[key] = theParameters.value(key);
    }

    return true;
}

bool InflowParameterWidget::inputFromJSON(QJsonObject &rvObject)
{
    // initialize theParameters to reflect all properties
    refreshParameterMap();

    // update theParameters using information from the JSON file
    foreach (QString key, theParameters.keys())
    {
        if (rvObject.contains(key)) {
          QJsonValue theValue = rvObject[key];
          theParameters[key] = theValue.toDouble();
        }
        else
          return false;
    }

    // update parameter values
    refreshDisplay();

    return true;
}

bool InflowParameterWidget::outputAppDataToJSON(QJsonObject &rvObject)
{
    rvObject["EventClassification"]="Wind";
    rvObject["Application"] = "CFD Inflow";
    QJsonObject dataObj;


    rvObject["ApplicationData"] = dataObj;
    return true;

    return true;
}

bool InflowParameterWidget::inputAppDataFromJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);
    return true;
}

bool InflowParameterWidget::copyFiles(QString &dirName)
{
    qWarning() << "*** InflowParameterWidget::copyFiles(QString &dirName) should not be called by WE_UQ";

    // time to export :)

    // we place new files into the existing file structure
    // but we do save one version of the existing file as
    // filename.orig before writing the new one

    //
    // ... inflowProperties file
    //

    newLocation = QDir(dirName);
    if (!newLocation.cd("constant")) {
        newLocation.mkdir("constant");
        newLocation.cd("constant");
    }

    QString newFile = newLocation.absoluteFilePath("inflowProperties");
    QString origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // write the new file
    this->exportInflowParameterFile(newFile);

    //
    // ... U file
    //

    newLocation = QDir(dirName);
    if (!newLocation.cd("0")) {
        newLocation.mkdir("0");
        newLocation.cd("0");
    }

    newFile  = newLocation.absoluteFilePath("U");
    origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // update U file
    this->exportUFile(newFile);

    //
    // ... controlDict file
    //

    newLocation = QDir(dirName);
    if (!newLocation.cd("system")) {
        newLocation.mkdir("system");
        newLocation.cd("system");
    }

    newFile  = newLocation.absoluteFilePath("controlDict");
    origFile = newFile + ".orig";

    if (QFile(origFile).exists()) {
        qWarning() << "overwriting " << origFile;
        QFile::remove(origFile);
    }
    QFile::rename(newFile, origFile);

    qDebug() << "move" << newFile << origFile;

    // update controlDict file
    this->exportControlDictFile(origFile, newFile);

    return true;
}

/* ************************************************************
 *
 *      public slots
 *
 * ************************************************************/

void InflowParameterWidget::clear(void)
{
    setDefaultParameters();
}

void InflowParameterWidget::chooseFileName(void)
{

}
