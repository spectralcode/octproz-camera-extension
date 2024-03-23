#include "cameraextensionform.h"
#include "camerasettingsdialog.h"
#include "ui_cameraextensionform.h"

CameraExtensionForm::CameraExtensionForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraExtensionForm) {
	ui->setupUi(this);
	fillCameraComboBox();

	connect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this, &CameraExtensionForm::connectToSelectedCamera);
	connect(ui->toolButton_settings, &QToolButton::clicked, this, &CameraExtensionForm::openSettingsDialog);
	connect(ui->toolButton_reload, &QToolButton::clicked, this, &CameraExtensionForm::fillCameraComboBox);
	
	//connect to save changed CameraViewWidget settings
	connect(ui->widget_video, &CameraViewWidget::rotationAngleChanged, this, [this](qreal rotationAngle) {
		this->parameters.rotationAngle = rotationAngle;
		emit this->paramsChanged();
	});
	connect(ui->widget_video, &CameraViewWidget::currentCameraChanged, this, [this](QString cameraName) {
		this->parameters.selectedCamera = cameraName;
		emit this->paramsChanged();
	});	
}

CameraExtensionForm::~CameraExtensionForm() {
	delete ui;
}

void CameraExtensionForm::setSettings(QVariantMap settings){
	//apply settings to widgets
	QString cameraName = settings.value(CAMERA_SELECTION).toString();
	this->connectToCamera(cameraName);
	this->ui->widget_video->rotateAbsolute(settings.value(CAMERA_ROTATION_ANGLE).toDouble());
	
	//update gui to represent correct settings
	int index = this->ui->comboBox_camera->findData(cameraName);
	if (index != -1) {
		disconnect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraExtensionForm::connectToSelectedCamera);
		this->ui->comboBox_camera->setCurrentIndex(index);
		connect(ui->comboBox_camera, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraExtensionForm::connectToSelectedCamera);
	}
}

void CameraExtensionForm::getSettings(QVariantMap* settings) {
	settings->insert(CAMERA_SELECTION, this->parameters.selectedCamera);
	settings->insert(CAMERA_ROTATION_ANGLE, this->parameters.rotationAngle);
}

void CameraExtensionForm::openSettingsDialog() {
	QCamera* currentCamera = ui->widget_video->getCamera();
	if(currentCamera) {
		CameraSettingsDialog dialog(currentCamera, this);
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
			this->ui->widget_video->openCamera(cameraInfo);
			break;
		}
	}
}
