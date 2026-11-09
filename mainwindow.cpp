#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTemporaryFile>
#include <QTextStream>
#include <QRegularExpression>
#include <cmath>
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      prevPC(0),
      stepCount(0),
      initialEntryPoint(0),
      timerIntervalMs(150)
{
    setWindowTitle("Vole Machine Simulator - Brookshear Architecture");
    resize(1300, 860);
    setMinimumSize(1050, 700);

    for (int i = 0; i < 16; ++i) prevRegs[i] = 0;
    for (int i = 0; i < 256; ++i) prevMem[i] = 0;

    runTimer = new QTimer(this);
    connect(runTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    setupUI();
    applyTheme();
    updateAllViews();
    logMessage("Simulator initialized. Ready to load program.");
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    // =========================================================================
    // 1. TOP TOOLBAR / CONTROL BAR
    // =========================================================================
    QGroupBox* topBar = new QGroupBox("Control Panel", this);
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 6, 10, 6);
    topLayout->setSpacing(8);

    // File loading section
    filePathEdit = new QLineEdit(this);
    filePathEdit->setPlaceholderText("Program file path (*.txt)...");
    filePathEdit->setReadOnly(true);
    btnBrowse = new QPushButton("Browse...", this);
    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::onLoadFileClicked);

    QLabel* lblStart = new QLabel("Load Addr (Hex):", this);
    spinStartAddress = new QSpinBox(this);
    spinStartAddress->setDisplayIntegerBase(16);
    spinStartAddress->setRange(0, 254);
    spinStartAddress->setValue(0);
    spinStartAddress->setPrefix("0x");

    QLabel* lblEntry = new QLabel("Start PC (Hex):", this);
    spinEntryPoint = new QSpinBox(this);
    spinEntryPoint->setDisplayIntegerBase(16);
    spinEntryPoint->setRange(0, 254);
    spinEntryPoint->setValue(0);
    spinEntryPoint->setPrefix("0x");

    btnLoad = new QPushButton("Load File", this);
    btnLoad->setObjectName("btnLoad");
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadProgramClicked);

    // Execution controls
    btnStep = new QPushButton("Step", this);
    btnStep->setObjectName("btnStep");
    connect(btnStep, &QPushButton::clicked, this, &MainWindow::onStepClicked);

    btnRun = new QPushButton("Run (Timed)", this);
    btnRun->setObjectName("btnRun");
    connect(btnRun, &QPushButton::clicked, this, &MainWindow::onRunClicked);

    btnRunToHalt = new QPushButton("Run to Halt", this);
    btnRunToHalt->setObjectName("btnRunToHalt");
    connect(btnRunToHalt, &QPushButton::clicked, this, &MainWindow::onRunToHaltClicked);

    btnReset = new QPushButton("Reset", this);
    btnReset->setObjectName("btnReset");
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);

    QLabel* lblSpeed = new QLabel("Speed:", this);
    speedSlider = new QSlider(Qt::Horizontal, this);
    speedSlider->setRange(20, 1000);
    speedSlider->setValue(150);
    speedSlider->setFixedWidth(100);
    speedLabel = new QLabel("150 ms", this);
    speedLabel->setFixedWidth(50);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);

    topLayout->addWidget(filePathEdit, 2);
    topLayout->addWidget(btnBrowse);
    topLayout->addWidget(lblStart);
    topLayout->addWidget(spinStartAddress);
    topLayout->addWidget(lblEntry);
    topLayout->addWidget(spinEntryPoint);
    topLayout->addWidget(btnLoad);
    topLayout->addSpacing(10);
    topLayout->addWidget(btnStep);
    topLayout->addWidget(btnRun);
    topLayout->addWidget(btnRunToHalt);
    topLayout->addWidget(btnReset);
    topLayout->addSpacing(10);
    topLayout->addWidget(lblSpeed);
    topLayout->addWidget(speedSlider);
    topLayout->addWidget(speedLabel);

    rootLayout->addWidget(topBar);

    // =========================================================================
    // 2. MAIN 3-COLUMN LAYOUT
    // =========================================================================
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(8);

    // -------------------------------------------------------------------------
    // LEFT COLUMN: CPU & REGISTERS
    // -------------------------------------------------------------------------
    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setSpacing(8);

    // CPU Card
    QGroupBox* cpuBox = new QGroupBox("CPU Status & Current Instruction", this);
    QVBoxLayout* cpuLayout = new QVBoxLayout(cpuBox);
    cpuLayout->setContentsMargins(10, 8, 10, 8);
    cpuLayout->setSpacing(6);

    QHBoxLayout* pcStatusRow = new QHBoxLayout();
    lblPC = new QLabel("PC: 0x00 (0)", this);
    lblPC->setObjectName("lblPC");
    lblStatus = new QLabel("READY", this);
    lblStatus->setObjectName("lblStatusReady");
    lblStatus->setAlignment(Qt::AlignCenter);
    pcStatusRow->addWidget(lblPC);
    pcStatusRow->addStretch();
    pcStatusRow->addWidget(lblStatus);
    cpuLayout->addLayout(pcStatusRow);

    QHBoxLayout* irRow = new QHBoxLayout();
    lblIRHex = new QLabel("IR: 0x0000", this);
    lblIRHex->setObjectName("lblIRHex");
    lblIROpcode = new QLabel("Opcode: -", this);
    irRow->addWidget(lblIRHex);
    irRow->addStretch();
    irRow->addWidget(lblIROpcode);
    cpuLayout->addLayout(irRow);

    lblIRDecoded = new QLabel("Decoded: (None)", this);
    lblIRDecoded->setObjectName("lblIRDecoded");
    lblIRDecoded->setWordWrap(true);
    cpuLayout->addWidget(lblIRDecoded);

    QHBoxLayout* setPcRow = new QHBoxLayout();
    QLabel* lblSet = new QLabel("Set PC:", this);
    spinNewPC = new QSpinBox(this);
    spinNewPC->setDisplayIntegerBase(16);
    spinNewPC->setRange(0, 254);
    spinNewPC->setPrefix("0x");
    btnSetPC = new QPushButton("Apply", this);
    connect(btnSetPC, &QPushButton::clicked, this, &MainWindow::onSetPcClicked);

    btnPc0 = new QPushButton("PC=00", this);
    btnPc0->setToolTip("Quickly jump PC to address 0x00");
    connect(btnPc0, &QPushButton::clicked, this, &MainWindow::onSetPcQuick0Clicked);

    btnPc1 = new QPushButton("PC=01", this);
    btnPc1->setToolTip("Quickly jump PC to address 0x01");
    connect(btnPc1, &QPushButton::clicked, this, &MainWindow::onSetPcQuick1Clicked);
    setPcRow->addWidget(lblSet);
    setPcRow->addWidget(spinNewPC);
    setPcRow->addWidget(btnSetPC);
    setPcRow->addWidget(btnPc0);
    setPcRow->addWidget(btnPc1);
    cpuLayout->addLayout(setPcRow);

    leftCol->addWidget(cpuBox);

    // Registers Card
    QGroupBox* regBox = new QGroupBox("Registers (R0 - RF)", this);
    QVBoxLayout* regLayout = new QVBoxLayout(regBox);
    regLayout->setContentsMargins(8, 6, 8, 6);

    regTable = new QTableWidget(16, 4, this);
    regTable->setHorizontalHeaderLabels({"Reg", "Hex", "Dec", "Float (Brookshear)"});
    regTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    regTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    regTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    regTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    regTable->verticalHeader()->setVisible(false);
    regTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    regTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(regTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onRegisterDoubleClicked);
    regLayout->addWidget(regTable);

    QLabel* regHint = new QLabel("(Double-click a register to edit its value)", this);
    regHint->setStyleSheet("color: #8888aa; font-size: 11px;");
    regLayout->addWidget(regHint);

    leftCol->addWidget(regBox, 1);
    bodyLayout->addLayout(leftCol, 3);

    // -------------------------------------------------------------------------
    // CENTER COLUMN: MEMORY GRID & SCREEN OUTPUT
    // -------------------------------------------------------------------------
    QVBoxLayout* centerCol = new QVBoxLayout();
    centerCol->setSpacing(8);

    QGroupBox* memBox = new QGroupBox("Memory Grid (256 Bytes)", this);
    QVBoxLayout* memLayout = new QVBoxLayout(memBox);
    memLayout->setContentsMargins(8, 6, 8, 6);

    memTable = new QTableWidget(16, 16, this);
    QStringList colHeaders;
    for (int c = 0; c < 16; ++c) {
        colHeaders << QString::number(c, 16).toUpper();
    }
    memTable->setHorizontalHeaderLabels(colHeaders);

    QStringList rowHeaders;
    for (int r = 0; r < 16; ++r) {
        rowHeaders << QString("%1:").arg(r * 16, 2, 16, QChar('0')).toUpper();
    }
    memTable->setVerticalHeaderLabels(rowHeaders);
    memTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(memTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onMemoryDoubleClicked);
    memLayout->addWidget(memTable);

    QLabel* memHint = new QLabel("(Double-click any cell to edit | PC pointer highlighted in blue | Screen 0xFF in purple)", this);
    memHint->setStyleSheet("color: #8888aa; font-size: 11px;");
    memLayout->addWidget(memHint);

    centerCol->addWidget(memBox, 3);

    // Screen Output (Address 0xFF)
    QGroupBox* screenBox = new QGroupBox("Screen Output (Mapped to Memory Address 0xFF & 0x00)", this);
    QVBoxLayout* screenLayout = new QVBoxLayout(screenBox);
    screenLayout->setContentsMargins(8, 6, 8, 6);

    QHBoxLayout* screenHeaderRow = new QHBoxLayout();
    lblScreenHex = new QLabel("Cell 0xFF Hex: 0x00", this);
    lblScreenChar = new QLabel("ASCII: '.'", this);
    btnClearScreen = new QPushButton("Clear Screen", this);
    connect(btnClearScreen, &QPushButton::clicked, this, &MainWindow::onClearScreenClicked);

    screenHeaderRow->addWidget(lblScreenHex);
    screenHeaderRow->addSpacing(15);
    screenHeaderRow->addWidget(lblScreenChar);
    screenHeaderRow->addStretch();
    screenHeaderRow->addWidget(btnClearScreen);
    screenLayout->addLayout(screenHeaderRow);

    screenTerminal = new QPlainTextEdit(this);
    screenTerminal->setReadOnly(true);
    screenTerminal->setMaximumHeight(100);
    screenTerminal->setPlaceholderText("Virtual display screen output will stream here when instructions write to 0xFF or 0x00...");
    screenLayout->addWidget(screenTerminal);

    centerCol->addWidget(screenBox, 1);
    bodyLayout->addLayout(centerCol, 5);

    // -------------------------------------------------------------------------
    // RIGHT COLUMN: PROGRAM EDITOR & EXECUTION TRACE LOG
    // -------------------------------------------------------------------------
    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setSpacing(8);

    rightTabs = new QTabWidget(this);

    // Tab 1: Program Editor
    QWidget* tabEditor = new QWidget();
    QVBoxLayout* editorLayout = new QVBoxLayout(tabEditor);
    editorLayout->setContentsMargins(8, 8, 8, 8);

    QHBoxLayout* sampleRow = new QHBoxLayout();
    QLabel* lblSamples = new QLabel("Sample Code:", this);
    sampleCombo = new QComboBox(this);
    sampleCombo->addItem("Select a sample program...");
    sampleCombo->addItem("1. Test Program (From test.txt)");
    sampleCombo->addItem("2. Brookshear Floating Point Add");
    sampleCombo->addItem("3. Bitwise Logic & Rotate");
    sampleCombo->addItem("4. Screen Print Loop (ASCII)");
    connect(sampleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSampleSelected);
    sampleRow->addWidget(lblSamples);
    sampleRow->addWidget(sampleCombo, 1);
    editorLayout->addLayout(sampleRow);

    codeEditor = new QPlainTextEdit(this);
    codeEditor->setPlaceholderText("; Write or paste Brookshear hex instructions here\n; Example:\n2105 ; R1 = 5\n2203 ; R2 = 3\n5312 ; R3 = R1 + R2\n33FF ; Screen = R3\nC000 ; Halt\n");
    editorLayout->addWidget(codeEditor);

    btnLoadEditor = new QPushButton("Load Code into Machine", this);
    btnLoadEditor->setObjectName("btnLoadEditor");
    connect(btnLoadEditor, &QPushButton::clicked, this, &MainWindow::onLoadFromEditorClicked);
    editorLayout->addWidget(btnLoadEditor);

    rightTabs->addTab(tabEditor, "Program Editor");

    // Tab 2: Execution Trace Log
    QWidget* tabLog = new QWidget();
    QVBoxLayout* logLayout = new QVBoxLayout(tabLog);
    logLayout->setContentsMargins(8, 8, 8, 8);

    logConsole = new QPlainTextEdit(this);
    logConsole->setReadOnly(true);
    logLayout->addWidget(logConsole);

    btnClearLog = new QPushButton("Clear Trace Log", this);
    connect(btnClearLog, &QPushButton::clicked, this, &MainWindow::onClearLogClicked);
    logLayout->addWidget(btnClearLog);

    rightTabs->addTab(tabLog, "Execution Trace");

    rightCol->addWidget(rightTabs);
    bodyLayout->addLayout(rightCol, 4);

    rootLayout->addLayout(bodyLayout, 1);

    // Status bar
    statusBar()->showMessage("Vole Simulator Ready.");
}

