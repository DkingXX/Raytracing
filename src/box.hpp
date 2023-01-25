#ifndef __AABB__
#define __AABB__

#include <Eigen/Dense>
#include <cmath>
#include <tucano/mesh.hpp>

class Box {
public:
	Eigen::Vector3f min, max;
	vector<int> face_indexs;
	bool alreadySplitted[3]; // 0 -> x, 1 -> y, 2 -> z
	bool null = true;
	int left = -1;
	int right = -1;


	Box(Eigen::Vector3f _min, Eigen::Vector3f _max, vector<int> _face_indexs)
	{
		min = _min;
		max = _max;
		face_indexs = _face_indexs;
		alreadySplitted[0] = false;
		alreadySplitted[1] = false;
		alreadySplitted[2] = false;
		this->null = false;
	}

	Box(Eigen::Vector3f _min, Eigen::Vector3f _max)
	{
		min = _min;
		max = _max;
		alreadySplitted[0] = false;
		alreadySplitted[1] = false;
		alreadySplitted[2] = false;
		this->null = false;
	}

	Box() { }

	// Compute a ray-box intersection
	bool intersect(Eigen::Vector3f& rayDirection, Eigen::Vector3f& origin) {

		float tXmin = (min.x() - origin.x()) / rayDirection.x();
		float tXmax = (max.x() - origin.x()) / rayDirection.x();

		if (tXmin > tXmax) swap(tXmin, tXmax);

		float tYmin = (min.y() - origin.y()) / rayDirection.y();
		float tYmax = (max.y() - origin.y()) / rayDirection.y();

		if (tYmin > tYmax) swap(tYmin, tYmax);

		if ((tXmin > tYmax) || (tYmin > tXmax))
			return false;

		if (tYmin > tXmin)
			tXmin = tYmin;

		if (tYmax < tXmax)
			tXmax = tYmax;

		float tZmin = (min.z() - origin.z()) / rayDirection.z();
		float tZmax = (max.z() - origin.z()) / rayDirection.z();

		if (tZmin > tZmax) swap(tZmin, tZmax);

		if ((tXmin > tZmax) || (tZmin > tXmax))
			return false;

		if (tZmin > tXmin)
			tXmin = tZmin;

		if (tZmax < tXmax)
			tXmax = tZmax;
		return true;
	}

	static Box generateBoundingBox(Tucano::Mesh mesh) {
		float xMin = FLT_MAX;
		float yMin = FLT_MAX;
		float zMin = FLT_MAX;
		float xMax = FLT_MIN;
		float yMax = FLT_MIN;
		float zMax = FLT_MIN;

		for (int i = 0; i < mesh.getNumberOfVertices(); i++) {
			
			Eigen::Vector3f v = mesh.getShapeModelMatrix() * mesh.getVertex(i).head<3>();
			//x
			if (v.x() > xMax) xMax = v.x();
			if (v.x() < xMin) xMin = v.x();
			//y
			if (v.y() > yMax) yMax = v.y();
			if (v.y() < yMin) yMin = v.y();
			//z
			if (v.z() > zMax) zMax = v.z();
			if (v.z() < zMin) zMin = v.z();
		}
		
		vector<int> f;
		for (int i = 0; i < mesh.getNumberOfFaces(); i++) {
			f.push_back(i);
		}

		Box b = Box(Eigen::Vector3f(xMin, yMin, zMin), Eigen::Vector3f(xMax, yMax, zMax), f);
		return b;
	}

	Eigen::Vector3f getBoxCenter() {
		float w = (min.x() + max.x()) / 2.0f;
		float h = (min.y() + max.y()) / 2.0f;
		float d = (min.z() + max.z()) / 2.0f;
		return Eigen::Vector3f(w, h, d);
	}

	float getWidth() { return max.x() - min.x(); }
	float getHeight() { return max.y() - min.y(); }
	float getDepth() { return max.z() - min.z(); }

	void offsetBox(Eigen::Vector3f offset) {
		this->min = this->min - offset;
		this->max = this->max - offset;
	}

	void scaleBox(float scale) {
		this->min = this->min * scale;
		this->max = this->max * scale;
	}

	float getVolume() {
		Eigen::Vector3f absLengths = this->max - this->min;
		return absLengths.x() * absLengths.y() * absLengths.z();
	}

	void addFace(int face, Tucano::Mesh &mesh) {
		this->face_indexs.push_back(face);
		Tucano::Face f = mesh.getFace(face);
		Eigen::Vector3f v0 = mesh.getShapeModelMatrix() * mesh.getVertex(f.vertex_ids[0]).head<3>();
		Eigen::Vector3f v1 = mesh.getShapeModelMatrix() * mesh.getVertex(f.vertex_ids[1]).head<3>();
		Eigen::Vector3f v2 = mesh.getShapeModelMatrix() * mesh.getVertex(f.vertex_ids[2]).head<3>();
		
		//computeResize(v0);
		//computeResize(v1);
		//computeResize(v2);
	}

	void computeResize(Eigen::Vector3f v) {
		if (v.x() > max.x()) max[0] = v.x();
		if (v.x() < min.x()) min[0] = v.x();
		//y
		if (v.y() > max.y()) max[1] = v.y();
		if (v.y() < min.y()) min[1] = v.y();
		//z
		if (v.z() > max.z()) max[2] = v.z();
		if (v.z() < min.z()) min[2] = v.z();
	}

