#include "mainwindow.h"
#include <QGridLayout>
#include <QWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    waitingForOperand(true),
    isInRadianMode(true),
    isInverseMode(false),
    isShiftMode(false),
    memoryValue(0.0)
{
    setupUI();
    applyDarkTheme();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    QHBoxLayout *modeLayout = new QHBoxLayout();
    QPushButton *angleModeBtn = createButton(isInRadianMode ? "RAD" : "DEG", "background-color: #333333;");
    QPushButton *inverseBtn = createButton("INV", "background-color: #333333;");
    QPushButton *shiftBtn = createButton("SHIFT", "background-color: #333333;");

    connect(angleModeBtn, &QPushButton::clicked, this, &MainWindow::toggleAngleMode);
    connect(inverseBtn, &QPushButton::clicked, this, &MainWindow::toggleInverse);
    connect(shiftBtn, &QPushButton::clicked, this, &MainWindow::shiftPressed);

    modeLayout->addWidget(angleModeBtn);
    modeLayout->addWidget(inverseBtn);
    modeLayout->addWidget(shiftBtn);
    mainLayout->addLayout(modeLayout);

    expressionDisplay = new QLineEdit("");
    expressionDisplay->setReadOnly(true);
    expressionDisplay->setAlignment(Qt::AlignRight);

    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);

    mainLayout->addWidget(expressionDisplay);
    mainLayout->addWidget(display);

    QGridLayout *buttonLayout = new QGridLayout();

    setupMemoryButtons(buttonLayout);

    setupScientificButtons(buttonLayout);

    const char* basicButtons[20] = {
        "C", "⌫", "%", "÷",
        "7", "8", "9", "×",
        "4", "5", "6", "-",
        "1", "2", "3", "+",
        "±", "0", ".", "="
    };

    for(int i = 0; i < 20; ++i) {
        QString style = "";
        if(QString(basicButtons[i]) == "C" || QString(basicButtons[i]) == "⌫")
            style = "background-color: #FF5555;";
        else if(QString(basicButtons[i]) == "=")
            style = "background-color: #5555FF;";

        QPushButton *button = createButton(basicButtons[i], style);
        buttonLayout->addWidget(button, 4 + i/4, i%4);
    }

    mainLayout->addLayout(buttonLayout);
    centralWidget->setLayout(mainLayout);
    setFixedSize(400, 600);
}

void MainWindow::setupMemoryButtons(QGridLayout *layout)
{
    const char* memoryButtons[6] = {"MC", "MR", "M+", "M-", "MS", "MH"};
    for(int i = 0; i < 6; ++i) {
        QPushButton *button = createButton(memoryButtons[i], "background-color: #333333;");
        layout->addWidget(button, 0, i);
        connect(button, &QPushButton::clicked, this, &MainWindow::memoryPressed);
    }
}

void MainWindow::setupScientificButtons(QGridLayout *layout)
{
    const char* row1Buttons[4] = {"sin", "cos", "tan", "log"};
    for(int i = 0; i < 4; ++i) {
        QPushButton *button = createButton(row1Buttons[i]);
        layout->addWidget(button, 1, i);
        connect(button, &QPushButton::clicked, this, &MainWindow::scientificOperatorPressed);
    }

    const char* row2Buttons[4] = {"asin", "acos", "atan", "ln"};
    for(int i = 0; i < 4; ++i) {
        QPushButton *button = createButton(row2Buttons[i]);
        layout->addWidget(button, 2, i);
        connect(button, &QPushButton::clicked, this, &MainWindow::scientificOperatorPressed);
    }

    const char* row3Buttons[4] = {"x²", "x³", "xⁿ", "√"};
    for(int i = 0; i < 4; ++i) {
        QPushButton *button = createButton(row3Buttons[i]);
        layout->addWidget(button, 3, i);
        connect(button, &QPushButton::clicked, this, &MainWindow::scientificOperatorPressed);
    }
}

