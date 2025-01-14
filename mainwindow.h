// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QStack>
#include <QGridLayout>
#include <cmath>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void numberPressed();
    void operatorPressed();
    void equalPressed();
    void clearPressed();
    void backspacePressed();
    void scientificOperatorPressed();
    void constantPressed();
    void memoryPressed();
    void toggleAngleMode();
    void toggleInverse();
    void shiftPressed();

private:
    void setupUI();
    void applyDarkTheme();
    void setupMemoryButtons(QGridLayout *layout);
    void setupScientificButtons(QGridLayout *layout);
    void setupConstantsButtons(QGridLayout *layout);

    QLineEdit *display;
    QLineEdit *expressionDisplay;
    QString currentNumber;
    QString storedNumber;
    QString pendingOperator;
    bool waitingForOperand;
    bool isInRadianMode;
    bool isInverseMode;
    bool isShiftMode;
    QString currentExpression;

    double memoryValue;
    QStack<double> calculationHistory;

    QPushButton *createButton(const QString &text, const QString &style = "");
    void calculateResult();
    void updateDisplay();
    double scientificCalculation(double number, const QString &op);
    double convertToRadians(double value);
    double convertToDegrees(double value);

    void memoryAdd(double value);
    void memorySubtract(double value);
    void memoryRecall();
    void memoryClear();
    void memoryStore();

    double factorial(double n);
    double nthRoot(double base, double n);
    bool isPrecise(double value, int precision = 10);
};

#endif // MAINWINDOW_H
