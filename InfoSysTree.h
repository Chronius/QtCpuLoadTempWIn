#ifndef INFOSYSTREE_H
#define INFOSYSTREE_H

#include <QObject>
#include <QTreeWidget>
#include <QString>
#include <QIcon>
#include <QDebug>
#include <QHeaderView>
#include <QFont>

typedef struct {
    QString     Name;
    QIcon       Icon;
    QStringList List;
} TreeNode, *pTreeNode;

class InfoSysTree: public QObject
{
    Q_OBJECT

private:

    QTreeWidget *pTreeWidget;

public:

        InfoSysTree(QTreeWidget *pTrWidget, QList<TreeNode> &TreeNodeList){

                pTreeWidget = pTrWidget;

                pTreeWidget->setColumnCount(3);
                //pTreeWidget->setColumnMWidth(1,50);
                pTreeWidget->setWordWrap(true);

                pTreeWidget->header()->setVisible(true);

                pTreeWidget->header()->setDefaultSectionSize(200);
                pTreeWidget->header()->setMinimumSectionSize(50);
                pTreeWidget->header()->setStretchLastSection(true);

                pTreeWidget->hideColumn(2);

                if(QTreeWidgetItem* header = pTreeWidget->headerItem()){

                    QFont font;
                    font.setFamily(QStringLiteral("Times New Roman"));
                    font.setPointSize(12);
                    font.setBold(true);

                    header->setFont(0, font);
                    header->setFont(1, font);
                    header->setText(0, "Конфигурация");
                    header->setText(1, "Использование");

                }
                else{

                    pTreeWidget->setHeaderLabel("My Text");

                }

                pTreeWidget->setIconSize(QSize(48,48));

                QFont font;
                font.setFamily(QStringLiteral("Times New Roman"));
                font.setPointSize(14);
                font.setBold(true);

                QFont font2;
                font2.setFamily(QStringLiteral("Times New Roman"));
                font2.setPointSize(12);

                foreach (TreeNode Node, TreeNodeList) {

                    QTreeWidgetItem *TreeItem = new QTreeWidgetItem(pTreeWidget);

                    TreeItem->setFont(0, font);
                    TreeItem->setFont(1, font);
                    TreeItem->setIcon(0, Node.Icon);
                    TreeItem->setText(0, Node.Name);

                    int i = 0;

                    foreach (QString str, Node.List) {

                        QTreeWidgetItem *TreeItemChild = new QTreeWidgetItem(TreeItem);
                        TreeItemChild->setText(0,str);
                        TreeItemChild->setFont(0, font2);
                        TreeItemChild->setFont(1, font2);

                        TreeItemChild->setText(2, QString::number(i++));

                    }

                }

                pTreeWidget->expandAll();

    }

signals:

public slots:

    bool SetNameValue(QString Node, QString Value){

        if(Node.isEmpty()){

            return false;
        }

        QTreeWidgetItemIterator it(pTreeWidget);

        while (*it){

            if ((*it)->text(0)==Node){

                (*it)->setText(1,Value);

                return true;

            }
            it++;

        }

        return false;

    }

    bool SetValue(QString Node, QStringList Values){

        bool result = false;

        if(Values.isEmpty() || Node.isEmpty()){

            return result;
        }

        QTreeWidgetItemIterator it(pTreeWidget);

        while (*it){

            if ((*it)->text(0)==Node){

                QTreeWidgetItemIterator itc(*it);

                itc++;

                while((*itc)){

                    bool ok;

                    int i = (*itc)->text(2).toInt(&ok);

                    if (ok && i < Values.size()){

                        (*itc)->setText(1,Values[i]);

                    } else {

                        break;

                    }

                    itc++;

                };

                return true;

            }

            ++it;
        }

        return result;

    }

    bool SetValue(QString Node, int Indx, QString Value){

        QTreeWidgetItemIterator it(pTreeWidget);

        while (*it){

            if ((*it)->text(0)==Node){

                QTreeWidgetItemIterator itc(*it);

                while (*itc){

                    //qDebug()<<(*itc)->text(0)

                    bool ok;

                    int i = (*itc)->text(2).toInt(&ok);

                    if ((ok)&&(i==Indx)){

                        (*itc)->setText(1,Value);

                        return true;

                    }

                    ++itc;

                }

            }

            ++it;
        }

        return false;

    }

};

#endif // INFOSYSTREE_H
