#ifndef CAMERASETTINGSDIALOG_H
#define CAMERASETTINGSDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QCameraInfo>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCameraImageProcessingControl>
#include <QDebug>
#include <QMetaEnum>


class CameraSettingsDialog : public QDialog {
	Q_OBJECT

public:
	explicit CameraSettingsDialog(QCamera *camera, QWidget *parent = nullptr);
	~CameraSettingsDialog();

private:
	QCamera* camera;
	QVBoxLayout* layout;
	
	void setupUi();
	void addCameraImageProcessingControl(const QString &labelText, QCameraImageProcessingControl::ProcessingParameter param);
	void addColorFilterControl();
	void addPixelFormatControl();
	void addResolutionAndFpsControl();
	QString pixelFormatToString(QVideoFrame::PixelFormat format);
	void addZoomControl();
	void standardizeLabelSize(QLabel *label);
};

#endif // CAMERASETTINGSDIALOG_H