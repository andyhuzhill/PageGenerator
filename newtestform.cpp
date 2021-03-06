#include "newtestform.h"
#include "ui_newtestform.h"
#include <QSqlQuery>
#include <QDebug>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <ActiveQt/QAxWidget>
#include <ActiveQt/QAxObject>
#include <QDesktopServices>
#include <QDateTime>
#include <QMap>

#include "defs.h"
#include "docreadwriter.h"
#include "autonewpaper.h"

newTestForm::newTestForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::newTestForm)
{
    ui->setupUi(this);

    setWindowTitle(tr("新建一份试卷"));

    QSqlQuery query;

    query.exec("SELECT subjectName FROM subjects");

    while (query.next()) {
        ui->SubjectNameCB->addItem(query.value(0).toString());
    }

    questions.clear();
    points.clear();

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onQuestionsrefresh()));

    onQuestionsrefresh();

    QStringList difficulties;
    difficulties.append(tr("任意难度"));
    for (int var = 1; var < 11; ++var) {
        difficulties.append(QString("%1").arg(var));
    }
    ui->difficultyCB->addItems(difficulties);
    ui->difficultyCB->setCurrentIndex(0);

    connect(ui->pointsCB, SIGNAL(activated(int)), this, SLOT(questionNumRefresh()));
    connect(ui->difficultyCB, SIGNAL(activated(int)), this, SLOT(questionNumRefresh()));
    connect(ui->SubjectNameCB, SIGNAL(activated(int)), this, SLOT(questionNumRefresh()));
    connect(ui->questionTypeCB, SIGNAL(activated(int)), this, SLOT(questionNumRefresh()));
    questionNumRefresh();
}

newTestForm::~newTestForm()
{
    delete ui;
}

void newTestForm::closeEvent(QCloseEvent *)
{
    savePaper("autoSavedPaper");
    emit contentChanged();
}

void newTestForm::setPaperName(QString paperName)
{
    qDebug() << "paperName is :" << paperName;
    questions.clear();
    QSqlQuery query;
    query.exec(QString("SELECT * FROM '%1_Paper'").arg(paperName));
    while(query.next()){
        QString subjectName = query.value(1).toString();
        QString questionTypeName = query.value(2).toString();
        int id = query.value(3).toInt();
        QString Point = query.value(4).toString();
        QString Difficulty = query.value(5).toString();
        QString QuestionPath = query.value(6).toString();
        QString AnswerPath = query.value(7).toString();
        QString grade = query.value(8).toString();
        questions.append(Question(subjectName, id, questionTypeName, QuestionPath, AnswerPath, Point, Difficulty, grade.toInt()));
    }
    onQuestionsrefresh();
    ui->tabWidget->setCurrentWidget(ui->alreadyTab);
}