QPushButton* MainWindow::createButton(const QString &text, const QString &style)
{
    QPushButton *button = new QPushButton(text);
    if (!style.isEmpty()) {
        button->setStyleSheet(style);
    }

    if (text >= "0" && text <= "9" || text == "." || text == "±") {
        connect(button, &QPushButton::clicked, this, &MainWindow::numberPressed);
    } else if (text == "=" || text == "C" || text == "⌫") {
        if (text == "=") connect(button, &QPushButton::clicked, this, &MainWindow::equalPressed);
        if (text == "C") connect(button, &QPushButton::clicked, this, &MainWindow::clearPressed);
        if (text == "⌫") connect(button, &QPushButton::clicked, this, &MainWindow::backspacePressed);
    } else if (text == "+" || text == "-" || text == "×" || text == "÷" || text == "%") {
        connect(button, &QPushButton::clicked, this, &MainWindow::operatorPressed);
    }

    return button;
}

void MainWindow::numberPressed()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QString digit = button->text();

    if (digit == "±") {
        QString currentText = display->text();
        if (currentText.startsWith("-")) {
            currentText.remove(0, 1);
        } else {
            currentText.prepend("-");
        }
        display->setText(currentText);
        return;
    }

    if (waitingForOperand) {
        display->clear();
        waitingForOperand = false;
    }

    if (digit == "." && display->text().contains(".")) {
        return;
    }

    currentNumber = display->text() + digit;
    display->setText(currentNumber);
    updateDisplay();
}

void MainWindow::operatorPressed()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QString newOperator = button->text();

    if (!waitingForOperand) {
        calculateResult();
        waitingForOperand = true;
    }

    storedNumber = display->text();
    pendingOperator = newOperator;
    currentExpression = storedNumber + " " + newOperator + " ";
    expressionDisplay->setText(currentExpression);
}

void MainWindow::scientificOperatorPressed()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QString op = button->text();
    double number = display->text().toDouble();
    double result = 0;

    if (op == "sin") result = sin(convertToRadians(number));
    else if (op == "cos") result = cos(convertToRadians(number));
    else if (op == "tan") result = tan(convertToRadians(number));
    else if (op == "asin") result = convertToDegrees(asin(number));
    else if (op == "acos") result = convertToDegrees(acos(number));
    else if (op == "atan") result = convertToDegrees(atan(number));
    else if (op == "log") result = log10(number);
    else if (op == "ln") result = log(number);
    else if (op == "x²") result = pow(number, 2);
    else if (op == "x³") result = pow(number, 3);
    else if (op == "√") result = sqrt(number);
    else if (op == "xⁿ") {
        pendingOperator = "^";
        storedNumber = QString::number(number);
        currentExpression = storedNumber + " ^ ";
        expressionDisplay->setText(currentExpression);
        waitingForOperand = true;
        return;
    }

    if (isPrecise(result)) {
        display->setText(QString::number(result, 'g', 12));
        calculationHistory.push(result);
    } else {
        display->setText("Error");
    }

    waitingForOperand = true;
}

void MainWindow::equalPressed()
{
    if (!pendingOperator.isEmpty() && !waitingForOperand) {
        QString finalExpression = currentExpression + display->text();
        expressionDisplay->setText(finalExpression);
        calculateResult();
        pendingOperator.clear();
        waitingForOperand = true;
        currentExpression.clear();
    }
}

void MainWindow::clearPressed()
{
    currentNumber.clear();
    storedNumber.clear();
    pendingOperator.clear();
    currentExpression.clear();
    display->setText("0");
    expressionDisplay->clear();
    waitingForOperand = true;
}

void MainWindow::backspacePressed()
{
    if (waitingForOperand)
        return;

    QString text = display->text();
    text.chop(1);
    if (text.isEmpty() || text == "-") {
        text = "0";
        waitingForOperand = true;
    }
    display->setText(text);
    updateDisplay();
}

void MainWindow::calculateResult()
{
    double result = 0;
    double operand2 = display->text().toDouble();
    double operand1 = storedNumber.toDouble();

    if (pendingOperator == "+") result = operand1 + operand2;
    else if (pendingOperator == "-") result = operand1 - operand2;
    else if (pendingOperator == "×") result = operand1 * operand2;
    else if (pendingOperator == "÷") {
        if (operand2 != 0) {
            result = operand1 / operand2;
        } else {
            display->setText("Error");
            return;
        }
    }
    else if (pendingOperator == "%") result = operand1 * operand2 / 100.0;
    else if (pendingOperator == "^") result = pow(operand1, operand2);
    else result = operand2;

    if (isPrecise(result)) {
        display->setText(QString::number(result, 'g', 12));
        calculationHistory.push(result);
    } else {
        display->setText("Error");
    }
}

