#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QTimer>
#include <QTabWidget>
#include <QStatusBar>
#include <QStyle>

#include "Machine.h"
#include "Instructions.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onLoadFileClicked();
    void onLoadProgramClicked();
    void onLoadFromEditorClicked();
    void onStepClicked();
    void onRunClicked();
    void onRunToHaltClicked();
    void onResetClicked();
    void onSetPcClicked();
    void onSetPcQuick0Clicked();
    void onSetPcQuick1Clicked();
    void onTimerTick();
    void onRegisterDoubleClicked(int row, int col);
    void onMemoryDoubleClicked(int row, int col);
    void onClearScreenClicked();
    void onClearLogClicked();
    void onSampleSelected(int index);
    void onSpeedChanged(int value);

private:
    void setupUI();
    void applyTheme();
    void updateAllViews();
    void updateCPUView();
    void updateRegistersView();
    void updateMemoryView();
    void updateScreenView();
    void logMessage(const QString& msg);
    QString decodeInstruction(const Instructions& ir, uint8_t highByte, uint8_t lowByte);
    static QString formatFloat(uint8_t byteVal);

    bool stepSimulation();
    void appendScreenOutput(uint8_t val, uint8_t addr);
    void detectFileHeaders(const QString& filePath);
    void detectCodeHeaders(const QString& code);

    // Core simulation engine
    Machine machine;

    // State tracking for visual change highlighting
    uint8_t prevRegs[16];
    uint8_t prevMem[256];
    int prevPC;
    int stepCount;
    int initialEntryPoint;

    // Animation timer
    QTimer* runTimer;
    int timerIntervalMs;

    // Widgets: Toolbar / Controls
    QLineEdit* filePathEdit;
    QPushButton* btnBrowse;
    QSpinBox* spinStartAddress;
    QSpinBox* spinEntryPoint;
    QPushButton* btnLoad;

    QPushButton* btnStep;
    QPushButton* btnRun;
    QPushButton* btnRunToHalt;
    QPushButton* btnReset;
    QSlider* speedSlider;
    QLabel* speedLabel;

    // Widgets: CPU Panel
    QLabel* lblPC;
    QLabel* lblStatus;
    QLabel* lblIRHex;
    QLabel* lblIROpcode;
    QLabel* lblIRDecoded;
    QSpinBox* spinNewPC;
    QPushButton* btnSetPC;
    QPushButton* btnPc0;
    QPushButton* btnPc1;

    // Widgets: Registers Panel
    QTableWidget* regTable;

    // Widgets: Memory Panel
    QTableWidget* memTable;

    // Widgets: Screen Output
    QLabel* lblScreenHex;
    QLabel* lblScreenChar;
    QPlainTextEdit* screenTerminal;
    QPushButton* btnClearScreen;

    // Widgets: Right side (Editor + Log)
    QTabWidget* rightTabs;
    QComboBox* sampleCombo;
    QPlainTextEdit* codeEditor;
    QPushButton* btnLoadEditor;
    QPlainTextEdit* logConsole;
    QPushButton* btnClearLog;
};