bool newTestForm::savePaper(QString paperName)
{
    QSqlQuery query;
    if(paperName == "autoSavedPaper"){
        qDebug() << "autoSave Paper";
        query.exec("DELETE FROM autoSavedPaper_Paper");
    }

    if(query.exec(QString("SELECT * FROM savedPaper WHERE paperName = '%1'").arg(paperName))){
        query.next();
        if(query.value(1).toString() == paperName && paperName != "autoSavedPaper"){
            if (QMessageBox::warning(this, tr("警告"),tr("已经保存了一个名为'%1'的试题方案！ 是否覆盖?").arg(paperName), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
                return false;
            } else {
                query.exec(QString("DROP TABLE '%1_Paper'").arg(paperName));
            }
        }
    }

    bool status1 = query.exec(QString("INSERT INTO savedPaper VALUES( 0, '%1')").arg(paperName));
    bool status2 = query.exec(QString("CREATE TABLE '%1_Paper'("
                                      "id integer primay key,"
                                      "subjectName,"
                                      "questionTypeName, "
                                      "prvid integer, "
                                      "point, "
                                      "difficulty,"
                                      "questionDocPath, "
                                      "answerDocPath, "
                                      "grade"
                                      ")").arg(paperName));

    bool status3 = true;

    int idx = 0;
    foreach (Question q, questions) {
        if(!query.exec(QString("INSERT INTO '%10_Paper' VALUES( %1, '%2', '%3', %4, '%5', '%6', '%7', '%8', '%9')").arg(idx).arg(q.getSubjectName()).arg(q.getQuestionType()).arg(q.getPrvid()).arg(q.getPoint()).arg(q.getDifficulty()).arg(q.getQuestionDocPath()).arg(q.getAnswerDocPath()).arg(q.getGrade()).arg(paperName))){
            status3 = false;
            break;
        }
        idx ++;
    }

    if(status1 && status2 && status3){
        QMessageBox::about(this, tr("通知"), tr("保存成功"));
        return true;
    }else{
        if (paperName != "autoSavedPaper") {
            QMessageBox::warning(this, tr("警告"), tr("保存失败!"), QMessageBox::Ok);
        }
        return false;
    }
}



bool newTestForm::checkSubjectQuestionType()
{
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    if (subjectName.isEmpty() || questionTypeName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先建立课程和题型"), QMessageBox::Ok);
        return false;
    }else{
        return true;
    }
}

QStringList newTestForm::getPoints(QString subjectName, QString questionTypeName)
{
    QStringList Points;
    QSqlQuery query;
    query.exec(QString("SELECT Point FROM '%1_%2'").arg(subjectName).arg(questionTypeName));
    Points.clear();
    while (query.next()) {
        QString point = query.value(0).toString();
        if (Points.contains(point) || point == "") {
            continue;
        }
        Points.append(point);
    }
    return Points;
}

void newTestForm::setQuestionList(QList<Question> listOfQuestion)
{
    questions.clear();

    foreach (Question quest, listOfQuestion) {
        questions.append(quest);
    }
}

int newTestForm::getTotalGradeOfQuestionType(QString questionType)
{
    int result = 0;
    foreach (Question quest , questions) {
        if (quest.getQuestionType() == questionType) {
            result += quest.getGrade();
        }
    }

    return result;
}

void newTestForm::on_exitButton_clicked()
{
    close();
}

void newTestForm::on_SubjectNameCB_currentIndexChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        return ;
    }
    QSqlQuery query;

    query.exec("SELECT subjectName FROM subjects");

    while (query.next()) {
        if (query.value(0).toString() == arg1) {
            break;
        }
    }
    ui->questionTypeCB->clear();
    if (query.value(0).toString() == arg1) {
        query.exec(QString("SELECT questionTypes FROM \"%1\"").arg(arg1));
        while (query.next()) {
            ui->questionTypeCB->addItem(query.value(0).toString());
        }
    }

    questionNumRefresh();
}

void newTestForm::on_questionTypeCB_currentIndexChanged(const QString &arg1)
{
    (void)arg1;
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    questionNumRefresh();
    points = getPoints(subjectName, questionTypeName);

    ui->pointsCB->clear();
    ui->pointsCB->addItem(tr("任意知识点"));
    ui->pointsCB->addItems(points);
    ui->pointsCB->setCurrentIndex(0);
}

void newTestForm::on_chooseThisQuestion_clicked()
{
    QSqlQuery query;
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    if (subjectName.isEmpty() || questionTypeName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先建立课程和题型！"), QMessageBox::Ok);
        return ;
    }
    int id;
    if (ui->questionNumListWidget->currentItem()) {
        id = ui->questionNumListWidget->currentItem()->text().toInt();
    }else{
        QMessageBox::warning(this, tr("警告"), tr("请先选中试题"), QMessageBox::Ok);
        return;
    }

    query.exec(QString("SELECT * FROM '%1_%2' WHERE id = %3").arg(subjectName).arg(questionTypeName).arg(id));

    query.next();
    QString QuestionPath = query.value(3).toString();
    QString AnswerPath = query.value(4).toString();
    QString Point = query.value(5).toString();
    QString Difficulty = query.value(6).toString();
    int grade = QInputDialog::getInteger(this, tr("请设置"), tr("设置该题的分数值"));
    questions.append(Question(subjectName, id, questionTypeName, QuestionPath, AnswerPath, Point, Difficulty, grade));
    QMessageBox::about(this, tr("信息"), tr("已选中本题！"));
}

void newTestForm::questionNumRefresh()
{
    QSqlQuery query;
    ui->questionNumListWidget->clear();
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    QString point = (ui->pointsCB->currentText() == tr("任意知识点")) ? "%" : ui->pointsCB->currentText() ;
    QString difficulty = (ui->difficultyCB->currentText() == tr("任意难度")) ? "%" : ui->difficultyCB->currentText();
    if (subjectName.isEmpty() || questionTypeName.isEmpty() ) {
        ui->questionTextBrowser->clear();
        ui->answerTextBrowser->clear();
        ui->difficultyEdit->clear();
        ui->pointEdit->clear();
        return ;
    }

    if(query.exec(QString("SELECT id FROM '%1_%2' WHERE Point LIKE '%3' AND Difficulty LIKE '%4'").arg(subjectName).arg(questionTypeName).arg(point).arg(difficulty))){
        while(query.next()){
            ui->questionNumListWidget->addItem(query.value(0).toString());
        }
    }
}

