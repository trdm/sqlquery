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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QStringList>
#include <QStringListModel>
#include <QString>
#include <QCompleter>
QT_BEGIN_NAMESPACE
// !!! https://www.qtcentre.org/threads/23518-How-to-change-completion-rule-of-QCompleter
//class QCompleter;
QT_END_NAMESPACE

class MyCompleter : public QCompleter
{
		Q_OBJECT

public:
		inline MyCompleter(const QStringList& words, QObject * parent) :
		QCompleter(parent), m_list(words), m_model()
	{
		setModel(&m_model);
	}

	inline void update(QString word)
	{
		// Do any filtering you like.
		// Here we just include all items that contain word.
		QStringList filtered = m_list.filter(word, caseSensitivity());
		QStringList filtered2;
		QString vWord = word, var1;
		vWord = vWord.toLower();
		foreach (QString var0, filtered) {
			var1 = var0; var1 = var1.toLower();
			if (var1 != vWord) {
				filtered2 << var0; // не нужны уже набранные слова в попупе
			}
		}
		m_model.setStringList(filtered2);
		m_word = word;
		complete();
	}
	inline void updateList(const QStringList& words)
	{
		m_list.clear();
		foreach (QString var, words) {
			m_list << var;
		}
	}

	inline QString word()
	{
		return m_word;
	}

private:
	QStringList m_list;
	QStringListModel m_model;
	QString m_word;
};
class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    ~TextEdit();

	void setCompleter(MyCompleter *c);
	MyCompleter *completer() const;
	void updateList(const QStringList& words);

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
	void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;

private:
	MyCompleter *c;
};
//! [0]

#endif // TEXTEDIT_H

