/*
 * IRTDImageProcessor.cpp
 *
 *  Created on: 2016. szept. 9.
 *      Author: M�t�
 */


#include "IRTDImageProcessor.h"
#include <chrono>
#include <opencv2/imgproc.hpp>

template<typename CONFIG>

void IRTDImageProcessor<CONFIG>::startLedController() {
	while (true) {
		auto time = std::chrono::steady_clock::now();
		auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

		if (currentTimestamp >= ledController.getLastTimestamp() + ledController.getDuration()) {
			ledController.flashNext(currentTimestamp, duration);
		}
	}
}

template<typename CONFIG>
std::vector<Cluster> IRTDImageProcessor<CONFIG>::clusterContours(std::vector<std::vector<cv::Point> > contours, int distanceThreshold) {
	std::vector<Cluster> clusters;

	std::vector<ContourPair> paired;

	if (contours.size() > 1) {

		for (size_t i = 0; i < contours.size() - 1; i++) {
			for (size_t j = i + 1; j < contours.size(); j++) {
				cv::Point2f center1, center2;
				float r1, r2;

				cv::minEnclosingCircle(contours[i], center1, r1);
				cv::minEnclosingCircle(contours[j], center2, r2);

				Contour c1;
				Contour c2;

				c1.points = contours[i];
				c2.points = contours[j];

				c1.area = cv::contourArea(contours[i]);
				c2.area = cv::contourArea(contours[j]);

				ContourPair cpair;
				cpair.c1 = c1;
				cpair.c2 = c2;

				cpair.distance = (sqrtf(
					powf(center1.x - center2.x, 2.0)
					+ powf(center1.y - center2.y, 2.0))) - r1 - r2;

				paired.push_back(cpair);
			}
		}

		std::sort(paired.begin(), paired.end(),
			[](ContourPair a, ContourPair b) {
			return a.distance < b.distance;
		});

		for (auto p : paired) {
			if (p.distance < distanceThreshold) {
				auto c1 = p.c1;
				auto c2 = p.c2;

				int c1Index = -1;
				int c2Index = -1;

				int i = 0;
				for (auto& c : clusters) {
					if (std::find(c.contours.begin(), c.contours.end(), c1)
						!= c.contours.end()) {
						c1Index = i;
					}
					if (std::find(c.contours.begin(), c.contours.end(), c2)
						!= c.contours.end()) {
						c2Index = i;
					}
					i++;
				}

				if (c1Index == -1 && c2Index == -1) {
					Cluster temp;
					temp.contours.push_back(c1);
					temp.contours.push_back(c2);
					clusters.push_back(temp);
				}
				else if (c1Index != c2Index) {
					if (c1Index == -1 && c2Index != -1) {
						clusters[c2Index].contours.push_back(c1);
					}
					else if (c2Index == -1 && c1Index != -1) {
						clusters[c1Index].contours.push_back(c2);
					}
					else if (c1Index != -1 && c2Index != -1) {
						clusters[c1Index].contours.insert(
							clusters[c1Index].contours.begin(),
							clusters[c2Index].contours.begin(),
							clusters[c2Index].contours.end());

						clusters.erase(clusters.begin() + c2Index);
					}
				}

			}
		}

		for (auto c : contours) {
			bool found = false;
			for (auto& cluster : clusters) {
				for (auto& ref : cluster.contours) {
					if (c == ref.points) {
						found = true;
					}
				}
			}
			if (!found) {
				Contour temp;
				temp.area = cv::contourArea(c);
				temp.points = c;
				Cluster cluster;
				cluster.area = temp.area;
				cluster.contours.push_back(temp);
				clusters.push_back(cluster);
			}
		}


		for (auto& cluster : clusters) {

			std::vector<cv::Point> allPoints;
			cluster.area = 0;

			for (auto contour : cluster.contours) {
				allPoints.insert(allPoints.begin(), contour.points.begin(),
					contour.points.end());
				cluster.area += cv::contourArea(contour.points);
			}

			cluster.allPoints = allPoints;

			if (cluster.area > 10) {
				cluster.boundingRect = cv::boundingRect(allPoints);
			}
		}

	}

	return clusters;
}

template<typename CONFIG>
ImageProcessingData<CONFIG> IRTDImageProcessor<CONFIG>::process(Frame frame) {


	ImageProcessingData<CONFIG> foundMarkers;

	foundMarkers.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	foundMarkers.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());

	tbb::parallel_for(size_t(0), frame.images.size(), [&](size_t i) {

		auto iter = ledController.getFrameIteration(frame.timestamp, setupTime);

		tbb::concurrent_vector<cv::Point2f> markerPosition;
		tbb::concurrent_vector<int> 		markerIdentifier;

		cv::Mat thresholded;
		cv::threshold(frame.images[i], thresholded, threshold, 255, CV_THRESH_TOZERO);
		blur(thresholded, thresholded, cv::Size(3, 3));

		cv::Mat cont = thresholded.clone();
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(cont, contours, hierarchy, CV_RETR_EXTERNAL,
			cv::CHAIN_APPROX_SIMPLE);

		auto clusters = clusterContours(contours, 10);

		if (contours.size() == 1) {

			Cluster ctemp;
			ctemp.allPoints = contours[0];
			ctemp.area = cv::contourArea(contours[0]);
			ctemp.boundingRect = cv::boundingRect(contours[0]);

			Contour conttemp;
			conttemp.area = cv::contourArea(contours[0]);
			conttemp.clusterId = 0;
			conttemp.points = contours[0];

			ctemp.contours.push_back(conttemp);

			clusters.clear();
			clusters.push_back(ctemp);
		}

		if (contours.size() > 1) {
			for (auto& cluster : clusters) {
				double averageLuminosity = 0;

				if (cluster.area > 10) {
					for (int i = 0; i < cluster.boundingRect.width; i++) {
						for (int j = 0; j < cluster.boundingRect.height; j++) {

							averageLuminosity += (int)thresholded.at<uchar>(
								cluster.boundingRect.y + j,
								cluster.boundingRect.x + i);
						}
					}
					averageLuminosity /= cluster.area;

					if (iter >= 0) {
						BodyPart ankle;
						BodyPart hand;

						if (iter == 0) {
							ankle = LEFT_ANKLE;
							hand = LEFT_HAND;
						}
						else if (iter == 1) {
							ankle = RIGHT_ANKLE;
							hand = RIGHT_HAND;
						}
						markerPosition.push_back(cv::Point2f(cluster.boundingRect.x, cluster.boundingRect.y));
						if (averageLuminosity > 120) {
							markerIdentifier.push_back(ankle);
						}if (averageLuminosity < 100) {
							markerIdentifier.push_back(hand);
						}
					}
				}
			}
		}

		foundMarkers.data[i] = markerPosition;
		foundMarkers.identifiers[i] = markerIdentifier;
	});

	foundMarkers.frameIndex = frame.frameIndex;
	foundMarkers.timestamp = frame.timestamp;

	return foundMarkers;
}

template<typename CONFIG>
void IRTDImageProcessor<CONFIG>::reconfigure(boost::property_tree::ptree config) {
	threshold = config.get<int>(THRESHOLD);
	duration = config.get<int>(DURATION);
	setupTime = config.get<int>(SETUP_TIME);
}
