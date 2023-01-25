#ifndef __TREESTRUCTURE__
#define __TREESTRUCTURE__

#include <Eigen/Dense>
#include <cmath>
#include "box.hpp"
#include <tucano/mesh.hpp>


class Tree {
	Tucano::Mesh mesh;
	vector<Box> boxes;
	vector<vector<int>> faceBuckets;
	int size = 0;
	int threshold = 1;
	int facesPerBox = -1;

	// Returns the index of the left child of a node with index i.
	int childLeft(int i)	{ return 2 * i + 1; }

	// Returns the index of the right child of a node with index i.
	int childRight(int i)	{ return 2 * i + 2; }
public:
	Tree() {}

	Tree(Tucano::Mesh& _mesh, int _threshold, int _facesPerBox) {
		vector<int> meshFaces;
		this->mesh = _mesh;
		this->threshold = _threshold;
		this->size = pow(2, threshold) - 1;
		this->boxes.resize(size);
		this->faceBuckets.resize(size);
		this->facesPerBox = _facesPerBox;

		for (int i = 0; i < size; i++)	this->boxes[i] = Box();
		for (int j = 0; j < mesh.getNumberOfFaces(); j++) meshFaces.push_back(j);

		this->boxes[0] = Box::generateBoundingBox(mesh);
		this->faceBuckets[0] = meshFaces;
	}

	Box getBox(int i) {
		if (i >= 0 && i < size)
			return boxes[i];
	}

	Box getLeft(int i) {
		int c = boxes[i].left;
		if (c > -1)
			return boxes[c];
	}

	Box getRight(int i) {
		int c = boxes[i].right;
		if (c > -1)
			return boxes[c];
	}

	void addLeft(int i, Box b) {
		int c = childLeft(i);
		if (c >= 0 && c < size) {
			this->boxes[i].left = c;
			this->boxes[c] = b;
		}
	}

	void addRight(int i, Box b) {
		int c = childRight(i);
		if (c >= 0 && c < size)
			this->boxes[i].right = c;
			this->boxes[c] = b;
	}

	vector<int> getBucket(int i) {
		if (i >= 0 && i < size)
			return faceBuckets[i];
	}

	vector<int> getLeftBucket(int i) {
		int c = childLeft(i);
		if (c > -1)
			return faceBuckets[c];
	}

	vector<int> getRightBucket(int i) {
		int c = childRight(i);
		if (c > -1)
			return faceBuckets[c];
	}

	void addLeftBucket(int i, vector<int> b) {
		int c = childLeft(i);
		if (c >= 0 && c < size) {
			this->faceBuckets[c] = b;
		}
	}

	void addRightBucket(int i, vector<int> b) {
		int c = childRight(i);
		if (c >= 0 && c < size)
			this->faceBuckets[c] = b;
	}

	vector<int> traceTree(Eigen::Vector3f &rayDirection, Eigen::Vector3f &origin, int i, vector<int> &bucketList) {
		
	}

	void spliterator(int i) {
		split(i, threshold);
	}