void MainWindow::applyTheme() {
    QString qss = R"(
        QMainWindow {
            background-color: #1a1b26;
            color: #c0caf5;
            font-family: 'Segoe UI', sans-serif;
            font-size: 13px;
        }
        QGroupBox {
            background-color: #24283b;
            border: 1px solid #414868;
            border-radius: 8px;
            margin-top: 14px;
            font-weight: bold;
            color: #7aa2f7;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
            color: #7aa2f7;
        }
        QLabel {
            color: #c0caf5;
        }
        QLineEdit, QSpinBox, QComboBox {
            background-color: #1f2335;
            border: 1px solid #414868;
            border-radius: 5px;
            padding: 4px 8px;
            color: #f7768e;
            selection-background-color: #3d59a1;
            font-family: 'Consolas', 'Courier New', monospace;
            font-weight: bold;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid #7aa2f7;
        }
        QPushButton {
            background-color: #3b4261;
            border: 1px solid #545c7e;
            border-radius: 5px;
            padding: 6px 12px;
            color: #c0caf5;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #485175;
            border-color: #7aa2f7;
        }
        QPushButton:pressed {
            background-color: #2e344e;
        }
        QPushButton#btnStep {
            background-color: #2ac3de;
            color: #1a1b26;
        }
        QPushButton#btnStep:hover {
            background-color: #7dcfff;
        }
        QPushButton#btnRun {
            background-color: #9ece6a;
            color: #1a1b26;
        }
        QPushButton#btnRun:hover {
            background-color: #b9f27c;
        }
        QPushButton#btnRunToHalt {
            background-color: #bb9af7;
            color: #1a1b26;
        }
        QPushButton#btnReset {
            background-color: #f7768e;
            color: #1a1b26;
        }
        QPushButton#btnLoad, QPushButton#btnLoadEditor {
            background-color: #7aa2f7;
            color: #1a1b26;
        }
        QTableWidget {
            background-color: #1f2335;
            alternate-background-color: #24283b;
            border: 1px solid #414868;
            border-radius: 6px;
            gridline-color: #292e42;
            color: #c0caf5;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
        }
        QHeaderView::section {
            background-color: #24283b;
            color: #7aa2f7;
            border: 1px solid #414868;
            padding: 4px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        QPlainTextEdit {
            background-color: #1f2335;
            border: 1px solid #414868;
            border-radius: 6px;
            color: #c0caf5;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
        }
        QTabWidget::pane {
            border: 1px solid #414868;
            border-radius: 6px;
            background-color: #24283b;
        }
        QTabBar::tab {
            background-color: #1f2335;
            border: 1px solid #414868;
            padding: 6px 14px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            color: #a9b1d6;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #24283b;
            color: #7aa2f7;
            border-bottom-color: #24283b;
            font-weight: bold;
        }
        QLabel#lblPC {
            font-size: 16px;
            font-weight: bold;
            color: #7dcfff;
            font-family: 'Consolas', monospace;
        }
        QLabel#lblIRHex {
            font-size: 15px;
            font-weight: bold;
            color: #bb9af7;
            font-family: 'Consolas', monospace;
        }
        QLabel#lblIRDecoded {
            background-color: #1a1b26;
            border: 1px solid #414868;
            border-radius: 5px;
            padding: 6px;
            color: #e0af68;
            font-family: 'Consolas', monospace;
            font-size: 12px;
        }
        QLabel#lblStatusReady {
            background-color: #3b4261;
            color: #c0caf5;
            font-weight: bold;
            border-radius: 4px;
            padding: 3px 8px;
        }
        QLabel#lblStatusRunning {
            background-color: #2ac3de;
            color: #1a1b26;
            font-weight: bold;
            border-radius: 4px;
            padding: 3px 8px;
        }
        QLabel#lblStatusHalted {
            background-color: #f7768e;
            color: #1a1b26;
            font-weight: bold;
            border-radius: 4px;
            padding: 3px 8px;
        }
    )";
    setStyleSheet(qss);
}

