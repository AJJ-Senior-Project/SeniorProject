#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <windows.h>
#include <psapi.h>
#include <QStringList>
#include <QCameraDevice>
#include <QDebug>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
    , ndiReceiver(new NDIReceiver())
    , ndiSender(new NDISender())
    , ndiSendCombined(nullptr)
    , combineAndSendTimer(nullptr)
    , previewScene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    // Initialize NDI library
    if (!NDIlib_initialize()) {
        qWarning() << "Failed to initialize NDI library.";
    }

    // Set up the graphicsViews
    graphicsViews["graphicsView_3"] = ui->graphicsView_3;
    graphicsViews["graphicsView_4"] = ui->graphicsView_4;
    graphicsViews["graphicsView_5"] = ui->graphicsView_5;
    graphicsViews["graphicsView_6"] = ui->graphicsView_6;
    graphicsViews["graphicsView_7"] = ui->graphicsView_7;
    graphicsViews["graphicsView_8"] = ui->graphicsView_8;
    graphicsViews["graphicsView_9"] = ui->graphicsView_9;
    graphicsViews["graphicsView_10"] = ui->graphicsView_10;
    graphicsViews["graphicsView_11"] = ui->graphicsView_11; // For additional 10th view

    // Initialize scenes
    scenes["graphicsView_3"] = new QGraphicsScene(this);
    scenes["graphicsView_4"] = new QGraphicsScene(this);
    scenes["graphicsView_5"] = new QGraphicsScene(this);
    scenes["graphicsView_6"] = new QGraphicsScene(this);
    scenes["graphicsView_7"] = new QGraphicsScene(this);
    scenes["graphicsView_8"] = new QGraphicsScene(this);
    scenes["graphicsView_9"] = new QGraphicsScene(this);
    scenes["graphicsView_10"] = new QGraphicsScene(this);
    scenes["graphicsView_11"] = new QGraphicsScene(this); // For additional 10th view

    ui->graphicsView_3->setScene(scenes["graphicsView_3"]);
    ui->graphicsView_4->setScene(scenes["graphicsView_4"]);
    ui->graphicsView_5->setScene(scenes["graphicsView_5"]);
    ui->graphicsView_6->setScene(scenes["graphicsView_6"]);
    ui->graphicsView_7->setScene(scenes["graphicsView_7"]);
    ui->graphicsView_8->setScene(scenes["graphicsView_8"]);
    ui->graphicsView_9->setScene(scenes["graphicsView_9"]);
    ui->graphicsView_10->setScene(scenes["graphicsView_10"]);
    ui->graphicsView_11->setScene(scenes["graphicsView_11"]); // For additional 10th view

    // Install event filters on graphics views
    foreach (QGraphicsView *view, graphicsViews.values()) {
        view->installEventFilter(this);
    }

    // Set up the preview scene for graphicsView_2
    ui->graphicsView_2->setScene(previewScene);

    // Populate the source combo boxes
    populateApplicationSources();
    populateCameraSources();
    populateAudioSources();

    QVBoxLayout *senderDropdown = ui->verticalLayout_2;
    senderDropdown->setSpacing(1);
    senderDropdown->setContentsMargins(0, 0, 0, 0);

    // Connect signals from NDIReceiver to the UI slots
    connect(ndiReceiver, &NDIReceiver::sourcesDiscovered, this, &mainpage::updateAvailableSources);
    connect(ndiReceiver, &NDIReceiver::frameReceived, this, &mainpage::displayVideoFrame);
}

mainpage::~mainpage()
{
    if (ndiReceiver) {
        ndiReceiver->terminateReceiver();
        delete ndiReceiver;
        ndiReceiver = nullptr;
    }
    if (ndiSender) {
        ndiSender->stopAll();
        delete ndiSender;
        ndiSender = nullptr;
    }

    if (combineAndSendTimer) {
        combineAndSendTimer->stop();
        delete combineAndSendTimer;
        combineAndSendTimer = nullptr;
    }

    if (ndiSendCombined) {
        NDIlib_send_destroy(ndiSendCombined);
        ndiSendCombined = nullptr;
    }

    NDIlib_destroy();

    delete ui;
}

