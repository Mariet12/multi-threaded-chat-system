/*
 * MIT License
 * Main GUI Window Implementation
 */

#include "MainWindow.h"
#include "SocketClient.h"
#include "ShmClient.h"
#include "common.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), currentMode(SOCKET_MODE) {
    
    socketClient = std::make_unique<SocketClient>(this);
    shmClient = std::make_unique<ShmClient>(this);
    
    setupUi();
    
    connect(socketClient.get(), &SocketClient::messageReceived,
            this, &MainWindow::onMessageReceived);
    connect(socketClient.get(), &SocketClient::userListUpdated,
            this, &MainWindow::onUserListUpdated);
    connect(socketClient.get(), &SocketClient::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(socketClient.get(), &SocketClient::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    
    connect(shmClient.get(), &ShmClient::messageReceived,
            this, &MainWindow::onMessageReceived);
    connect(shmClient.get(), &ShmClient::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(shmClient.get(), &ShmClient::errorOccurred,
            this, &MainWindow::onErrorOccurred);
}

MainWindow::~MainWindow() {
    if (socketClient->isConnected()) {
        socketClient->disconnect();
    }
    if (shmClient->isConnected()) {
        shmClient->leaveRoom();
    }
}

void MainWindow::setupUi() {
    setWindowTitle("Multi-Threaded Chat System");
    resize(900, 600);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    QVBoxLayout* leftLayout = new QVBoxLayout();
    
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    leftLayout->addWidget(chatDisplay);
    
    QHBoxLayout* inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type your message here...");
    sendButton = new QPushButton("Send", this);
    
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    leftLayout->addLayout(inputLayout);
    
    mainLayout->addLayout(leftLayout, 3);
    
    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    QGroupBox* modeGroup = new QGroupBox("Connection Mode", this);
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    modeSelector = new QComboBox(this);
    modeSelector->addItem("Socket (Network)");
    modeSelector->addItem("Shared Memory (Local)");
    modeLayout->addWidget(modeSelector);
    rightLayout->addWidget(modeGroup);
    
    QGroupBox* userGroup = new QGroupBox("User Info", this);
    QVBoxLayout* userLayout = new QVBoxLayout(userGroup);
    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    usernameInput->setText("user_" + QString::number(QTime::currentTime().msec()));
    userLayout->addWidget(usernameInput);
    rightLayout->addWidget(userGroup);
    
    QGroupBox* socketGroup = new QGroupBox("Socket Settings", this);
    QVBoxLayout* socketLayout = new QVBoxLayout(socketGroup);
    serverIpInput = new QLineEdit(this);
    serverIpInput->setPlaceholderText("Server IP");
    serverIpInput->setText("127.0.0.1");
    serverPortInput = new QLineEdit(this);
    serverPortInput->setPlaceholderText("Port");
    serverPortInput->setText("5000");
    socketLayout->addWidget(serverIpInput);
    socketLayout->addWidget(serverPortInput);
    rightLayout->addWidget(socketGroup);
    
    QGroupBox* shmGroup = new QGroupBox("Shared Memory Settings", this);
    QVBoxLayout* shmLayout = new QVBoxLayout(shmGroup);
    shmNameInput = new QLineEdit(this);
    shmNameInput->setPlaceholderText("Shared Memory Name");
    shmNameInput->setText("/os_chat_shm");
    shmLayout->addWidget(shmNameInput);
    rightLayout->addWidget(shmGroup);
    
    connectButton = new QPushButton("Connect", this);
    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setEnabled(false);
    rightLayout->addWidget(connectButton);
    rightLayout->addWidget(disconnectButton);
    
    statusLabel = new QLabel("Status: Disconnected", this);
    statusLabel->setStyleSheet("QLabel { background-color: #ffcccc; padding: 5px; }");
    rightLayout->addWidget(statusLabel);
    
    QGroupBox* usersGroup = new QGroupBox("Online Users", this);
    QVBoxLayout* usersLayout = new QVBoxLayout(usersGroup);
    usersList = new QListWidget(this);
    usersLayout->addWidget(usersList);
    rightLayout->addWidget(usersGroup);
    
    rightLayout->addStretch();
    mainLayout->addLayout(rightLayout, 1);
    
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
    connect(modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeChanged);
}

void MainWindow::onModeChanged(int index) {
    currentMode = (index == 0) ? SOCKET_MODE : SHM_MODE;
}

void MainWindow::onConnectClicked() {
    QString username = usernameInput->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a username");
        return;
    }
    
    bool success = false;
    
    if (currentMode == SOCKET_MODE) {
        QString ip = serverIpInput->text().trimmed();
        int port = serverPortInput->text().toInt();
        
        if (ip.isEmpty() || port <= 0) {
            QMessageBox::warning(this, "Error", "Invalid server IP or port");
            return;
        }
        
        success = socketClient->connectToServer(ip, port, username);
    } else {
        QString shmName = shmNameInput->text().trimmed();
        if (shmName.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter shared memory name");
            return;
        }
        
        success = shmClient->joinRoom(shmName, username);
    }
    
    if (success) {
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        modeSelector->setEnabled(false);
        usernameInput->setEnabled(false);
    }
}

void MainWindow::onDisconnectClicked() {
    if (currentMode == SOCKET_MODE) {
        socketClient->disconnect();
    } else {
        shmClient->leaveRoom();
    }
    
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    modeSelector->setEnabled(true);
    usernameInput->setEnabled(true);
    usersList->clear();
}

void MainWindow::onSendClicked() {
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    if (currentMode == SOCKET_MODE && socketClient->isConnected()) {
        socketClient->sendMessage(message);
        addMessageToChat(usernameInput->text(), QString::fromStdString(get_timestamp()), message);
    } else if (currentMode == SHM_MODE && shmClient->isConnected()) {
        shmClient->sendMessage(message);
        addMessageToChat(usernameInput->text(), QString::fromStdString(get_timestamp()), message);
    }
    
    messageInput->clear();
}

void MainWindow::onMessageReceived(QString user, QString time, QString text) {
    addMessageToChat(user, time, text);
}

void MainWindow::onUserListUpdated(QStringList users) {
    usersList->clear();
    usersList->addItems(users);
}

void MainWindow::onConnectionStatusChanged(bool connected) {
    if (connected) {
        statusLabel->setText("Status: Connected");
        statusLabel->setStyleSheet("QLabel { background-color: #ccffcc; padding: 5px; }");
    } else {
        statusLabel->setText("Status: Disconnected");
        statusLabel->setStyleSheet("QLabel { background-color: #ffcccc; padding: 5px; }");
        
        if (!connectButton->isEnabled()) {
            QMessageBox::warning(this, "Connection Lost", "Connection to server was lost");
            onDisconnectClicked();
        }
    }
}

void MainWindow::onErrorOccurred(QString error) {
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::addMessageToChat(const QString& user, const QString& time, const QString& text) {
    QString html = QString("<div style='margin: 5px 0;'>"
                          "<b style='color: #0066cc;'>%1</b> "
                          "<span style='color: #666; font-size: 10px;'>%2</span><br>"
                          "<span>%3</span>"
                          "</div>")
                   .arg(user, time, text.toHtmlEscaped());
    
    chatDisplay->append(html);
    
    QScrollBar* scrollBar = chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