void MainWindow::detectFileHeaders(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    detectCodeHeaders(in.readAll());
}

void MainWindow::detectCodeHeaders(const QString& code) {
    QStringList lines = code.split('\n');
    for (const QString& rawLine : lines) {
        QString line = rawLine.trimmed();
        if (!line.startsWith(";")) continue;

        static QRegularExpression rePc(R"(;\s*pc\s*[:=]\s*([0-9a-fA-F]+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch matchPc = rePc.match(line);
        if (matchPc.hasMatch()) {
            bool ok = false;
            int pcVal = matchPc.captured(1).toInt(&ok, 16);
            if (ok && pcVal >= 0 && pcVal <= 254) {
                spinEntryPoint->setValue(pcVal);
                spinNewPC->setValue(pcVal);
                logMessage(QString("Auto-detected Starting PC from comments: 0x%1 (%2)")
                           .arg(pcVal, 2, 16, QChar('0')).toUpper().arg(pcVal));
            }
        }

        static QRegularExpression reStart(R"(;\s*start(?:\s*address)?\s*[:=]\s*([0-9a-fA-F]+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch matchStart = reStart.match(line);
        if (matchStart.hasMatch()) {
            bool ok = false;
            int startVal = matchStart.captured(1).toInt(&ok, 16);
            if (ok && startVal >= 0 && startVal <= 254) {
                spinStartAddress->setValue(startVal);
                logMessage(QString("Auto-detected Load Address from comments: 0x%1")
                           .arg(startVal, 2, 16, QChar('0')).toUpper());
            }
        }
    }
}

void MainWindow::onLoadFileClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Select Program File", "", "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        filePathEdit->setText(path);
        detectFileHeaders(path);
    }
}