void mainpage::on_selectSendButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void mainpage::on_selectReceiveButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    // Initialize the NDI receiver
    if (!ndiReceiver->initializeReceiver()) {
        qDebug() << "Failed to initialize NDI Receiver.";
        return;
    }

    // Start discovering sources
    ndiReceiver->startDiscovery();
}

void mainpage::on_senderBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainpage::on_receiverBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainpage::on_sourceComboBox_currentTextChanged(const QString &text)
{
    // Deprecated; we're now displaying all sources
}

void mainpage::updateAvailableSources(const QStringList &sources)
{
    ui->sourceComboBox->clear();
    ui->sourceComboBox->addItems(sources);

    // Clear existing mappings
    viewToSourceMap.clear();
    sourceScenes.clear();

    // Assign sources to graphicsViews
    int index = 0;
    QStringList graphicsViewNames = { "graphicsView_3", "graphicsView_4", "graphicsView_5", "graphicsView_6", "graphicsView_7", "graphicsView_8", "graphicsView_9", "graphicsView_10", "graphicsView_11" };
    qDebug() << "INDEX MAX " << graphicsViewNames.size();
    foreach (const QString &source, sources) {
        if (index < graphicsViewNames.size()) {
            QString viewName = graphicsViewNames[index];
            QGraphicsView *view = graphicsViews[viewName];

            sourceScenes[source] = scenes[viewName];
            viewToSourceMap[view] = source; // Map view to source

            ndiReceiver->selectSource(source); // Automatically start receiving from this source
            ++index;
        } else {
            // No more graphicsViews available
            break;
        }
    }

    // Update highlights in case sources have changed
    updateHighlights();
}

void mainpage::displayVideoFrame(const QString &sourceName, const QImage &frame)
{
    if (frame.isNull()) {
        qDebug() << "Received null image from source:" << sourceName;
        return;
    }

    if (!sourceScenes.contains(sourceName)) {
        qDebug() << "No scene assigned for source:" << sourceName;
        return;
    }

    QGraphicsScene *scene = sourceScenes[sourceName];
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(frame));

    // Get the corresponding graphicsView
    foreach (const QString &viewName, scenes.keys()) {
        if (scenes[viewName] == scene) {
            QGraphicsView *view = graphicsViews[viewName];
            view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
            break;
        }
    }

    // Store frames for selected sources
    frameMutex.lock();
    if (sourceName == primarySourceName) {
        primaryFrame = frame.copy();
    } else if (sourceName == secondarySourceName) {
        secondaryFrame = frame.copy();
    }
    frameMutex.unlock();
}

void mainpage::populateApplicationSources()
{
    QStringList applications = getRunningApplications();
    ui->comboBox_3->addItems(applications);
}

QStringList mainpage::getRunningApplications()
{
    QStringList appList;

    // Using EnumWindows to get list of running applications
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {
            TCHAR windowTitle[MAX_PATH];
            GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));
            if (wcslen(windowTitle) > 0) {
                auto appList = reinterpret_cast<QStringList*>(lParam);
                appList->append(QString::fromWCharArray(windowTitle));
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&appList));

    return appList;
}

void mainpage::populateCameraSources()
{
    auto cameras = QMediaDevices::videoInputs(); // List of available video cameras
    for (const auto &camera : cameras) {
        ui->comboBox_2->addItem(camera.description(), QVariant::fromValue(camera.id()));
    }
}

void mainpage::populateAudioSources()
{
    auto audioInputs = QMediaDevices::audioInputs(); // List of available audio inputs
    for (const QAudioDevice &audio : audioInputs) {
        QString deviceName = audio.description();
        ui->comboBox->addItem(deviceName, QVariant::fromValue(audio.id()));
    }
}

void mainpage::on_sendSignalButton_clicked()
{
    QString selectedApplication = ui->comboBox_3->currentText();
    QString selectedCameraId = ui->comboBox_2->currentData().toString();
    QString selectedAudioId = ui->comboBox->currentData().toString();

    if (!ndiSender->initializeNDI()) {
        qDebug() << "Failed to initialize NDI sender.";
        return;
    }

    ndiSender->sendMessage("Starting stream", 1, selectedApplication);

    // Start all streams
    ndiSender->startAllStreams(selectedCameraId, selectedAudioId, selectedApplication);

    qDebug() << "NDI streams started with selected options.";
}

