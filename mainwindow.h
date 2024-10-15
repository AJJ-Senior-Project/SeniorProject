#ifndef mainpage_H
#define mainpage_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class mainpage;
}
QT_END_NAMESPACE

class mainpage : public QWidget
{
    Q_OBJECT

public:
    mainpage(QWidget *parent = nullptr);
    ~mainpage();

private:
    Ui::mainpage *ui;
};
#endif // mainpage_H