void MainWindow::onLoadProgramClicked() {
    QString path = filePathEdit->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Load Error", "Please select or enter a program file path first.");
        return;
    }

    int start = spinStartAddress->value();
    int pc = spinEntryPoint->value();
    initialEntryPoint = pc;

    try {
        machine.loadProgram(path.toStdString(), start, pc);
        stepCount = 0;
        prevPC = machine.getCPU().getPC();
        for (int i = 0; i < 16; ++i) prevRegs[i] = machine.getCPU().getRegisters().read(i).getValue();
        for (int i = 0; i < 256; ++i) prevMem[i] = machine.getMemory().readCell(i).getValue();

        logMessage(QString("Program loaded successfully from '%1' at Load Address 0x%2, Entry Point 0x%3.")
                   .arg(path)
                   .arg(start, 2, 16, QChar('0')).toUpper()
                   .arg(pc, 2, 16, QChar('0')).toUpper());

        updateAllViews();
        statusBar()->showMessage("Program loaded successfully.");
    } catch (const exception& ex) {
        QMessageBox::critical(this, "Program Load Error", QString("Failed to load program: %1").arg(ex.what()));
        logMessage(QString("[ERROR] Failed to load program: %1").arg(ex.what()));
    }
}

void MainWindow::onLoadFromEditorClicked() {
    QString code = codeEditor->toPlainText().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "Editor Error", "Code editor is empty. Enter instructions or choose a sample.");
        return;
    }

    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        QMessageBox::critical(this, "Editor Error", "Could not create temporary file for loading.");
        return;
    }

    QTextStream out(&tempFile);
    out << code;
    tempFile.close();

    int start = spinStartAddress->value();
    int pc = spinEntryPoint->value();
    initialEntryPoint = pc;

    try {
        machine.loadProgram(tempFile.fileName().toStdString(), start, pc);
        stepCount = 0;
        prevPC = machine.getCPU().getPC();
        for (int i = 0; i < 16; ++i) prevRegs[i] = machine.getCPU().getRegisters().read(i).getValue();
        for (int i = 0; i < 256; ++i) prevMem[i] = machine.getMemory().readCell(i).getValue();

        logMessage(QString("Program loaded from Code Editor (Start: 0x%1, PC: 0x%2).")
                   .arg(start, 2, 16, QChar('0')).toUpper()
                   .arg(pc, 2, 16, QChar('0')).toUpper());

        updateAllViews();
        statusBar()->showMessage("Code editor program loaded.");
    } catch (const exception& ex) {
        QMessageBox::critical(this, "Editor Load Error", QString("Failed to load editor code: %1").arg(ex.what()));
        logMessage(QString("[ERROR] Failed to load editor code: %1").arg(ex.what()));
    }
}

void MainWindow::appendScreenOutput(uint8_t val, uint8_t addr) {
    QString addrStr = (addr == 0xFF) ? "0xFF" : "0x00";
    QString valHex = QString("%1").arg(val, 2, 16, QChar('0')).toUpper();

    QString entry;
    if (val >= 32 && val <= 126) {
        entry = QString("[Screen %1] ASCII: '%2'  (Hex: 0x%3, Dec: %4)\n")
                    .arg(addrStr).arg(static_cast<char>(val)).arg(valHex).arg(val);
    } else if (val == 10) {
        entry = QString("[Screen %1] Newline (\\n)\n").arg(addrStr);
    } else {
        entry = QString("[Screen %1] Value: %2  (Hex: 0x%3)\n")
                    .arg(addrStr).arg(val).arg(valHex);
    }

    screenTerminal->insertPlainText(entry);
    screenTerminal->ensureCursorVisible();
}

