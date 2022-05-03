/********************************************************************
* Copyright (C) PanteR
*-------------------------------------------------------------------
*
* QPgAdmin is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* QPgAdmin is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Panther Commander; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor,
* Boston, MA 02110-1301 USA
*-------------------------------------------------------------------
* Project:		QPgAdmin
* Author:		PanteR
* Contact:		panter.dsd@gmail.com
*******************************************************************/

#ifndef SQLHIGHLIGHTER_H
#define SQLHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

#include <QTextCharFormat>

class QTextDocument;

class SQLHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SQLHighlighter(QTextDocument *parent = 0);
	//void setWordList(QStringList psWords);
	void setWord(QString psWord);

protected:
    void highlightBlock(const QString &text);

private:
	struct HighlightingRule
	{
			QRegExp pattern;
			QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	QRegExp commentStartExpression;
	QRegExp commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat multiLineCommentFormat;

};

#endif
