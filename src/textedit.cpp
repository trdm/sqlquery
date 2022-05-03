/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "textedit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QMenu>
#include <QProcess>

TextEdit::TextEdit(QWidget *parent)
: QTextEdit(parent), completer_m(0)
{
	m_sqlhighlighter = new SQLHighlighter(document());
	m_ExePath = qApp->applicationDirPath();
	m_ExePath.append("/").append("SqlFormatter.exe");
	if (!QFile::exists(m_ExePath)) {
		m_ExePath = "";
	}


}
TextEdit::~TextEdit()
{
}
void TextEdit::setCompleter(MyCompleter *psCompleter)
{
	if (completer_m)
		QObject::disconnect(completer_m, 0, this, 0);

	completer_m = psCompleter;

	if (!completer_m)
        return;
	m_selectWord = "";
	completer_m->setWidget(this);
	completer_m->setCompletionMode(QCompleter::PopupCompletion);
	completer_m->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(completer_m, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
	connect(this,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));
}
MyCompleter *TextEdit::completer() const
{
	return completer_m;
}

void TextEdit::updateList(const QStringList &words)
{
	//c->u
}

void TextEdit::insertCompletion(const QString& completion)
{
	if (completer_m->widget() != this)
        return;
    QTextCursor tc = textCursor();
	int extra = completion.length() - completer_m->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
	setTextCursor(tc);
}

void TextEdit::onSelectionChanged()
{
	QString vSelectRes = textCursor().selectedText();
	if (vSelectRes.isEmpty()) {
		return;
	} else {
		vSelectRes = vSelectRes.trimmed();
		if (vSelectRes.indexOf(" ") > 0) {
			// оптимизадница...
			return;
		}
	}
	QRegExp re("(\\w+)");
	m_sqlhighlighter->setWord("");
	if (re.exactMatch(vSelectRes)) {
		int ttt = 200;
		m_sqlhighlighter->setWord(vSelectRes);
	}
	m_sqlhighlighter->rehighlight();
	update();

}
QString TextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
void TextEdit::focusInEvent(QFocusEvent *e)
{
	if (completer_m)
		completer_m->setWidget(this);
	QTextEdit::focusInEvent(e);
}

// trdm 2022-03-17 23:08:44
void TextEdit::contextMenuEvent(QContextMenuEvent *event) {
	//http://www.prog.org.ru/topic_13418_0.html
	QString vExePath = qApp->applicationDirPath();
	QMenu *menu = createStandardContextMenu();
	QMenu *subMenu = new QMenu(this);
	QAction *vActT, *vAct_1 = new QAction(tr("To singl line"),this);
	QAction *vActFormat = new QAction(tr("Format"),this);

	QAction *vActMenu =  menu->addMenu(subMenu);
	vActMenu->setText("Until");
	subMenu->addAction(vAct_1);
	if (!m_ExePath.isEmpty()) {
		///\todo - доделать не пашет....... OK
		//subMenu->addAction(vActFormat);
	}
	 //...
	 vActT = menu->exec(event->globalPos());
	 if (vAct_1 == vActT) {
		 doSelTextToLine();
	 } else if (vActT == vActFormat) {
		 doSelTextFormat();
	 }

	 delete vAct_1;
	 delete subMenu;
	 delete menu;
}

void TextEdit::doSelTextToLine()
{
	QString vSel_2, vSel = textCursor().selectedText();
   int vOldLen = 0;
   QChar vch;
   for (int var = 0; var < vSel.length(); ++var) {
	   vch = vSel.at(var);
	   int v2a = vch.toAscii();
	   if (v2a != 63) { /*8233, 32*/
		   vSel_2.append(vch);
	   } else {
		   vSel_2.append(' ');
		   ++vOldLen;
	   }
   }
   if (vOldLen != 0) {
	   vSel_2.append("\r\n");
	   QTextCursor cur = textCursor();
	   setTextCursor(cur);
	   cur.removeSelectedText();
	   cur.insertText(vSel_2);
   }

}