bool MainWindow::stepSimulation() {
    if (!machine.isProgramLoaded() || machine.getCPU().isHalted()) return false;

    int currentPc = machine.getCPU().getPC();
    uint8_t prevScreenFF = machine.getMemory().readCell(255).getValue();
    uint8_t prevScreen00 = machine.getMemory().readCell(0).getValue();

    uint8_t highByte = 0;
    uint8_t lowByte = 0;
    if (currentPc <= 254) {
        highByte = machine.getMemory().readCell(currentPc).getValue();
        lowByte = machine.getMemory().readCell(currentPc + 1).getValue();
    }
    uint8_t op = (highByte >> 4) & 0x0F;
    uint8_t r = highByte & 0x0F;
    uint8_t xy = lowByte;
    uint8_t rVal = machine.getCPU().getRegisters().read(r).getValue();

    machine.stepOnce();
    stepCount++;

    if (op == 3 && (xy == 0xFF || xy == 0x00)) {
        appendScreenOutput(rVal, xy);
    } else {
        uint8_t curScreenFF = machine.getMemory().readCell(255).getValue();
        uint8_t curScreen00 = machine.getMemory().readCell(0).getValue();
        if (curScreenFF != prevScreenFF) {
            appendScreenOutput(curScreenFF, 0xFF);
        }
        if (curScreen00 != prevScreen00 && stepCount > 1) {
            appendScreenOutput(curScreen00, 0x00);
        }
    }

    return true;
}
void MainWindow::onStepClicked() {
    if (!machine.isProgramLoaded()) {
        QMessageBox::information(this, "Notice", "Please load a program first.");
        return;
    }

    if (machine.getCPU().isHalted()) {
        QMessageBox::information(this, "Halted", "CPU is currently halted. Reset the machine to run again.");
        return;
    }

    int currentPc = machine.getCPU().getPC();
    uint8_t highByte = 0;
    uint8_t lowByte = 0;
    if (currentPc <= 254) {
        highByte = machine.getMemory().readCell(currentPc).getValue();
        lowByte = machine.getMemory().readCell(currentPc + 1).getValue();
    }

    try {
        stepSimulation();

        Instructions ir = machine.getCPU().getIR();
        QString desc = decodeInstruction(ir, highByte, lowByte);

        logMessage(QString("[Step %1] PC=0x%2 | %3")
                   .arg(stepCount)
                   .arg(currentPc, 2, 16, QChar('0')).toUpper()
                   .arg(desc));

        updateAllViews();

        if (machine.getCPU().isHalted()) {
            logMessage("=== Machine execution reached HALT (C000) ===");
            statusBar()->showMessage("CPU HALTED.");
        } else {
            statusBar()->showMessage(QString("Step %1 executed. Next PC: 0x%2")
                                     .arg(stepCount)
                                     .arg(machine.getCPU().getPC(), 2, 16, QChar('0')).toUpper());
        }

    } catch (const exception& ex) {
        QString errorMsg = QString::fromUtf8(ex.what());
        logMessage(QString("[ERROR during step %1] %2").arg(stepCount + 1).arg(errorMsg));
        statusBar()->showMessage("Execution halted: " + errorMsg);
        updateAllViews();

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Execution Error");
        msgBox.setText(QString("<b>Execution Halted on Error:</b><br>%1").arg(errorMsg));
        msgBox.setInformativeText(
            "<b>Brookshear Instruction Set Reference:</b><br>"
            "• Valid opcodes are <b>0x1 to 0xC (1 to 12)</b>.<br>"
            "• <code>0xE</code> (decimal 14) is not a valid Vole opcode.<br><br>"
            "<b>Troubleshooting:</b><br>"
            "1. Check for typos in your code (e.g., did you mean <code>210C</code>, <code>110C</code>, or <code>B10C</code> instead of <code>E10C</code>?).<br>"
            "2. Verify that your program ends with <code>C000</code> (HALT) so execution doesn't run into data cells.<br>"
            "3. If you placed variables in memory, make sure the Starting PC points to your first instruction, not a variable."
        );
        msgBox.exec();
    }
}

void MainWindow::onRunClicked() {
    if (!machine.isProgramLoaded()) {
        QMessageBox::information(this, "Notice", "Please load a program first.");
        return;
    }

    if (runTimer->isActive()) {
        runTimer->stop();
        btnRun->setText("Run (Timed)");
        lblStatus->setText("PAUSED");
        lblStatus->setObjectName("lblStatusReady");
        lblStatus->style()->unpolish(lblStatus);
        lblStatus->style()->polish(lblStatus);
        statusBar()->showMessage("Execution paused.");
    } else {
        if (machine.getCPU().isHalted()) {
            QMessageBox::information(this, "Halted", "CPU is already halted. Reset machine to run again.");
            return;
        }
        btnRun->setText("Pause");
        lblStatus->setText("RUNNING");
        lblStatus->setObjectName("lblStatusRunning");
        lblStatus->style()->unpolish(lblStatus);
        lblStatus->style()->polish(lblStatus);
        runTimer->start(timerIntervalMs);
        statusBar()->showMessage("Running program step-by-step...");
    }
}

void MainWindow::onTimerTick() {
    if (machine.getCPU().isHalted()) {
        runTimer->stop();
        btnRun->setText("Run (Timed)");
        updateAllViews();
        return;
    }
    onStepClicked();
    if (machine.getCPU().isHalted()) {
        runTimer->stop();
        btnRun->setText("Run (Timed)");
    }
}

void MainWindow::onRunToHaltClicked() {
    if (!machine.isProgramLoaded()) {
        QMessageBox::information(this, "Notice", "Please load a program first.");
        return;
    }

    if (runTimer->isActive()) {
        runTimer->stop();
        btnRun->setText("Run (Timed)");
    }

    try {
        logMessage("Executing runToHalt()...");
        const int MAX_STEPS = 100000;
        int executed = 0;
        while (!machine.getCPU().isHalted() && executed < MAX_STEPS) {
            stepSimulation();
            executed++;
        }

        updateAllViews();
        if (machine.getCPU().isHalted()) {
            logMessage(QString("=== Program executed to HALT (%1 instructions executed) ===").arg(executed));
            statusBar()->showMessage(QString("Execution completed (HALTED after %1 steps).").arg(executed));
        } else {
            logMessage("[WARNING] Step limit reached without HALT.");
            statusBar()->showMessage("Execution stopped: step limit reached.");
        }
    } catch (const exception& ex) {
        QString errorMsg = QString::fromUtf8(ex.what());
        logMessage(QString("[ERROR in runToHalt] %1").arg(errorMsg));
        statusBar()->showMessage("Execution halted: " + errorMsg);
        updateAllViews();

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Execution Error");
        msgBox.setText(QString("<b>Execution Halted on Error:</b><br>%1").arg(errorMsg));
        msgBox.setInformativeText(
            "<b>Brookshear Instruction Set Reference:</b><br>"
            "• Valid opcodes are <b>0x1 to 0xC (1 to 12)</b>.<br>"
            "• <code>0xE</code> (decimal 14) is not a valid Vole opcode.<br><br>"
            "<b>Troubleshooting:</b><br>"
            "1. Check for typos in your code (e.g., did you mean <code>210C</code>, <code>110C</code>, or <code>B10C</code> instead of <code>E10C</code>?).<br>"
            "2. Verify that your program ends with <code>C000</code> (HALT) so execution doesn't run into data cells.<br>"
            "3. If you placed variables in memory, make sure the Starting PC points to your first instruction, not a variable."
        );
        msgBox.exec();
    }
}