	int verticesInBox(int face, Tucano::Mesh& mesh) {
		int count = 0;
		Tucano::Face f = mesh.getFace(face);
		for (int i = 0; i < f.vertex_ids.size(); i++) {
			Eigen::Vector3f v = (mesh.getShapeModelMatrix() * mesh.getVertex(f.vertex_ids[i])).head<3>();
			if (inBox(v)) count++;
		}
		return count;
	}
	bool inBox(Eigen::Vector3f v) {
		return min.x() <= v.x() && v.x() <= max.x()
			&& min.y() <= v.y() && v.y() <= max.y()
			&& min.z() <= v.z() && v.z() <= max.z();
	}


	float checkDistance(int index, Tucano::Mesh &mesh) {
		Tucano::Face face = mesh.getFace(index);
		Eigen::Vector3f cb = this->getBoxCenter();

		Eigen::Vector3f ct = mesh.getShapeModelMatrix() * mesh.getVertex(face.vertex_ids[0]).head<3>();
		ct += mesh.getShapeModelMatrix() * mesh.getVertex(face.vertex_ids[1]).head<3>();
		ct += mesh.getShapeModelMatrix() * mesh.getVertex(face.vertex_ids[2]).head<3>();
		ct /= 3;

		return  sqrt(pow(cb.x() - ct.x(), 2) + pow(cb.y() - ct.y(), 2) + pow(cb.z() - ct.z(), 2));
	}

	int getLongestAxe() {
		float xLength = max.x() - min.x();
		float yLength = max.y() - min.y();
		float zLength = max.z() - min.z();
		// 0 -> x, 1 -> y, 2 -> z, 3 -> already splitted on all axis
		if (this->alreadySplitted[0] && this->alreadySplitted[1] && this->alreadySplitted[2]) return 3;
		if (this->alreadySplitted[0] && this->alreadySplitted[1]) return 2;
		if (this->alreadySplitted[0] && this->alreadySplitted[2]) return 1;
		if (this->alreadySplitted[1] && this->alreadySplitted[2]) return 0;
		if (this->alreadySplitted[0]) {
			if (yLength >= zLength) return 1;
			else return 2;
		}
		if (this->alreadySplitted[1]) {
			if (xLength >= zLength) return 0;
			else return 2;
		}
		if (this->alreadySplitted[2]) {
			if (xLength >= yLength) return 0;
			else return 1;
		}
		if (xLength >= yLength && xLength >= zLength) return 0;
		if (yLength >= xLength && yLength >= zLength) return 1;
		if (zLength >= xLength && zLength >= yLength) return 2;
	}

	void computeBoundigBox(Tucano::Mesh &mesh) {
		float xMin = FLT_MAX;
		float yMin = FLT_MAX;
		float zMin = FLT_MAX;
		float xMax = FLT_MIN;
		float yMax = FLT_MIN;
		float zMax = FLT_MIN;

		for (int i = 0; i < face_indexs.size(); i++) {
			Tucano::Face face = mesh.getFace(face_indexs[i]);
			for (int j = 0; j < face.vertex_ids.size(); j++) {
				Eigen::Vector3f v = mesh.getShapeModelMatrix() * mesh.getVertex(face.vertex_ids[j]).head<3>();
				//x
				if (v.x() > xMax) xMax = v.x();
				if (v.x() < xMin) xMin = v.x();
				//y
				if (v.y() > yMax) yMax = v.y();
				if (v.y() < yMin) yMin = v.y();
				//z
				if (v.z() > zMax) zMax = v.z();
				if (v.z() < zMin) zMin = v.z();
			}
			
		}

		this->min = Eigen::Vector3f(xMin, yMin, zMin);
		this->max = Eigen::Vector3f(xMax, yMax, zMax);
	}

	static float overlapAreaPercent(Box a, Box b) {
		if (a.max.x() < b.min.x() || a.min.x() > b.max.x()) return 0;
		if (a.max.y() < b.min.y() || b.min.y() > a.max.y()) return 0;
		if (a.max.z() < b.min.z() || b.min.z() > a.max.z()) return 0;

		float areaOverlap =
				(std::min(a.max.x(), b.max.x()) - std::max(a.min.x(), b.min.x())) *
				(std::min(a.max.y(), b.max.y()) - std::max(a.min.y(), b.min.y())) *
				(std::min(a.max.z(), b.max.z()) - std::max(a.min.z(), b.min.z()));


		return areaOverlap / (a.getVolume() + b.getVolume() - areaOverlap);
	}

	void setSplitted(int i) {
		this->alreadySplitted[i] = true;
	}

	int getSplittingAxe() {
		if (!this->alreadySplitted[0]) return 0;
		if (!this->alreadySplitted[1]) return 1;
		if (!this->alreadySplitted[2]) return 2;
	}

	int longestAxis() {
		float w = getWidth(), h = getHeight(), d = getDepth();
		if (w >= h && w >= d)	return 0;	// X-axis
		else if (h >= w && h >= d)	return 1;	// Y-axis
		else						return 2;	// Z-axis
	}

};

#endif // AABB