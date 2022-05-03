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

#include <QtGui/QTextDocument>

#include "sqlhighlighter.h"
#include <QDebug>
#include <QStringList>

//! [0]
SQLHighlighter::SQLHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
	keywordFormat.setBackground(Qt::yellow/*Qt::gray*/);
	//keywordFormat.setFontWeight(QFont::Bold);
	commentStartExpression = QRegExp("/\\*");
	commentEndExpression = QRegExp("\\*/");
	multiLineCommentFormat.setForeground(Qt::darkGray);
	multiLineCommentFormat.setForeground(Qt::darkCyan);

}

void SQLHighlighter::setWord(QString psWord)
{
	HighlightingRule rule;
	highlightingRules.clear();
	if (psWord.isEmpty()) {
		return;
	}
	QString pattern = psWord;
	pattern.append("\\b").prepend("\\b");
	rule.pattern = QRegExp(pattern);
	rule.format = keywordFormat;
	highlightingRules.append(rule);

}

void SQLHighlighter::highlightBlock(const QString &text)
{
	int pos = 0; 	

	setFormat (0, text.length(), Qt::black); 
	
	static const QRegExp commandsRegexp (
	"\\b(?:select|from|where|and|case|when|then|else|distinct|all|null|"
	"is|like|between|not|group|by|having|order|inner|outer|right|left|alter|with|isnull|cast|create|replace|function|"
	"returns|language|volatile|cost|table|view|or|"
	"join|on|using|union|exists|in|as|intersect|except|coalesce|insert|into|update|"
	"values|varchar|integer|number|logical|primary|key|add|set|asc|desc)\\b",
	Qt::CaseInsensitive);


 	pos = 0;
	while ((pos = commandsRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, commandsRegexp.matchedLength(), Qt::blue);
		pos += commandsRegexp.matchedLength();
	}

	static const QRegExp aggregationsRegexp ("\\b(?:count|min|max|cast)\\b\\s*\\([^\\)]+\\)",
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = aggregationsRegexp.indexIn (text, pos)) != -1)	{
        setFormat (pos, aggregationsRegexp.matchedLength(), Qt::darkGreen);
		pos += aggregationsRegexp.matchedLength();
	}

	static const QRegExp numbersRegexp ("[^\\w]((\\d+)(\\.)?)",
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = numbersRegexp.indexIn (text, pos)) != -1)	{
        setFormat (pos, numbersRegexp.matchedLength(), Qt::black);
		pos += numbersRegexp.matchedLength();
	}

	static const QRegExp stringsRegexp ("'[^']+'", 
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = stringsRegexp.indexIn (text, pos)) != -1)	{
        setFormat (pos, stringsRegexp.matchedLength(), Qt::darkGray);
		pos += stringsRegexp.matchedLength();
	}

	pos = 0;
//    static const QRegExp operatorRegexp ("([+|\\(|\\]|\\)|\\.|\\;|\\*]){1,1}");
//    static const QRegExp operatorRegexp ("([\\.|\\,|\\+|\\(|\\)]){1,1}");
//    static const QRegExp operatorRegexp ("([\\,|\\+|\\(|\\)]){1,1}");
	static const QRegExp operatorRegexp ("([,|\\.|\\(|\\)|\\+|\\-|\\*|\\=]){1,1}");

	while ((pos = operatorRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, operatorRegexp.matchedLength(), Qt::red);
		pos += operatorRegexp.matchedLength();
	}

    pos = 0;
    static const QRegExp commentRegexp ("^\\s*(--)");
    if ((pos = commentRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, text.length(), Qt::gray);
		//return; // мешал остальному хайлайту
    }
	pos = 0; // trdm 2022-04-22 10:53:51
	static const QRegExp commentTitle ("^\\s*(--)\\s*\\btitle\\b",	Qt::CaseInsensitive);
	if ((pos = commentTitle.indexIn (text, pos)) != -1)	{
		setFormat (pos, text.length(), Qt::darkYellow);
		//setFormat (pos, text.length(), Qt::darkMagenta);
		//setFormat (pos, text.length(), Qt::magenta);
	//return; // мешал остальному хайлайту
	}


	//qDebug() << "text: " << text;
	foreach (const HighlightingRule &rule, highlightingRules) {
		QRegExp expression(rule.pattern);
		expression.setCaseSensitivity(Qt::CaseInsensitive);
		int index = expression.indexIn(text);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = commentStartExpression.indexIn(text);

	while (startIndex >= 0) {
		int endIndex = commentEndExpression.indexIn(text, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} else {
			commentLength = endIndex - startIndex
			+ commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
	}

//	setCurrentBlockState(0);


}