void MainWindow::onResetClicked() {
    if (runTimer->isActive()) {
        runTimer->stop();
        btnRun->setText("Run (Timed)");
    }

    machine.reset();
    stepCount = 0;
    prevPC = initialEntryPoint;
    machine.setPc(initialEntryPoint);
    for (int i = 0; i < 16; ++i) prevRegs[i] = 0;
    for (int i = 0; i < 256; ++i) prevMem[i] = 0;

    logMessage(QString("Machine state reset. PC restored to 0x%1.")
               .arg(initialEntryPoint, 2, 16, QChar('0')).toUpper());
    updateAllViews();
    statusBar()->showMessage(QString("Machine reset (PC: 0x%1).")
                             .arg(initialEntryPoint, 2, 16, QChar('0')).toUpper());
}

void MainWindow::onSetPcQuick0Clicked() {
    spinNewPC->setValue(0);
    onSetPcClicked();
}

void MainWindow::onSetPcQuick1Clicked() {
    spinNewPC->setValue(1);
    onSetPcClicked();
}

void MainWindow::onSetPcClicked() {
    int newPc = spinNewPC->value();
    try {
        machine.setPc(newPc);
        logMessage(QString("PC changed to 0x%1").arg(newPc, 2, 16, QChar('0')).toUpper());
        updateAllViews();
    } catch (const exception& ex) {
        QMessageBox::warning(this, "Set PC Error", ex.what());
    }
}

void MainWindow::onRegisterDoubleClicked(int row, int /*col*/) {
    if (row < 0 || row >= 16) return;

    uint8_t currentVal = machine.getCPU().getRegisters().read(row).getValue();
    QString currentHex = QString("%1").arg(currentVal, 2, 16, QChar('0')).toUpper();

    bool ok;
    QString text = QInputDialog::getText(this,
                                         QString("Edit Register R%1").arg(QString::number(row, 16).toUpper()),
                                         "Enter Hex Value (00 - FF):",
                                         QLineEdit::Normal,
                                         currentHex,
                                         &ok);
    if (!ok || text.trimmed().isEmpty()) return;

    try {
        Cell c;
        c.fromHex(text.trimmed().toStdString());
        machine.editRegister(row, c);
        logMessage(QString("Register R%1 updated to 0x%2")
                   .arg(QString::number(row, 16).toUpper())
                   .arg(c.toHex().c_str()));
        updateRegistersView();
    } catch (const exception& ex) {
        QMessageBox::warning(this, "Edit Error", QString("Invalid register value: %1").arg(ex.what()));
    }
}

void MainWindow::onMemoryDoubleClicked(int row, int col) {
    if (row < 0 || row >= 16 || col < 0 || col >= 16) return;
    int address = row * 16 + col;

    uint8_t currentVal = machine.getMemory().readCell(address).getValue();
    QString currentHex = QString("%1").arg(currentVal, 2, 16, QChar('0')).toUpper();

    bool ok;
    QString text = QInputDialog::getText(this,
                                         QString("Edit Memory Cell 0x%1").arg(address, 2, 16, QChar('0')).toUpper(),
                                         "Enter Hex Byte (00 - FF):",
                                         QLineEdit::Normal,
                                         currentHex,
                                         &ok);
    if (!ok || text.trimmed().isEmpty()) return;

    try {
        Cell c;
        c.fromHex(text.trimmed().toStdString());
        machine.editMemory(address, c);
        logMessage(QString("Memory [0x%1] updated to 0x%2")
                   .arg(address, 2, 16, QChar('0')).toUpper()
                   .arg(c.toHex().c_str()));
        updateMemoryView();
    } catch (const exception& ex) {
        QMessageBox::warning(this, "Edit Error", QString("Invalid memory value: %1").arg(ex.what()));
    }
}

void MainWindow::onClearScreenClicked() {
    screenTerminal->clear();
    machine.getMemory().clearScreen();
    updateScreenView();
    logMessage("Screen terminal and buffer cleared.");
}

void MainWindow::onClearLogClicked() {
    logConsole->clear();
}

void MainWindow::onSpeedChanged(int value) {
    timerIntervalMs = value;
    speedLabel->setText(QString("%1 ms").arg(value));
    if (runTimer->isActive()) {
        runTimer->setInterval(timerIntervalMs);
    }
}

