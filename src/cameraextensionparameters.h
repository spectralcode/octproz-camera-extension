#ifndef CAMERAEXTENSIONPARAMETERS_H
#define CAMERAEXTENSIONPARAMETERS_H

#include <QString>
#include <QtGlobal>
#include <QMetaType>

#define CAMERA_SELECTION "selected_camera"
#define CAMERA_ROTATION_ANGLE "camera_rotation_angle"


struct CameraExtensionParameters {
	QString selectedCamera;
	qreal rotationAngle;
};
Q_DECLARE_METATYPE(CameraExtensionParameters)



#endif // CAMERAEXTENSIONPARAMETERS_H


