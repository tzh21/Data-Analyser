#include "testform.h"
#include "ui_testform.h"

testform::testform(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::testform)
{
    ui->setupUi(this);
}

testform::~testform()
{
    delete ui;
}
