#ifndef USQLPARSER_H
#define USQLPARSER_H
#include <QtCore>

class uSqlParser
{
	public:
		uSqlParser();
		uSqlParser(QString psStr = "");
		int parse();
	private:
		QString m_src;
};

#endif // USQLPARSER_H