void MainWindow::onSampleSelected(int index) {
    switch (index) {
        case 1: // test.txt
            codeEditor->setPlainText(
                "; Vole Machine test.txt from project\n"
                "; Variables at 0x00, 0x01, 0x02\n"
                "01      ; Mem[0x00] = 1\n"
                "02      ; Mem[0x01] = 2\n"
                "0A      ; Mem[0x02] = 10\n"
                "1000    ; R0 <- Mem[0x00]\n"
                "1101    ; R1 <- Mem[0x01]\n"
                "1202    ; R2 <- Mem[0x02]\n"
                "5301    ; R3 <- R0 + R1\n"
                "5423    ; R4 <- R2 + R3\n"
                "34FF    ; Store R4 to Screen (0xFF)\n"
                "C000    ; Halt\n"
            );
            spinStartAddress->setValue(0);
            spinEntryPoint->setValue(3);
            break;
        case 2: // Floating point
            codeEditor->setPlainText(
                "; Brookshear Floating Point Addition\n"
                "; 2.75 in hex = 0x5B, -1.25 in hex = 0xD4\n"
                "215B    ; R1 <- 0x5B (+2.75)\n"
                "22D4    ; R2 <- 0xD4 (-1.25)\n"
                "6312    ; R3 <- R1 + R2 (Float: +1.50 = 0x54)\n"
                "33FF    ; Output result to Screen\n"
                "C000    ; Halt\n"
            );
            spinStartAddress->setValue(0);
            spinEntryPoint->setValue(0);
            break;
        case 3: // Bitwise logic
            codeEditor->setPlainText(
                "; Bitwise Logic & Rotation Demonstration\n"
                "21AA    ; R1 <- 0xAA (10101010)\n"
                "220F    ; R2 <- 0x0F (00001111)\n"
                "7312    ; R3 <- R1 OR R2  (0xAF)\n"
                "8412    ; R4 <- R1 AND R2 (0x0A)\n"
                "9512    ; R5 <- R1 XOR R2 (0xA5)\n"
                "A602    ; R6 <- R6 rotated right by 2 (with initial val)\n"
                "C000    ; Halt\n"
            );
            spinStartAddress->setValue(0);
            spinEntryPoint->setValue(0);
            break;
        case 4: // Screen output loop
            codeEditor->setPlainText(
                "; Print Characters 'A', 'B', 'C' to Screen\n"
                "2141    ; R1 <- 'A' (0x41)\n"
                "31FF    ; Screen <- 'A'\n"
                "2201    ; R2 <- 1\n"
                "5112    ; R1 <- R1 + 1 ('B')\n"
                "31FF    ; Screen <- 'B'\n"
                "5112    ; R1 <- R1 + 1 ('C')\n"
                "31FF    ; Screen <- 'C'\n"
                "C000    ; Halt\n"
            );
            spinStartAddress->setValue(0);
            spinEntryPoint->setValue(0);
            break;
        default:
            break;
    }
}

void MainWindow::updateAllViews() {
    updateCPUView();
    updateRegistersView();
    updateMemoryView();
    updateScreenView();
}

void MainWindow::updateCPUView() {
    int pc = machine.getCPU().getPC();
    lblPC->setText(QString("PC: 0x%1 (%2)")
                   .arg(pc, 2, 16, QChar('0')).toUpper()
                   .arg(pc));
    spinNewPC->setValue(pc);

    if (machine.getCPU().isHalted()) {
        lblStatus->setText("HALTED");
        lblStatus->setObjectName("lblStatusHalted");
    } else if (runTimer->isActive()) {
        lblStatus->setText("RUNNING");
        lblStatus->setObjectName("lblStatusRunning");
    } else {
        lblStatus->setText(machine.isProgramLoaded() ? "LOADED" : "READY");
        lblStatus->setObjectName("lblStatusReady");
    }
    lblStatus->style()->unpolish(lblStatus);
    lblStatus->style()->polish(lblStatus);

    // Current instruction preview
    if (pc <= 254) {
        uint8_t h = machine.getMemory().readCell(pc).getValue();
        uint8_t l = machine.getMemory().readCell(pc + 1).getValue();
        lblIRHex->setText(QString("IR: 0x%1%2")
                          .arg(h, 2, 16, QChar('0')).toUpper()
                          .arg(l, 2, 16, QChar('0')).toUpper());
        try {
            Instructions ir = Instructions::decode(Cell(h), Cell(l));
            lblIROpcode->setText(QString("Opcode: %1").arg(QString::number(ir.getOpcode(), 16).toUpper()));
            lblIRDecoded->setText(decodeInstruction(ir, h, l));
        } catch (...) {
            lblIROpcode->setText("Opcode: ?");
            lblIRDecoded->setText("Non-instruction or invalid code at current PC.");
        }
    } else {
        lblIRHex->setText("IR: 0x----");
        lblIROpcode->setText("Opcode: -");
        lblIRDecoded->setText("PC out of memory bounds.");
    }
}

void MainWindow::updateRegistersView() {
    for (int i = 0; i < 16; ++i) {
        uint8_t val = machine.getCPU().getRegisters().read(i).getValue();
        bool changed = (val != prevRegs[i] && stepCount > 0);

        QString regName = QString("R%1").arg(QString::number(i, 16).toUpper());
        QString hexVal = QString("%1").arg(val, 2, 16, QChar('0')).toUpper();
        QString decVal = QString("%1").arg(static_cast<int>(val));
        QString floatVal = formatFloat(val);

        auto setItem = [&](int col, const QString& text, Qt::Alignment align) {
            QTableWidgetItem* item = regTable->item(i, col);
            if (!item) {
                item = new QTableWidgetItem();
                regTable->setItem(i, col, item);
            }
            item->setText(text);
            item->setTextAlignment(align);
            if (changed) {
                item->setBackground(QColor(230, 160, 40, 80)); // Amber accent highlight
            } else {
                item->setBackground(QBrush(Qt::NoBrush));
            }
        };

        setItem(0, regName, Qt::AlignCenter);
        setItem(1, hexVal, Qt::AlignCenter);
        setItem(2, decVal, Qt::AlignRight | Qt::AlignVCenter);
        setItem(3, floatVal, Qt::AlignRight | Qt::AlignVCenter);

        prevRegs[i] = val;
    }
}