void newTestForm::on_generatePaperButton_clicked()
{
    if (questions.isEmpty()) {
        QMessageBox::warning(this, tr("警告"),tr("已选题目为空，请先选择题目！"),QMessageBox::Ok);
        return;
    }

    QString outFileName = QFileDialog::getSaveFileName(this, tr("请输入要保存的试卷文件的名字："),".", "Microsoft Word 97-2003(*.doc);;Microsoft Word 2007-2013(*.docx)");
    if (outFileName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"),tr("输入的试卷文件名为空！"),QMessageBox::Ok);
        return ;
    }

    word = new QAxWidget(wordAppName);
    word->setProperty("Visible", false);
    word->setProperty("DisplayAlerts", true);

    QAxObject *docs = word->querySubObject("Documents");
    if (!docs) {
        QMessageBox::warning(this, tr("警告"), tr("无法获得Documents对象!"),QMessageBox::Ok);
        return ;
    }

    QStringList items;

    paperGenerate(docs, questions, items, outFileName);
    paperGenerate(docs, questions, items, tr("%1_答案.doc").arg(outFileName.remove(".doc")));

    word->dynamicCall("Quit(boolean)", true);
    delete word;

    QDesktopServices::openUrl(QUrl::fromLocalFile(outFileName));

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString("%1_答案.doc").arg(outFileName.remove(".doc"))));

    if(QMessageBox::information(this, tr("通知"), tr("成功生成试卷, 是否保存该次组卷结果？"),QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
        QString paperName = QInputDialog::getText(this, tr("请输入"), tr("要保存的试卷名称"));
        if (paperName.isEmpty()) {
            QMessageBox::warning(this, tr("警告"), tr("没有输入要保存的试卷名称，取消保存！"), QMessageBox::Ok);
            return;
        }
        savePaper(paperName);
    }
}

void newTestForm::onQuestionsrefresh()
{
    ui->alreadySelectQuestions->clear();
    on_tabWidget_currentChanged(NULL);
}