void TextEdit::doSelTextFormat()
{
	if (!QFile::exists(m_ExePath)) {
		return;
	}
	QString vSel = textCursor().selectedText();
	if (vSel.isEmpty()) {
		return;
	}
	//QFile vFile(m_ExePath);
	QString vExeFile = "SqlFormatter.exe";
	QString vSqlFile = "SqlFormatter.sql";
	QString vSqlFile2 = "SqlFormatter2.sql";
	QString vSqlFile2p = " /o:SqlFormatter2.sql";
	QString vBatFile = "SqlFormatter.bat";

	QStringList vList = m_ExePath.split("/");

	vList[vList.size()-1] = vBatFile;
	QString vPathBat = vList.join("/");

	vList[vList.size()-1] = vSqlFile;
	QString vPathTxt = vList.join("/");

	vList[vList.size()-1] = vSqlFile2;
	QString vPathTxt2 = vList.join("/");

	vList.removeAt(vList.size()-1);
	QString vPathFolder = vList.join("/");
	writeToFile(vPathTxt, vSel);
	writeToFile(vPathBat, QString("SqlFormatter ").append(vSqlFile).append(vSqlFile2p),false);

	QStringList arguments;
	arguments << vSqlFile <<  vSqlFile2p;
	//SqlFormatter test*.sql /o:resultfile.sql

	QProcess myProcess; //= new QProcess(this);
	myProcess.setWorkingDirectory(vPathFolder);
	myProcess.start(vExeFile, arguments);
	myProcess.waitForFinished(/*6000*/);
//	if (!myProcess.waitForFinished())
//		return;
	QProcess::ProcessError err = myProcess.error();

	myProcess.close();

	vSel = loadFromFile(vPathTxt2);
	if (!vSel.isEmpty()) {

		QTextCursor cur = textCursor();
		setTextCursor(cur);

		//cur.removeSelectedText();
		//insertPlainText(vSel);
		//cur.insertText(vSel);

		cur.beginEditBlock();
		//cur.movePosition(QTextCursor::End);
		QStringList string_list = vSel.split('\n');

		for (int i = 0; i < string_list.size(); i++){
			cur.insertText(string_list.at(i));
			if ((i + 1) < string_list.size()){
				cur.insertBlock();
			}
		}


		cur.endEditBlock();
	}


	//qDebug() << vPathProc;

}

void TextEdit::writeToFile(QString &psPath, QString &psSrc, bool psUtf8)
{
	QFile vSqlFile(psPath);
	if (!vSqlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return;
	}
	QTextStream out(&vSqlFile);
	if (psUtf8) {
		out.setCodec("UTF-8");
	}
	out << psSrc /*<< "\n"*/;
	out.flush();
	vSqlFile.close();
}

QString TextEdit::loadFromFile(QString &psPath, bool psUtf8)
{
	QString vRetVal = "";
	QFile vSqlFile(psPath);
	if (!vSqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return vRetVal;
	}
	QTextStream in(&vSqlFile);
	if (psUtf8) {
		in.setCodec("UTF-8");
	}
	vRetVal = in.readAll();
	vSqlFile.close();
	return vRetVal;

}
void TextEdit::keyPressEvent(QKeyEvent *e)
{
	if (completer_m && completer_m->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore(); 
            return; // let the completer do default behavior
       default:
           break;
       }
    }

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	bool isShortcutQ = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Q); // CTRL+Q
	if (!completer_m || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);
	if (isShortcutQ) {
		int tt = 20;
	}
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!completer_m || (ctrlOrShift && e->text().isEmpty()))
        return;

	static QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor(); 	//qDebug() << "textUnderCursor: "+completionPrefix;

	if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
                      || eow.contains(e->text().right(1)))) {
		completer_m->popup()->hide();
        return;
    }

	if (completionPrefix != completer_m->completionPrefix()) {
		completer_m->update(completionPrefix);
		completer_m->setCompletionPrefix(completionPrefix);
		completer_m->popup()->setCurrentIndex(completer_m->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
	cr.setWidth(completer_m->popup()->sizeHintForColumn(0)+ completer_m->popup()->verticalScrollBar()->sizeHint().width());
	completer_m->complete(cr); // popup it up!
}