bool mainpage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QGraphicsView *view = qobject_cast<QGraphicsView*>(watched);
        if (view && viewToSourceMap.contains(view)) {
            onGraphicsViewClicked(viewToSourceMap[view]);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void mainpage::onGraphicsViewClicked(const QString &sourceName)
{
    if (primarySourceName.isEmpty()) {
        primarySourceName = sourceName;
        qDebug() << "Primary source selected:" << primarySourceName;
    } else if (secondarySourceName.isEmpty()) {
        if (sourceName != primarySourceName) {
            secondarySourceName = sourceName;
            qDebug() << "Secondary source selected:" << secondarySourceName;
            // Both sources selected, start combining frames
            startCombiningFrames();
        }
    } else {
        // Reset selections if both already selected
        primarySourceName = sourceName;
        secondarySourceName.clear();
        qDebug() << "Primary source re-selected:" << primarySourceName;
    }

    // Update the highlights on the graphics views
    updateHighlights();
}

void mainpage::updateHighlights()
{
    foreach (QGraphicsView *view, graphicsViews.values()) {
        QString sourceName = viewToSourceMap.value(view);
        if (sourceName == primarySourceName) {
            // Apply red border
            view->setStyleSheet("border: 3px solid red;");
        } else if (sourceName == secondarySourceName) {
            // Apply blue border
            view->setStyleSheet("border: 3px solid blue;");
        } else {
            // Remove border
            view->setStyleSheet("");
        }
    }
}

void mainpage::startCombiningFrames()
{
    if (!combineAndSendTimer) {
        combineAndSendTimer = new QTimer(this);
        connect(combineAndSendTimer, &QTimer::timeout, this, &mainpage::combineAndSendFrames);
    }
    // Initialize NDI sender for combined frames
    if (!ndiSendCombined) {
        NDIlib_send_create_t sendDesc = {};
        sendDesc.p_ndi_name = "NDIReceiver";
        ndiSendCombined = NDIlib_send_create(&sendDesc);
        if (!ndiSendCombined) {
            qWarning() << "Failed to create NDI sender for combined frames.";
            return;
        }
    }
    combineAndSendTimer->start(33); // Approximately 30 fps
}

void mainpage::combineAndSendFrames()
{
    frameMutex.lock();
    if (primaryFrame.isNull()) {
        frameMutex.unlock();
        qDebug() << "Primary frame is null.";
        return;
    }

    QImage combinedImage = primaryFrame.copy();

    if (!secondaryFrame.isNull()) {
        QPainter painter(&combinedImage);
        int pipWidth = combinedImage.width() / 4;
        int pipHeight = combinedImage.height() / 4;
        int pipX = combinedImage.width() - pipWidth - 10;
        int pipY = combinedImage.height() - pipHeight - 10;

        QImage scaledSecondary = secondaryFrame.scaled(pipWidth, pipHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawImage(pipX, pipY, scaledSecondary);
    }
    frameMutex.unlock();

    // Display the combined image in graphicsView_2 (Preview)
    previewScene->clear();
    previewScene->addPixmap(QPixmap::fromImage(combinedImage));
    ui->graphicsView_2->fitInView(previewScene->itemsBoundingRect(), Qt::KeepAspectRatio);

    // Convert image to ARGB32 format if necessary
    if (combinedImage.format() != QImage::Format_ARGB32) {
        combinedImage = combinedImage.convertToFormat(QImage::Format_ARGB32);
    }

    // Prepare NDI video frame
    QByteArray imageData((const char*)combinedImage.bits(), combinedImage.sizeInBytes());

    NDIlib_video_frame_v2_t ndiFrame;
    ndiFrame.xres = combinedImage.width();
    ndiFrame.yres = combinedImage.height();
    ndiFrame.FourCC = NDIlib_FourCC_type_BGRA;
    ndiFrame.frame_rate_N = 30000;
    ndiFrame.frame_rate_D = 1001;
    ndiFrame.picture_aspect_ratio = static_cast<float>(combinedImage.width()) / static_cast<float>(combinedImage.height());
    ndiFrame.line_stride_in_bytes = combinedImage.bytesPerLine();
    ndiFrame.p_data = reinterpret_cast<uint8_t*>(imageData.data());

    NDIlib_send_send_video_v2(ndiSendCombined, &ndiFrame);
}
