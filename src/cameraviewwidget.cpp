#include "cameraviewwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QRubberBand>
#include <QGraphicsProxyWidget>
#include <QCamera>
#include <QCameraInfo>
#include <QTimer>
#include <QCameraImageCapture>
#include <QDateTime>


CameraViewWidget::CameraViewWidget(QWidget *parent)
	: QGraphicsView(parent),
	  camera(nullptr),
	  scene(new QGraphicsScene(this)),
	  videoWidget(new QGraphicsVideoItem()),
	  oldRotationAngle(0.0),
	  isFirstShowEvent(true)
{
	this->setScene(scene);
	this->scene->addItem(videoWidget);
}

CameraViewWidget::~CameraViewWidget() {
	if(this->camera) {
		this->camera->stop();
		delete this->camera;
	}
	if(this->videoWidget){
		delete this->videoWidget;
	}
}

qreal CameraViewWidget::getRotationAngle() {
	return this->oldRotationAngle;
}

void CameraViewWidget::showEvent(QShowEvent *event) {
	QGraphicsView::showEvent(event);
	this->rotateAbsolute(this->oldRotationAngle);
	if(!this->currentCamera.isNull()){
		this->openCamera(this->currentCamera);
	}else{
		this->openCamera(QCameraInfo::defaultCamera());
	}
	if(this->isFirstShowEvent){
		//executing fitCameraViewToWindow immediately and then via singleShot to ensure accurate fitting, 
		//as the first call alone doesn't always adjust the view correctly due to pending. singleShot(0) executed as soon as all events are processed
		this->fitCameraViewToWindow();
		QTimer::singleShot(0, this, &CameraViewWidget::fitCameraViewToWindow);
		this->isFirstShowEvent = false;
	}
}

void CameraViewWidget::hideEvent(QHideEvent *event) {
	QGraphicsView::hideEvent(event);
	if (this->camera) {
		this->camera->stop();
		delete this->camera;
		this->camera = nullptr;
	}
}

void CameraViewWidget::mouseDoubleClickEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->fitCameraViewToWindow();
	}
	if (event->button() == Qt::MidButton) {
		this->rotateAbsolute(0);
	}
	if (event->button() == Qt::RightButton) {
		this->takeSnapshot();
	}
	QGraphicsView::mouseDoubleClickEvent(event);
}

void CameraViewWidget::wheelEvent(QWheelEvent *event) {
	if(this->underMouse() && QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)){
		QPoint numPixels = event->pixelDelta();
		QPoint numDegrees = event->angleDelta()/8;
		qreal scaleValue = 0;
		if (!numPixels.isNull()) {
			scaleValue += numPixels.y()/30.0;
		} else if (!numDegrees.isNull()) {
			QPoint numSteps = numDegrees/15;
			scaleValue += (float)numSteps.y()/30.0;
		}
		this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		this->scale(1.0+scaleValue, 1.0+scaleValue);
		event->accept();
	}
	if(this->underMouse() && QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)){
		QPoint numPixels = event->pixelDelta();
		QPoint numDegrees = event->angleDelta()/8;

		qreal angle = this->oldRotationAngle;
		if (!numPixels.isNull()) {
			angle += numPixels.y()/30.0;
		} else if (!numDegrees.isNull()) {
			QPoint numSteps = numDegrees/15;
			angle += (float)numSteps.y()/30.0;
		}
		this->rotateAbsolute(angle);
		event->accept();
	}
}

void CameraViewWidget::keyPressEvent(QKeyEvent* event) {
	if((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_S)){
		this->takeSnapshot();
	} else {
		QGraphicsView::keyPressEvent(event);
	}
}

void CameraViewWidget::fitCameraViewToWindow() {
	this->fitInView(this->videoWidget->boundingRect(), Qt::KeepAspectRatio);
	this->ensureVisible(this->videoWidget->boundingRect());
	this->centerOn(this->videoWidget);
	this->scene->setSceneRect(this->scene->itemsBoundingRect());
}

void CameraViewWidget::openCamera(const QCameraInfo& cameraInfo) {
	if (cameraInfo.isNull()) {
		emit error(tr("Camera not available!"));
		return;
	}
	
	//do nothing if camera is already selected and running
	if(this->currentCamera == cameraInfo && this->camera && this->camera->status() == QCamera::ActiveStatus){
		return;
	}
	
	//stop and delete camera to be able to create newly selected camera
	if (this->camera) {
		this->camera->stop();
		delete camera;
	}
	
	//create new camera and start live view
	this->camera = new QCamera(cameraInfo, this);
	this->camera->setViewfinder(this->videoWidget);
	this->camera->start();
	
	//remember current camera selection
	this->currentCamera = cameraInfo;
	emit currentCameraChanged(this->currentCamera.deviceName());
}

void CameraViewWidget::closeCamera() {
	this->camera->stop();
}

void CameraViewWidget::rotateAbsolute(qreal angle) {
	this->rotate(this->oldRotationAngle-angle);
	this->oldRotationAngle = angle;
	emit rotationAngleChanged(this->oldRotationAngle);
}

void CameraViewWidget::takeSnapshot() {
	if(!camera || camera->status() != QCamera::ActiveStatus){
		return;
	}
	QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
	connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &CameraViewWidget::saveSnapshot);
	imageCapture->capture();
}

void CameraViewWidget::saveSnapshot(int id, const QImage &image) {
	Q_UNUSED(id);
	QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_snapshot" + ".png";
	if (image.save(fileName))
		qDebug() << "Snapshot saved as" << fileName;
	else
		qDebug() << "Failed to save snapshot";
}
