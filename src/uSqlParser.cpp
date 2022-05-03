#include "uSqlParser.h"

uSqlParser::uSqlParser() :m_src("")
{
}

uSqlParser::uSqlParser(QString psStr) :m_src(psStr)
{

}
/*
  void MainWin::onActionFormat()
  -> hsql::SQLParser::parse(vQuery.toStdString(), &result);

E:\Projects\__My\_tools\sqlquery\add\SQLParser\parser\flex_lexer.cpp
E:\Projects\1C\Dots_2_2\src\lang1c.tab.cpp
*/
int uSqlParser::parse()
{
	return 0;
}
