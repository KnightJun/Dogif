#include "KeySequenceEdit.h"
#include <QAction>
#include <QKeyEvent>
#include <qdebug.h>
#include <QSettings>
#include <QMessageBox>
#include <DeerShortcut.h>
#define NormalCSS "QLineEdit::focus{\n	border : 1px solid;\n	border-color: rgb(63, 153, 224);\n}"
#define ErrorCSS  "QLineEdit{\n	border : 1px solid;\n	border-color: rgb(255, 0, 0);\n	background-color: rgb(255, 187, 187);\n}"
QSettings* KeySequenceEdit::mSettings=nullptr;
void KeySequenceEdit::setOptionSetting(QSettings* setting)
{
	mSettings = setting;
}
KeySequenceEdit::KeySequenceEdit(QWidget* parent):QLineEdit(parent)
{
	this->setReadOnly(true);
	this->setAlignment(Qt::AlignCenter);
	connect(this, &KeySequenceEdit::objectNameChanged, this, &KeySequenceEdit::settingInit);
}

void KeySequenceEdit::settingInit(const QString &objectName)
{
	mkeySeq = mSettings->value(objectName).toString();
	mDeerShortcut = mSettings->findChild<DeerShortcut*>(objectName);
	Q_ASSERT(mDeerShortcut);
	if(mDeerShortcut->isValid()){
		QLineEdit::setStyleSheet(NormalCSS);
	}else{
		QLineEdit::setStyleSheet(ErrorCSS);
	}
	QLineEdit::setText(mkeySeq.toString());
	mDeerShortcut->blockSignals(true);
}
KeySequenceEdit::~KeySequenceEdit()
{
	mDeerShortcut->blockSignals(false);
}
void KeySequenceEdit::cleanSetting()
{
	mkeySeq = QKeySequence();
	mSettings->setValue(this->objectName(), mkeySeq.toString());
	mDeerShortcut->setShortcut(mkeySeq);
	QLineEdit::setText(mkeySeq.toString());
	this->setFocus();
	QLineEdit::setStyleSheet(NormalCSS);
}

void KeySequenceEdit::keyPressEvent(QKeyEvent* event)
{
	int uKey = event->key();
	Qt::Key key = static_cast<Qt::Key>(uKey);
	if (key == Qt::Key_unknown)return;
	if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt)
	{
		//QKeySequence pressKey(event->modifiers());
		//this->setText(pressKey.toString());
	}
	else
	{
		mkeySeq = QKeySequence(event->modifiers() + event->key());
		QLineEdit::setText(mkeySeq.toString());
		mSettings->setValue(this->objectName(), mkeySeq.toString());
		mDeerShortcut->setShortcut(mkeySeq);
		if(mDeerShortcut->isValid()){
			QLineEdit::setStyleSheet(NormalCSS);
		}else{
			QLineEdit::setStyleSheet(ErrorCSS);
		}
	}
}