void MainWindow::updateMemoryView() {
    int pc = machine.getCPU().getPC();

    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 16; ++c) {
            int addr = r * 16 + c;
            uint8_t val = machine.getMemory().readCell(addr).getValue();
            bool changed = (val != prevMem[addr] && stepCount > 0);

            QString hexText = QString("%1").arg(val, 2, 16, QChar('0')).toUpper();

            QTableWidgetItem* item = memTable->item(r, c);
            if (!item) {
                item = new QTableWidgetItem();
                memTable->setItem(r, c, item);
            }
            item->setText(hexText);
            item->setTextAlignment(Qt::AlignCenter);
            item->setToolTip(QString("Address: 0x%1 (%2)\nValue: 0x%3 (%4)")
                             .arg(addr, 2, 16, QChar('0')).toUpper()
                             .arg(addr)
                             .arg(hexText)
                             .arg(static_cast<int>(val)));

            // Color highlighting:
            if (addr == pc) {
                // PC Instruction High byte
                item->setBackground(QColor(61, 89, 171));
                item->setForeground(QColor(255, 255, 255));
                item->setFont(QFont("Consolas", 10, QFont::Bold));
            } else if (addr == pc + 1 && pc < 255) {
                // PC Instruction Low byte
                item->setBackground(QColor(42, 120, 180));
                item->setForeground(QColor(255, 255, 255));
                item->setFont(QFont("Consolas", 10, QFont::Bold));
            } else if (addr == 255) {
                // Screen buffer cell 0xFF
                item->setBackground(QColor(124, 58, 237, 100));
                item->setForeground(QColor(255, 220, 100));
            } else if (changed) {
                // Recently modified cell
                item->setBackground(QColor(230, 160, 40, 80));
                item->setForeground(QColor(255, 255, 255));
            } else {
                item->setBackground(QBrush(Qt::NoBrush));
                item->setForeground(QColor(192, 202, 245));
                item->setFont(QFont("Consolas", 10));
            }

            prevMem[addr] = val;
        }
    }
}

void MainWindow::updateScreenView() {
    uint8_t screenByteFF = machine.getMemory().readCell(255).getValue();
    uint8_t screenByte00 = machine.getMemory().readCell(0).getValue();

    lblScreenHex->setText(QString("Mem[0xFF]: 0x%1 (%2)  |  Mem[0x00]: 0x%3 (%4)")
                          .arg(screenByteFF, 2, 16, QChar('0')).toUpper().arg(screenByteFF)
                          .arg(screenByte00, 2, 16, QChar('0')).toUpper().arg(screenByte00));

    QString asciiFF = (screenByteFF >= 32 && screenByteFF <= 126) ? QString("'%1'").arg(static_cast<char>(screenByteFF)) : "non-printable";
    QString ascii00 = (screenByte00 >= 32 && screenByte00 <= 126) ? QString("'%1'").arg(static_cast<char>(screenByte00)) : "non-printable";

    lblScreenChar->setText(QString("ASCII -> [0xFF: %1, 0x00: %2]").arg(asciiFF).arg(ascii00));
}

void MainWindow::logMessage(const QString& msg) {
    logConsole->appendPlainText(msg);
}

QString MainWindow::decodeInstruction(const Instructions& ir, uint8_t highByte, uint8_t lowByte) {
    uint8_t op = ir.getOpcode();
    uint8_t r = ir.getMain_Register();
    uint8_t s = ir.getRegister1();
    uint8_t t = ir.getRegister2();
    uint8_t xy = ir.getMemoryXY();

    QString hexCode = QString("%1%2")
                      .arg(highByte, 2, 16, QChar('0'))
                      .arg(lowByte, 2, 16, QChar('0')).toUpper();

    switch (op) {
        case 1:
            return QString("[%1] LOAD R%2 <- Mem[0x%3]")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString("%1").arg(xy, 2, 16, QChar('0')).toUpper());
        case 2:
            return QString("[%1] LOAD R%2 <- 0x%3 (Immediate)")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString("%1").arg(xy, 2, 16, QChar('0')).toUpper());
        case 3:
            if (xy == 0xFF) {
                return QString("[%1] STORE R%2 -> Screen (Mem[0xFF])")
                    .arg(hexCode).arg(QString::number(r, 16).toUpper());
            } else if (xy == 0x00) {
                return QString("[%1] STORE R%2 -> Screen (Mem[0x00])")
                    .arg(hexCode).arg(QString::number(r, 16).toUpper());
            }
            return QString("[%1] STORE R%2 -> Mem[0x%3]")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString("%1").arg(xy, 2, 16, QChar('0')).toUpper());
        case 4:
            return QString("[%1] MOVE R%2 <- R%3")
                .arg(hexCode)
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 5:
            return QString("[%1] ADD INT: R%2 <- R%3 + R%4 (Two's Comp)")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 6:
            return QString("[%1] ADD FLOAT: R%2 <- R%3 + R%4 (Brookshear Float)")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 7:
            return QString("[%1] OR: R%2 <- R%3 | R%4")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 8:
            return QString("[%1] AND: R%2 <- R%3 & R%4")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 9:
            return QString("[%1] XOR: R%2 <- R%3 ^ R%4")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString::number(s, 16).toUpper())
                .arg(QString::number(t, 16).toUpper());
        case 0xA:
            return QString("[%1] ROTATE RIGHT: R%2 by %3 bits")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(t);
        case 0xB:
            return QString("[%1] JUMP: If R%2 == R0 -> PC=0x%3")
                .arg(hexCode)
                .arg(QString::number(r, 16).toUpper())
                .arg(QString("%1").arg(xy, 2, 16, QChar('0')).toUpper());
        case 0xC:
            return QString("[%1] HALT: Stop Execution").arg(hexCode);
        default:
            return QString("[%1] Unknown Opcode 0x%2").arg(hexCode).arg(QString::number(op, 16).toUpper());
    }
}

QString MainWindow::formatFloat(uint8_t byteVal) {
    int sign = (byteVal >> 7) & 1;
    int exp = (byteVal >> 4) & 7;
    int mant = byteVal & 0x0F;
    if (mant == 0) return "0.0";
    double fraction = mant / 16.0;
    int power = exp - 4;
    double val = (sign ? -1.0 : 1.0) * fraction * std::pow(2.0, power);
    return QString::number(val, 'g', 4);
}