	void split(int i, int k) {
		int leftIndex = (2 * i) + 1;
		vector<int> faces = faceBuckets[i];
		if (k <= 0 || leftIndex >= size || faces.size() < facesPerBox) return;
		//****************Box Split****************
		Box b = boxes[i];
		int splitAxis = b.longestAxis();
		float centerLongestAxis = b.getBoxCenter()[splitAxis];

		Eigen::Vector3f min = b.min;
		Eigen::Vector3f max = b.max;
		Eigen::Vector3f splitMin = b.min;
		Eigen::Vector3f splitMax = b.max;

		splitMin[splitAxis] = centerLongestAxis;
		splitMax[splitAxis] = centerLongestAxis;
		//*****************************************

		//***************Faces Split***************
		Eigen::Affine3f shapeModelMatrix = mesh.getShapeMatrix();
		vector<int> leftFaces, rightFaces;
		vector<float> resizeLeft, resizeRight;
		vector<GLuint> verts;
		float v0, v1, v2;
		int matchingLeft;

		int countLeft = 0;
		int countRight = 0;
		int rem = 0;

		for (int faceId : faces) {
			matchingLeft = 0;

			verts = mesh.getFace(faceId).vertex_ids;

			v0 = (shapeModelMatrix * mesh.getVertex(verts[0]))[splitAxis];
			v1 = (shapeModelMatrix * mesh.getVertex(verts[1]))[splitAxis];;
			v2 = (shapeModelMatrix * mesh.getVertex(verts[2]))[splitAxis];;

			if (v0 < centerLongestAxis) matchingLeft++;
			if (v1 < centerLongestAxis) matchingLeft++;
			if (v2 < centerLongestAxis) matchingLeft++;

			switch (matchingLeft) {
			// Face completely lays inside the RIGHT box.
			case 0:
				rightFaces.push_back(faceId);
				countRight++;
				break;
			case 1:
				rightFaces.push_back(faceId);
				splitMin[splitAxis] = minFloat(resizeRight);
				break;
			case 2:
				leftFaces.push_back(faceId);
				splitMax[splitAxis] = maxFloat(resizeLeft);
				break;
			// Face completely lays inside the LEFT box.
			case 3:
				leftFaces.push_back(faceId);
				countLeft++;
				break;
			// Face is on the split line and 1 box needs resizing.
			default:
				//float triangleMass = (v0 + v1 + v2) / 3;
				//// If triangleMass lays left on the split axis, it belongs to the left box.
				//if (triangleMass < centerLongestAxis) {
				//	leftFaces.push_back(faceId);
				//	resizeLeft.push_back(v0);
				//	resizeLeft.push_back(v1);
				//	resizeLeft.push_back(v2);
				//}
				//else {
				//	rightFaces.push_back(faceId);
				//	resizeRight.push_back(v0);
				//	resizeRight.push_back(v1);
				//	resizeRight.push_back(v2);
				//}
				//rem++;
				break;
			}

		}
		//std::cout << "LEFT IS: " << countLeft << std::endl;
		//std::cout << "RIGHT IS: " << countRight << std::endl;
		//std::cout << "REMAINING IS: " << rem << std::endl;

		
		// Resize every box if needed to resize.
		//if (!resizeLeft.empty()) {
		//	std::cout << "SPLIT AXIS " << centerLongestAxis << std::endl;
		//	std::cout << "BOX LEFT BEFORE RESIZE: " << splitMin << ", " << splitMax << std::endl;
		//	splitMax[splitAxis] = maxFloat(resizeLeft);
		//	std::cout << "BOX LEFT AFTER RESIZE: " << splitMin << ", " << splitMax << std::endl;
		//}
		//if (!resizeRight.empty()) {
		//	std::cout << "BOX RIGHT BEFORE RESIZE: " << splitMin << ", " << splitMax << std::endl;
		//	splitMin[splitAxis] = minFloat(resizeRight);
		//	std::cout << "BOX RIGHT AFTER RESIZE: " << splitMin << ", " << splitMax << std::endl;
		//}

		//*****************************************

		Box left = Box(min, splitMax);
		Box right = Box(splitMin, max);

		//Add faces to the face tree (faceBuckets)
		addLeftBucket(i, leftFaces);
		addRightBucket(i, rightFaces);

		//Add boxes to the box tree (boxes)
		addLeft(i, left);
		addRight(i, right);
		k--;
		split(leftIndex, k);
		split(leftIndex + 1, k);
	}
	
	vector<Box> getLevel(int level) {
		vector<Box> result;

		int start = pow(2, level - 1) - 1;
		int end = pow(2, level) - 1;

		for (int i = start; i < end; i++)	result.push_back(boxes[i]);

		return result;
	}

	void displayLevelFaces(int level) {
		int start = pow(2, level - 1) - 1;
		int end = pow(2, level) - 1;

		for (int i = start; i < end; i++) {
			vector<int> faces = faceBuckets[i];
			std::cout << "index: " << i << "  | " << std::ends;
			for (int faceId : faces) {
				std::cout << faceId << std::ends;
			}
			std::cout << std::endl;
		}

	}

	float minFloat(vector<float> axisValues) {
		float min = FLT_MAX;
		for (float value : axisValues) {
			if (value < min)	min = value;
		}
		return min;
	}

	float maxFloat(vector<float> values) {
		float max = FLT_MIN;
		for (float value : values) {
			if (value > max)	max = value;
		}
		return max;
	}

	void display() {
		int j = 1;
		int k = 0;
		char output;
		for (int i = 0; i < size; i++) {
			if (!boxes[i].null)	output = 'O';
			else output = '.';
			std::cout << output << std::ends;
			if (i == 0 || i % k == 0) {
				std::cout << std::endl;
				k = k + pow(2, j++);
			}
		}
			
	}

	void split() {

	}
};


#endif // TREESTRUCTURE