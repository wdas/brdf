/*
Copyright Disney Enterprises, Inc. All rights reserved.

This license governs use of the accompanying software. If you use the software, you
accept this license. If you do not accept the license, do not use the software.

1. Definitions
The terms "reproduce," "reproduction," "derivative works," and "distribution" have
the same meaning here as under U.S. copyright law. A "contribution" is the original
software, or any additions or changes to the software. A "contributor" is any person
that distributes its contribution under this license. "Licensed patents" are a
contributor's patent claims that read directly on its contribution.

2. Grant of Rights
(A) Copyright Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free copyright license to reproduce its contribution, prepare
derivative works of its contribution, and distribute its contribution or any derivative
works that you create.
(B) Patent Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free license under its licensed patents to make, have made,
use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the
software or derivative works of the contribution in the software.

3. Conditions and Limitations
(A) No Trademark License- This license does not grant you rights to use any
contributors' name, logo, or trademarks.
(B) If you bring a patent claim against any contributor over patents that you claim
are infringed by the software, your patent license from such contributor to the
software ends automatically.
(C) If you distribute any portion of the software, you must retain all copyright,
patent, trademark, and attribution notices that are present in the software.
(D) If you distribute any portion of the software in source code form, you may do
so only under this license by including a complete copy of this license with your
distribution. If you distribute any portion of the software in compiled or object code
form, you may only do so under a license that complies with this license.
(E) The software is licensed "as-is." You bear the risk of using it. The contributors
give no express warranties, guarantees or conditions. You may have additional
consumer rights under your local laws which this license cannot change.
To the extent permitted under your local laws, the contributors exclude the
implied warranties of merchantability, fitness for a particular purpose and non-
infringement.
*/

#include <QHBoxLayout>
#include <QSlider>
#include "FloatVarWidget.h"

FloatVarWidget::FloatVarWidget(QString name, float minVal, float maxVal, float defaultVal)
    : QGroupBox(name), minValue(minVal), maxValue(maxVal), defaultValue(defaultVal), updatingValue(false)
{
    QHBoxLayout *layout = new QHBoxLayout;

    // slider
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum( 0 );
    slider->setMaximum( 1000 );
    slider->setValue( floatToSlider(defaultVal) );
    slider->setMinimumWidth( 130 );
    slider->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    layout->addWidget( slider );

    // edit control
    edit = new FloatVarEdit;
    edit->setMinimumWidth( 20 );
    edit->setMaximumWidth( 80 );
    edit->setText( QString().setNum(defaultVal) );
    edit->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    edit->setFixedHeight( 20 );
    QFont font = edit->font();
    font.setPointSize( 9.0 );
    edit->setFont( font );
    connect(edit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
    layout->addWidget( edit );
    connect(edit, SIGNAL(setToDefault()), this, SLOT(setToDefault()));

    layout->setMargin( 0 );
    layout->setContentsMargins( 0, 2, 0, 3 );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    setLayout(layout);
}



FloatVarWidget::~FloatVarWidget()
{
    delete edit;
    delete slider;
}


void FloatVarWidget::sliderChanged(int x)
{
    if (updatingValue) return;
    updatingValue = true;

    edit->setText( QString().setNum( sliderToFloat(x) ) );
    emit( valueChanged(getValue()) );

    updatingValue = false;
}


void FloatVarWidget::textChanged()
{
    if (updatingValue) return;
    updatingValue = true;

    slider->setValue( floatToSlider( getValue() ) );
    emit( valueChanged(getValue()) );

    updatingValue = false;
}


float FloatVarWidget::sliderToFloat( int x )
{
    return minValue + (float(x) / float(1000)) * (maxValue - minValue);	
}


int FloatVarWidget::floatToSlider( float x )
{
    return int(((x - minValue) / (maxValue - minValue)) * 1000.0);
}


void FloatVarWidget::setValue( float val )
{
    edit->setText( QString().setNum( val ) );
    textChanged();
}

void FloatVarWidget::setToDefault()
{
    setValue(defaultValue);
}


float FloatVarWidget::getValue()
{
    return edit->text().toFloat();
}


void FloatVarWidget::setMinMax( float min, float max )
{
    minValue = min;
    maxValue = max;

    textChanged();
}



void FloatVarWidget::setEnabled( bool e )
{
    edit->setEnabled( e );
    slider->setEnabled( e );
}