void newTestForm::on_deleteThisQuestion_clicked()
{
    if (QMessageBox::warning(this, tr("警告"), tr("确定要删除选定试题？"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        return ;
    }
    QList<QListWidgetItem *> alreadyQuestions = ui->alreadySelectQuestions->selectedItems();

    if (alreadyQuestions.length() == 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要删除的题目！"), QMessageBox::Ok);
        return ;
    }

    foreach (QListWidgetItem *item, alreadyQuestions) {
        QStringList questionToBeRemoved = item->text().split(":");
        int i=0;
        foreach (Question ques, questions) {
            QString Id = questionToBeRemoved.at(2);
            Id.remove(tr("题"));
            int id = Id.toInt();
            if (ques.getSubjectName() == questionToBeRemoved.at(0) &&
                    ques.getQuestionType() == questionToBeRemoved.at(1) &&
                    ques.getPrvid() == id) {
                questions.removeAt(i);
            }
            i++;
        }
    }
    QMessageBox::information(this, tr("通知"), tr("删除成功！"),QMessageBox::Ok);
    on_tabWidget_currentChanged(NULL);
    ui->questionTextBrowser_2->clear();
    ui->answerTextBrowser_2->clear();
    ui->questionLabel->setText(tr("题目:"));
}


void newTestForm::on_tabWidget_currentChanged(QWidget *arg1)
{
    (void)arg1;
    ui->alreadySelectQuestions->clear();
    foreach (Question que, questions) {
        QListWidgetItem *item = new QListWidgetItem(QString("%1:%2:%3").arg(que.getSubjectName()).arg(que.getQuestionType()).arg(que.getPrvid()));
        item->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable);
        ui->alreadySelectQuestions->addItem(item);
    }
}

void newTestForm::on_clearAlreadySelection_clicked()
{
    if (QMessageBox::warning(this, tr("警告"), tr("确定要清空已选试题？"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        return ;
    }
    questions.clear();
    ui->alreadySelectQuestions->clear();
    ui->questionTextBrowser->clear();
    ui->questionTextBrowser_2->clear();
    ui->answerTextBrowser->clear();
    ui->answerTextBrowser_2->clear();
}

void newTestForm::paperGenerate(QAxObject *docs, QList<Question> question, QStringList &itemList, QString outFileName)
{
    const QString chineseDigit[] = {
        tr("零"),
        tr("一"),
        tr("二"),
        tr("三"),
        tr("四"),
        tr("五"),
        tr("六"),
        tr("七"),
        tr("八"),
        tr("九"),
        tr("十"),
        tr("十一"),
        tr("十二"),
        tr("十三"),
        tr("十四"),
        tr("十五"),
        tr("十六"),
        tr("十七"),
        tr("十八"),
        tr("十九"),
        tr("二十"),
    };

    QDir dir(".");
    if (outFileName.contains(tr("答案"))) {
        docs->dynamicCall("Add(QString)", QString("%1/answerTemplate.dot").arg(dir.absolutePath()));
    }else{
        docs->dynamicCall("Add(QString)", QString("%1/paperTemplate.dot").arg(dir.absolutePath()));
    }

    QAxObject *questionDoc = word->querySubObject("ActiveDocument");
    if(!questionDoc){
        QMessageBox::warning(this, tr("警告"), tr("无法获取当前打开文件对象!"),QMessageBox::Ok);
        return;
    }
    QAxObject *questionBookmarks = questionDoc->querySubObject("Bookmarks");
    if (!questionBookmarks) {
        QMessageBox::warning(this, tr("警告"), tr("无法获取试题模板中的书签，请先插入书签！"), QMessageBox::Ok);
        return ;
    }

    int count = questionBookmarks->property("Count").toInt();

    /* 填写试卷模板中的书签  */
    for (int i = count; i > 0; --i) {
        QAxObject *bookmark = questionBookmarks->querySubObject("Item(QVariant)", i);
        QString name= bookmark->property("Name").toString();
        int j=0;
        foreach(QString itemName , itemList){
            if (name == itemName) {
                QAxObject *curBM = questionDoc->querySubObject("Bookmarks(QString)", name);
                curBM->querySubObject("Range")->setProperty("Text", itemList.at(j+1));
                break;
            }
            j++;
        }
        if (j == itemList.length()) {
            QString text = QInputDialog::getText(this, tr("请输入"), tr("%1").arg(name));
            bookmark->querySubObject("Range")->setProperty("Text", text);
            itemList.append(name);
            itemList.append(text);
        }
    }

    int cnt=0;
    int questionNum = 1;
    do{
        Question ques = question.at(cnt);
        QString questionType = ques.getQuestionType();
        QAxObject *questionRange = questionDoc->querySubObject("Range()");
        int rangeEnd = questionRange->property("End").toInt();
        questionRange->dynamicCall("setRange(QVariant, QVariant)", rangeEnd, rangeEnd);
        questionRange->dynamicCall("InsertAfter(QString)", tr("\n%1. %2 (%3分)\n").arg(chineseDigit[questionNum]).arg(questionType).arg(getTotalGradeOfQuestionType(questionType)));
        questionNum ++;

        QList<int> quesToBeRemoved;
        int xiaotihao=1;
        /* 插入各个小题  */
        for (int i = 0; i < question.length(); ++i) {
            Question ques = question.at(i);
            if (questionType == ques.getQuestionType()) {
                questionDoc->dynamicCall("Select()");
                rangeEnd = questionDoc->querySubObject("Range()")->property("End").toInt();
                questionRange->dynamicCall("setRange(QVariant, QVariant)", rangeEnd, rangeEnd);

                questionRange->dynamicCall("InsertAfter(QString)", tr("%1.  (%2分)").arg(xiaotihao).arg(ques.getGrade()));

                questionDoc->dynamicCall("Select()");
                rangeEnd = questionDoc->querySubObject("Range()")->property("End").toInt();
                questionRange->dynamicCall("setRange(QVariant, QVariant)", rangeEnd, rangeEnd);
                if (outFileName.contains(tr("答案"))) {
                    questionRange->dynamicCall("InsertFile(QString)", ques.getAnswerDocPath());
                }else{
                    questionRange->dynamicCall("InsertFile(QString)", ques.getQuestionDocPath());
                }
                questionDoc->dynamicCall("Select()");
                rangeEnd = questionDoc->querySubObject("Range()")->property("End").toInt();
                questionRange->dynamicCall("setRange(QVariant, QVariant)", rangeEnd, rangeEnd);
                questionRange->dynamicCall("InsertAfter(QString)", "\n");
                quesToBeRemoved.append(i);
                xiaotihao ++;
            }
        }
        for (int i = (quesToBeRemoved.length()>0?quesToBeRemoved.length()-1:-1); i >= 0; --i) {
            question.removeAt(quesToBeRemoved.at(i));
        }
    }while(!question.isEmpty());

    questionDoc->dynamicCall("SaveAs(QString&)", outFileName);
    questionDoc->dynamicCall("Close()");
}

void newTestForm::on_alreadySelectQuestions_itemClicked(QListWidgetItem *item)
{
    if(ui->alreadySelectQuestions->currentItem()){
        ui->questionLabel->setText(tr("题目：(%1)").arg(item->text()));
        QString subjectName = item->text().split(":").at(0);
        QString questionTypeName = item->text().split(":").at(1);
        int id = item->text().split(":").at(2).toInt();
        QSqlQuery query;

        query.exec(QString("SELECT * FROM '%1_%2' WHERE id = %3").arg(subjectName).arg(questionTypeName).arg(id));

        query.next();
        QString Question = query.value(1).toString();
        QString Answer = query.value(2).toString();

        ui->answerTextBrowser_2->clear();
        ui->answerTextBrowser_2->insertHtml(Answer);
        ui->questionTextBrowser_2->clear();
        ui->questionTextBrowser_2->insertHtml(Question);
    }
}

void newTestForm::on_questionNumListWidget_itemClicked(QListWidgetItem *item)
{
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    if (subjectName.isEmpty() || questionTypeName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先建立课程和题型"), QMessageBox::Ok);
        ui->questionTextBrowser->clear();
        ui->questionTextBrowser_2->clear();
        ui->answerTextBrowser->clear();
        ui->answerTextBrowser_2->clear();
        ui->pointEdit->clear();
        ui->difficultyEdit->clear();
        return ;
    }

    QSqlQuery query;

    query.exec(QString("SELECT * FROM '%1_%2' WHERE id = %3").arg(subjectName).arg(questionTypeName).arg(item->text().toInt()));

    query.next();
    QString Question = query.value(1).toString();
    QString Answer = query.value(2).toString();
    QString Point = query.value(5).toString();
    QString Difficulty = query.value(6).toString();

    ui->questionTextBrowser->clear();
    ui->questionTextBrowser->insertHtml(Question);
    ui->answerTextBrowser->clear();
    ui->answerTextBrowser->insertHtml(Answer);
    ui->pointEdit->setPlainText(Point);
    ui->difficultyEdit->setText(Difficulty);
}

void newTestForm::on_questionNumListWidget_currentRowChanged(int currentRow)
{
    (void)currentRow;
    QString subjectName = ui->SubjectNameCB->currentText();
    QString questionTypeName = ui->questionTypeCB->currentText();
    if (!ui->questionNumListWidget->currentItem()) {
        return ;
    }
    int curow = ui->questionNumListWidget->currentItem()->text().toInt();

    if (subjectName.isEmpty() || questionTypeName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先新建课程与题目类型！"), QMessageBox::Ok);
        ui->questionTextBrowser->clear();
        ui->answerTextBrowser->clear();
        ui->difficultyEdit->clear();
        ui->pointEdit->clear();
        return ;
    }

    QSqlQuery query;

    query.exec(QString("SELECT * FROM '%1_%2' WHERE id = %3").arg(subjectName).arg(questionTypeName).arg(curow));

    query.next();
    QString Question = query.value(1).toString();
    QString Answer = query.value(2).toString();
    QString Point = query.value(5).toString();
    QString Difficulty = query.value(6).toString();

    ui->questionTextBrowser->clear();
    ui->questionTextBrowser->insertHtml(Question);
    ui->answerTextBrowser->clear();
    ui->answerTextBrowser->insertHtml(Answer);
    ui->pointEdit->setPlainText(Point);
    ui->difficultyEdit->setText(Difficulty);
}

void newTestForm::on_saveQuestionButton_clicked()
{
    if (questions.length() == 0) {
        QMessageBox::warning(this, tr("警告"), tr("已选试题为空，请先选择试题！"), QMessageBox::Ok);
        return;
    }

    QString paperName = QInputDialog::getText(this, tr("请输入"), tr("要保存的试卷名称"));
    if (paperName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("没有输入要保存的试卷名称，取消保存！"), QMessageBox::Ok);
        return;
    }

    savePaper(paperName);
}

void newTestForm::on_autoChooseButton_clicked()
{
    AutoNewPaper *paper = new AutoNewPaper(this);
    paper->exec();
}
