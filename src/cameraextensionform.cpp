#include "cameraextensionform.h"
#include "camerasettingsdialog.h"
#include "ui_cameraextensionform.h"
#include <QTimer>


CameraExtensionForm::CameraExtensionForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraExtensionForm) {
	ui->setupUi(this);
	fillCameraComboBox();

	connect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this, &CameraExtensionForm::connectToSelectedCamera);
	connect(ui->toolButton_settings, &QToolButton::clicked, this, &CameraExtensionForm::openSettingsDialog);
	connect(ui->toolButton_reload, &QToolButton::clicked, this, &CameraExtensionForm::fillCameraComboBox);
	connect(ui->widget_video, &CameraViewWidget::info, this, &CameraExtensionForm::info);
	connect(ui->widget_video, &CameraViewWidget::error, this, &CameraExtensionForm::error);

	//connect to save changed CameraViewWidget settings
	connect(ui->widget_video, &CameraViewWidget::overlayStateChanged, this, &CameraExtensionForm::paramsChanged);
	connect(ui->widget_video, &CameraViewWidget::rotationAngleChanged, this, [this](qreal rotationAngle) {
		this->parameters.rotationAngle = rotationAngle;
		emit this->paramsChanged();
	});
	connect(ui->widget_video, &CameraViewWidget::currentCameraChanged, this, [this](QString cameraName) {
		this->parameters.selectedCamera = cameraName;
		emit this->paramsChanged();
	});	
	connect(ui->widget_video, &CameraViewWidget::snapshotDirChanged, this, [this](QString snapshotDir) {
		this->parameters.snapShotSavePath = snapshotDir;
		emit this->paramsChanged();
	});

	this->installEventFilter(this);
}

CameraExtensionForm::~CameraExtensionForm() {
	delete ui;
}

void CameraExtensionForm::setSettings(QVariantMap settings){
	//update parameters struct, use default values if empty
	this->parameters.selectedCamera = settings.value(CAMERA_SELECTION, "").toString();
	this->parameters.rotationAngle = settings.value(CAMERA_ROTATION_ANGLE, 0.0).toDouble();
	this->parameters.snapShotSavePath = settings.value(CAMERA_SNAPSHOT_SAVE_PATH, "").toString();
	this->parameters.windowState = settings.value(CAMERA_WINDOW_STATE).toByteArray();

	//apply parameters to widgets
	this->ui->widget_video->setSnapshotSaveDir(this->parameters.snapShotSavePath);
	this->ui->widget_video->rotateAbsolute(this->parameters.rotationAngle);
	this->connectToCamera(this->parameters.selectedCamera);

	//update gui to represent correct settings
	int index = this->ui->comboBox_camera->findData(this->parameters.selectedCamera);
	if (index != -1) {
		disconnect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraExtensionForm::connectToSelectedCamera);
		this->ui->comboBox_camera->setCurrentIndex(index);
		connect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraExtensionForm::connectToSelectedCamera);
	}

	//restore window geometry
	this->restoreGeometry(this->parameters.windowState);

	//update overlays
	auto overlays = this->ui->widget_video->getOverlays();
	for (const auto &overlay : overlays) {
		if (settings.contains(overlay.second + "_state")) {
			QVariantMap overlayState = settings.value(overlay.second + "_state").toMap();
			overlay.first->loadState(overlayState);
		}
	}
}

void CameraExtensionForm::getSettings(QVariantMap* settings) {
	settings->insert(CAMERA_SELECTION, this->parameters.selectedCamera);
	settings->insert(CAMERA_ROTATION_ANGLE, this->parameters.rotationAngle);
	settings->insert(CAMERA_SNAPSHOT_SAVE_PATH, this->parameters.snapShotSavePath);
	settings->insert(CAMERA_WINDOW_STATE, this->parameters.windowState);

	//save states of overlays
	auto overlays = this->ui->widget_video->getOverlays();
	for (auto &overlay : overlays) {
		settings->insert(overlay.second + "_state", overlay.first->saveState());
	}
}

bool CameraExtensionForm::eventFilter(QObject* watched, QEvent* event) {
	if (watched == this) {
		if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
			if (this->isVisible()) {
				this->parameters.windowState = this->saveGeometry();
				emit paramsChanged();
			}
		}
	}
	return QWidget::eventFilter(watched, event);
}

void CameraExtensionForm::openSettingsDialog() {
	QCamera* currentCamera = ui->widget_video->getCamera();
	QList<QCameraViewfinderSettings> supportedSettings = ui->widget_video->getSupportedSettings();
	if(currentCamera) {
		CameraSettingsDialog dialog(currentCamera, supportedSettings, this);
		dialog.exec();
	} else {
		qDebug() << "No camera is currently selected or available.";
	}
}

void CameraExtensionForm::connectToSelectedCamera() {
	QString selectedCamera = this->ui->comboBox_camera->currentData().toString();
	this->connectToCamera(selectedCamera);
}

void CameraExtensionForm::disconnectCurrentCamera() {
	this->ui->widget_video->closeCamera();
}

void CameraExtensionForm::fillCameraComboBox() {
	this->ui->comboBox_camera->clear();
	QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
	for(const QCameraInfo &cameraInfo : cameras) {
		this->ui->comboBox_camera->addItem(cameraInfo.description(), cameraInfo.deviceName());
	}
}

void CameraExtensionForm::connectToCamera(QString deviceName) {
	if(deviceName.isEmpty()){
		return;
	}
	QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
	for(const QCameraInfo &cameraInfo : cameras) {
		if (cameraInfo.deviceName() == deviceName) {
			if(!this->ui->widget_video->isVisible()){
				this->ui->widget_video->setCamera(cameraInfo);
			} else {
				QTimer::singleShot(0, this, [this, cameraInfo]() { this->ui->widget_video->openCamera(cameraInfo); });
			}
			break;
		}
	}
}