void MainWindow::updateDisplay()
{
    if (!pendingOperator.isEmpty()) {
        expressionDisplay->setText(currentExpression + display->text());
    }
}

void MainWindow::memoryPressed()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QString operation = button->text();
    double currentValue = display->text().toDouble();

    if (operation == "MC") memoryClear();
    else if (operation == "MR") memoryRecall();
    else if (operation == "M+") memoryAdd(currentValue);
    else if (operation == "M-") memorySubtract(currentValue);
    else if (operation == "MS") memoryStore();
    else if (operation == "MH") {
        QString history;
        QStack<double> tempStack = calculationHistory;
        while (!tempStack.isEmpty()) {
            history += QString::number(tempStack.pop(), 'g', 12) + "\n";
        }
        QMessageBox::information(this, "Calculation History", history);
    }
}

void MainWindow::memoryAdd(double value)
{
    memoryValue += value;
}

void MainWindow::memorySubtract(double value)
{
    memoryValue -= value;
}

void MainWindow::memoryRecall()
{
    display->setText(QString::number(memoryValue, 'g', 12));
    waitingForOperand = true;
}

void MainWindow::memoryClear()
{
    memoryValue = 0;
}

void MainWindow::memoryStore()
{
    memoryValue = display->text().toDouble();
}

void MainWindow::toggleAngleMode()
{
    isInRadianMode = !isInRadianMode;
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    button->setText(isInRadianMode ? "RAD" : "DEG");
}

void MainWindow::toggleInverse()
{
    isInverseMode = !isInverseMode;
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    button->setStyleSheet(isInverseMode ?
                              "background-color: #555555;" : "background-color: #333333;");
}

void MainWindow::shiftPressed()
{
    isShiftMode = !isShiftMode;
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    button->setStyleSheet(isShiftMode ?
                              "background-color: #555555;" : "background-color: #333333;");
}

double MainWindow::convertToRadians(double value)
{
    return isInRadianMode ? value : value * M_PI / 180.0;
}

double MainWindow::convertToDegrees(double value)
{
    return isInRadianMode ? value : value * 180.0 / M_PI;
}

bool MainWindow::isPrecise(double value, int precision)
{
    return !std::isnan(value) && !std::isinf(value) &&
           abs(value) < pow(10, precision);
}

void MainWindow::constantPressed()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    double value;
    QString constantSymbol = button->text();

    if (constantSymbol == "π") {
        value = M_PI;
    } else if (constantSymbol == "e") {
        value = M_E;
    } else if (constantSymbol == "φ") {
        value = 1.618033988749895;
    } else {
        return;
    }

    display->setText(QString::number(value, 'g', 12));
    waitingForOperand = true;

    calculationHistory.push(value);
}

void MainWindow::applyDarkTheme()
{
    QString buttonStyle = "QPushButton {"
                          "    background-color: #424242;"
                          "    color: white;"
                          "    border: none;"
                          "    padding: 10px;"
                          "    font-size: 14px;"
                          "    border-radius: 15px;"
                          "    margin: 1px;"
                          "}"
                          "QPushButton:pressed {"
                          "    background-color: #616161;"
                          "}";

    QString displayStyle = "QLineEdit {"
                           "    background-color: #212121;"
                           "    color: white;"
                           "    border: none;"
                           "    padding: 15px;"
                           "    font-size: 20px;"
                           "    border-radius: 5px;"
                           "    margin-bottom: 5px;"
                           "}"
                           "QLineEdit#expressionDisplay {"
                           "    font-size: 14px;"
                           "    color: #888888;"
                           "}";

    QString windowStyle = "MainWindow {"
                          "    background-color: #212121;"
                          "}";

    setStyleSheet(windowStyle + buttonStyle + displayStyle);

    display->setObjectName("mainDisplay");
    expressionDisplay->setObjectName("expressionDisplay");

    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text().length() > 1 &&
            !button->text().contains(QRegularExpression("[0-9]"))) {
            button->setStyleSheet(button->styleSheet() +
                                  "QPushButton { background-color: #333333; }");
        }
        else if (button->text().length() == 1 &&
                 button->text()[0].isDigit()) {
            button->setStyleSheet(button->styleSheet() +
                                  "QPushButton { font-weight: bold; }");
        }
    }
}
