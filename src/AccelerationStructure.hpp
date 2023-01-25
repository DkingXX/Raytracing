#ifndef __ACCELERATIONSTRUCTURE__
#define __ACCELERATIONSTRUCTURE__

#include <Eigen/Dense>
#include <cmath>
#include "box.hpp"
#include <vector>
#include <queue>
#include <tucano/mesh.hpp>
#include <time.h>

class AccelerationStructure {
	std::queue<Box> waitingList;
	std::vector<Box> boxes;
	Tucano::Mesh mesh;
	int maxFacesPerBox;
	float maxOverlap;
	float computedOverlap;

public:
	AccelerationStructure() {}

	AccelerationStructure(Tucano::Mesh &_mesh, int _maxFacesPerBox, float _maxOverlap)
	{
		this->mesh = _mesh;
		this->maxFacesPerBox = _maxFacesPerBox;
		this->maxOverlap = _maxOverlap;
		this->computedOverlap = 0;
		waitingList.push(Box::generateBoundingBox(mesh));
		std::cout << std::endl << "<CALCULATING ACCELERATION STRUCTURE>" << std::endl;
		std::cout << "Max overlap allowed: " << maxOverlap * 100 << "%" << std::endl;
		clock_t timeStart = clock();
		split();
		clock_t timeEnd = clock();
		std::cout << "Accelleration structure: 100% | Splitting time: " << (float)((timeEnd - timeStart) / CLOCKS_PER_SEC) << " seconds" << std::endl;
		std::cout << "Total Bounding boxes created: " << boxes.size() << std::endl;
		std::cout << "Max overlap: " << computedOverlap * 100 << "%" << std::endl;
	}

	void split() {
		
		Box primary;
		
		while (!waitingList.empty()) {
			primary = waitingList.front();
			waitingList.pop();
			if (primary.face_indexs.size() > maxFacesPerBox) {
				Box b1;
				Box b2;
				float midpoint;
				int axe = primary.getLongestAxe();
				switch (axe) {
				case 0: // Split along x
					midpoint = (primary.max.x() - primary.min.x()) / 2;
					b1 = Box(Eigen::Vector3f(primary.min.x(), primary.min.y(), primary.min.z()), Eigen::Vector3f(primary.max.x() - midpoint, primary.max.y(), primary.max.z()));
					b2 = Box(Eigen::Vector3f(primary.min.x() + midpoint, primary.min.y(), primary.min.z()), Eigen::Vector3f(primary.max.x(), primary.max.y(), primary.max.z()));
					primary.setSplitted(0);
					break;
				case 1: // Split along y
					midpoint = (primary.max.y() - primary.min.y()) / 2;
					b1 = Box(Eigen::Vector3f(primary.min.x(), primary.min.y(), primary.min.z()), Eigen::Vector3f(primary.max.x(), primary.max.y() - midpoint, primary.max.z()));
					b2 = Box(Eigen::Vector3f(primary.min.x(), primary.min.y() + midpoint, primary.min.z()), Eigen::Vector3f(primary.max.x(), primary.max.y(), primary.max.z()));
					primary.setSplitted(1);
					break;
				case 2: // Split along z
					midpoint = (primary.max.z() - primary.min.z()) / 2;
					b1 = Box(Eigen::Vector3f(primary.min.x(), primary.min.y(), primary.min.z()), Eigen::Vector3f(primary.max.x(), primary.max.y(), primary.max.z() - midpoint));
					b2 = Box(Eigen::Vector3f(primary.min.x(), primary.min.y(), primary.min.z() + midpoint), Eigen::Vector3f(primary.max.x(), primary.max.y(), primary.max.z()));
					primary.setSplitted(2);
					break;
				case 3: // already splitted on all axis
					boxes.push_back(primary);
					continue;
				default:
					continue;
				}

				for (int i = 0; i < primary.face_indexs.size(); i++) {
					float d1 = b1.verticesInBox(primary.face_indexs[i], mesh);
					float d2 = b2.verticesInBox(primary.face_indexs[i], mesh);
					if (d1 > d2) {
						b1.addFace(primary.face_indexs[i],  mesh);
					}
					else {
						b2.addFace(primary.face_indexs[i], mesh);
					}
				}

				b1.computeBoundigBox(mesh);
				b2.computeBoundigBox(mesh);
				float deltaOverlap = Box::overlapAreaPercent(b1, b2);
				// we tried to split along one axe but the overlap was too high,
				// so we push it back into the splitting queue and we change axe
				// in case the split result of the same size of the parent, push back the
				// parent because it has updated the splitting axe bool vector.
				if (deltaOverlap < maxOverlap) {
					if (deltaOverlap > computedOverlap) computedOverlap = deltaOverlap;
					if (b1.face_indexs.size() > 0) {
						if(b1.face_indexs.size() == primary.face_indexs.size()) waitingList.push(primary);
						else waitingList.push(b1); 
					}
					if (b2.face_indexs.size() > 0) { 
						if (b2.face_indexs.size() == primary.face_indexs.size()) waitingList.push(primary);
						else waitingList.push(b2);
					}
				}
				else {
					waitingList.push(primary);
				}
				
			}
			else {
				boxes.push_back(primary);
			}
		}
	}
	vector<Tucano::Shapes::Box> getBoxMesh() {
		vector<Tucano::Shapes::Box> result;
		for (int i = 0; i < this->boxes.size(); i++) {
			Tucano::Shapes::Box b = Tucano::Shapes::Box(boxes[i].getWidth(), boxes[i].getHeight(), boxes[i].getDepth());
			b.resetModelMatrix();
			b.modelMatrix()->translate(boxes[i].getBoxCenter());
			b.setColor(Eigen::Vector4f(0.1, 0.1, 1.0, 0.1));
			result.push_back(b);
		}
		return result;
	}

	vector<int> intersectAccelStruct(Eigen::Vector3f rayDirection, Eigen::Vector3f origin) {
		vector<int> faces;
		for (Box b : boxes) {
			if (b.intersect(rayDirection, origin)) {
				for (int face : b.face_indexs) {
					faces.push_back(face);
				}
			}
		}
		return faces;
	}

};

#endif // ACCELERATIONSTRUCTURE