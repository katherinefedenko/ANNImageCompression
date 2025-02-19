#include "pch.h"
#include <iostream>
#include <cmath>
#include <vector>
#include "CImg_latest/CImg-2.7.5/CImg.h"
#include <armadillo>

using namespace cimg_library;
using namespace arma;
using namespace std;

int rgb = 3;
mat X;
mat X_;
mat Y;
mat deltaX;
mat W;
mat W_;
vector<mat> commonVector;
double convertColor(double number) {
	double valueColor = 2 * number / 255 - 1;
	return valueColor;
}

void dividedIntoRectangles(CImg<double> image, int width, int height, int n, int m) {
	int pixel = 0;

	for (int i = 0; i < width; i += m) {
		for (int j = 0; j < height; j += n) {
			mat matrix;
			matrix.set_size(1, n*m*rgb);
			pixel = 0;
			for (int k = i; k < i + m; k++) {
				for (int s = j; s < j + n; s++) {
					double colorRedInPixel = image(k, s, 0, 0);
					matrix(0, pixel++) = convertColor(colorRedInPixel);

					double colorGreenInPixel = image(k, s, 0, 1);
					matrix(0, pixel++) = convertColor(colorGreenInPixel);

					double colorBlueInPixel = image(k, s, 0, 2);
					matrix(0, pixel++) = convertColor(colorBlueInPixel);
				}
			}
			commonVector.push_back(matrix);
		}
	}
}
void normalizeW(int p, int N, mat W) {
	for (int i = 0; i < p; i++) {
		double sum = 0;
		for (int j = 0; j < N; j++) {
			sum += W(j, i) * W(j, i);
		}
		sum = sqrt(sum);
		for (int j = 0; j < N; j++) {
			W(j, i) = W(j, i) / sum;
		}
	}
}
void normalizeW_(int p, int N, mat W_) {
	for (int i = 0; i < N; i++) {
		double sum = 0;
		for (int j = 0; j < p; j++) {
			sum += W_(j, i) * W_(j, i);
		}
		sum = sqrt(sum);
		for (int j = 0; j < p; j++) {
			W_(j, i) = W_(j, i) / sum;
		}
	}
}
void buildWeightsMatrix(int n, int m, int p) {
	W.set_size(n*m*rgb, p);
	for (int i = 0; i < n*m*rgb; i++)
		for (int j = 0; j < p; j++) {
			double s = (double)(rand()) / RAND_MAX * 2 - 1;
			W(i, j) = 0.1 * s;
		}
	W_ = W.t();
	normalizeW(p, n*m*rgb, W);
	normalizeW_(p, n*m*rgb, W_);
}
void learningNetwork(int p, int N, int e, double alpha) {
	double E;
	int iteration = 1;

	do {
		E = 0;

		for (int i = 0; i < commonVector.size(); i++) {
			X = commonVector[i];
			Y = X * W;
			X_ = Y * W_;
			deltaX = X_ - X;

			W = W - alpha * X.t() * deltaX * W_.t();
			W_ = W_ - alpha * Y.t() * deltaX;
		}
		normalizeW(p, N, W);
		normalizeW_(p, N, W_);
		for (int i = 0; i < commonVector.size(); i++) {
			X = commonVector[i];
			Y = X * W;
			X_ = Y * W_;
			deltaX = X_ - X;

			for (int j = 0; j < N; j++) {
				E += pow(deltaX(0, j), 2);
			}
		}

		cout << "Current iteration: " << iteration << "\tTotal error: " << E << endl;
		iteration++;
	} while (E > e);

	double z = (N*commonVector.size()*1.0) / ((N + commonVector.size())*p + 2);
	
	cout << "Compression ratio(Z) = " << z << endl;
}

double buildColorPixel(double number) {
	double pixel = 255 * (number + 1) / 2;

	if (pixel < 0)
		pixel = 0;
	if (pixel > 255)
		pixel = 255;

	return pixel;
}

void buildNewImage(int n, int m, int width, int height) {
	CImg<double> img(width, height, 1, 3);
	int s = 0;

	for (int i = 0; i < width; i += m) {
		for (int j = 0; j < height; j += n) {
			X = commonVector[s];
			s++;
			Y = X * W;
			X_ = Y * W_;
			int pixel = 0;

			for (int k = i; k < i + m; k++) {
				for (int s = j; s < j + n; s++)
				{
					img(k, s, 0, 0) = buildColorPixel(X_(0, pixel++));
					img(k, s, 0, 1) = buildColorPixel(X_(0, pixel++));
					img(k, s, 0, 2) = buildColorPixel(X_(0, pixel++));
				}
			}
		}
	}
	img.display();
	img.save("newImage.bmp");
}

int main() {
	setlocale(LC_ALL, "Russian");
	CImg<double> image("1.bmp");
	int imWidth = image.width();
	int imHeight = image.height();
	int n, m, p, e;
	double alpha;

	cout << "Enter rectangle height(m): ";
	cin >> m;

	cout << "Enter rectangle width(n): ";
	cin >> n;

	cout << "Enter neuron amount for second layer(p): ";
	cin >> p;

	cout << "Enter error degree(e): ";
	cin >> e;

	cout << "Enter learning step(alpha): ";
	cin >> alpha;

	dividedIntoRectangles(image, imWidth, imHeight, n, m);
	buildWeightsMatrix(n, m, p);
	learningNetwork(p, n*m*rgb, e, alpha);
	buildNewImage(n, m, imWidth, imHeight);

	getchar();
	return 0;
}
