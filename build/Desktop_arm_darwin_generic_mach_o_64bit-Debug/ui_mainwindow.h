/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCharts/QChartView>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QChartView *chartWidget;
    QVBoxLayout *verticalLayout;
    QLabel *realtimePriceLabel;
    QSpacerItem *verticalSpacer_3;
    QPushButton *startEndBtn;
    QSpacerItem *verticalSpacer;
    QComboBox *accuracyCombo;
    QSpacerItem *verticalSpacer_2;
    QDateTimeEdit *historyRecord;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        chartWidget = new QChartView(centralwidget);
        chartWidget->setObjectName("chartWidget");

        horizontalLayout->addWidget(chartWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        realtimePriceLabel = new QLabel(centralwidget);
        realtimePriceLabel->setObjectName("realtimePriceLabel");
        realtimePriceLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(realtimePriceLabel);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        startEndBtn = new QPushButton(centralwidget);
        startEndBtn->setObjectName("startEndBtn");

        verticalLayout->addWidget(startEndBtn);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        accuracyCombo = new QComboBox(centralwidget);
        accuracyCombo->addItem(QString());
        accuracyCombo->addItem(QString());
        accuracyCombo->addItem(QString());
        accuracyCombo->addItem(QString());
        accuracyCombo->addItem(QString());
        accuracyCombo->addItem(QString());
        accuracyCombo->setObjectName("accuracyCombo");

        verticalLayout->addWidget(accuracyCombo);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        historyRecord = new QDateTimeEdit(centralwidget);
        historyRecord->setObjectName("historyRecord");

        verticalLayout->addWidget(historyRecord);


        horizontalLayout->addLayout(verticalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 37));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        realtimePriceLabel->setText(QCoreApplication::translate("MainWindow", "CurrentPrice:", nullptr));
        startEndBtn->setText(QCoreApplication::translate("MainWindow", "start record", nullptr));
        accuracyCombo->setItemText(0, QCoreApplication::translate("MainWindow", "3s", nullptr));
        accuracyCombo->setItemText(1, QCoreApplication::translate("MainWindow", "1m", nullptr));
        accuracyCombo->setItemText(2, QCoreApplication::translate("MainWindow", "15m", nullptr));
        accuracyCombo->setItemText(3, QCoreApplication::translate("MainWindow", "30m", nullptr));
        accuracyCombo->setItemText(4, QCoreApplication::translate("MainWindow", "1h", nullptr));
        accuracyCombo->setItemText(5, QCoreApplication::translate("MainWindow", "1d", nullptr));

    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
