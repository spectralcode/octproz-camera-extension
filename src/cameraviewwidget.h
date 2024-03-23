#ifndef CAMERAVIEWWIDGET_H
#define CAMERAVIEWWIDGET_H

#include <QVideoWidget>
#include <QCameraInfo>
#include <QCamera>
#include <QMediaRecorder>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QGuiApplication>


class CameraViewWidget : public QGraphicsView
{
	Q_OBJECT
public:
	explicit CameraViewWidget(QWidget *parent = nullptr);
	~CameraViewWidget();
	
	qreal getRotationAngle();
	void rotateAbsolute(qreal angle);
	QCamera* getCamera() const { return camera; }

protected:
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent*event) override;

private:
	QCamera* camera;
	QGraphicsScene* scene;
	QVideoWidget* videoWidget;
	qreal oldRotationAngle;
	QCameraInfo currentCamera;
	bool isFirstShowEvent;

public slots:
	void fitCameraViewToWindow();
	void openCamera(const QCameraInfo& camera);
	void closeCamera();


signals:
	void error(QString);
	void info(QString);
	void rotationAngleChanged(qreal angle);
	void currentCameraChanged(QString cameraName);
};

#endif // CAMERAVIEWWIDGET_H
