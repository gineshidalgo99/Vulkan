#ifndef FACE_TRACKER_H
#define FACE_TRACKER_H

#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_io.h>
#include <iostream>
#include <fstream>
#include "Landmarks.h"

class DlibFaceTracker
{
public:
	DlibFaceTracker(const char* trainFaceModel);
	std::pair<bool, Landmarks> getFaceLandmarks(const dlib::array2d<dlib::bgr_pixel> & image, dlib::rectangle & previousFaceRect = dlib::rectangle{});
private:
	dlib::frontal_face_detector m_faceDetector;
	dlib::shape_predictor m_sp;
	dlib::pyramid_down<2> m_pyrDown2;

	void resizeRectangle(const int factor, dlib::rectangle & rectangle);
	dlib::rectangle keepHigherRectangle(const std::vector<dlib::rectangle> & rectangles);
};

#endif // FACE_TRACKER_H
