#ifndef MANAGEUSERFORM_H
#define MANAGEUSERFORM_H

#include <QWidget>

#include <QList>
#include <QListWidgetItem>

class QSettings;

struct Login{
    QString name;
    QString passwd;
};

const QString OrgName = "www.csust.edu.cn";
const QString AppName = "TestPaperGenerator";

namespace Ui {
class manageUserForm;
}

class manageUserForm : public QWidget
{
    Q_OBJECT

public:
    explicit manageUserForm(QWidget *parent = 0);
    ~manageUserForm();

private slots:
    void on_exitButton_clicked();

    void on_addUserButton_clicked();


    void on_UserLists_itemClicked(QListWidgetItem *item);

    void on_deleteUserButton_clicked();

private:
    Ui::manageUserForm *ui;

    QSettings *settings;
    QList<Login> Logins;

};

#endif // MANAGEUSERFORM_H