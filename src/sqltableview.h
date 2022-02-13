#include <QtGui>
#include <QtSql>

class SqlTableView : public QTableView {
    Q_OBJECT
public:
    SqlTableView(QWidget *parent = 0);
};
