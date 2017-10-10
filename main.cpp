#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <c_predict_api.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

const mx_uint IMSIZE = 224;

// Loads a binary file into memory
vector<char>
load_file(const char *filename)
{
	ifstream f(filename, ios::binary | ios::ate);
	streamsize sz = f.tellg();
	f.seekg(0, ios::beg);
	vector<char> buffer(sz);
	if (!f.read(buffer.data(), sz)) {
		fprintf(stderr, "error reading %s\n", filename);
	}

	return buffer;
}

PredictorHandle
load_model(const char *json_file, const char *param_file)
{
	PredictorHandle ph;
	auto model_symbol = load_file(json_file);
	auto model_params = load_file(param_file);

	const char *input_keys[] = {"data"};
	const mx_uint input_shape_indptr[] = {0, 4};
	// 1 picture, rgb, 224*224
	const mx_uint input_shape_data[] = {1, 3, IMSIZE, IMSIZE};

	auto status =
	    MXPredCreate(model_symbol.data(), model_params.data(),
	                 model_params.size(), 1 /* 1 == CPU */,
	                 0, // device id
	                 1, // number of inputs
	                 input_keys, input_shape_indptr, input_shape_data, &ph);

	if (status != 0) {
		fprintf(stderr, "error loading model\n");
		return NULL;
	}

	return ph;
}

vector<string>
load_synset(const char *filename)
{
	vector<string> syn;
	ifstream f(filename);

	for (string l; getline(f, l);) {
		syn.push_back(l);
	}

	return syn;
}

int
load_image(const char *filename, Mat &img_out)
{
	Mat img_in, img_rgb, img_resized, img_float;

	img_in = imread(filename, cv::IMREAD_COLOR);
	if (img_in.data == NULL) {
		fprintf(stderr, "Error reading image: %s\n", filename);
		return -1;
	}

	/* Convert the loaded BGR to RGB */
	cvtColor(img_in, img_rgb, COLOR_BGR2RGB);

	/* Resize Image */
	Size input_size{width : IMSIZE, height : IMSIZE};
	resize(img_rgb, img_resized, input_size);

	/* Convert to 32bit float */
	img_resized.convertTo(img_float, CV_32FC3);

	/* Split channels into RRR, GGG, BBB as required for the neural net */
	vector<Mat> chans;
	split(img_float, chans);
	img_out.push_back(chans[0]);
	img_out.push_back(chans[1]);
	img_out.push_back(chans[2]);

	return 0;
}

/* Represents output of the neural network */
struct PredictedClass {
	float probability;
	string description;
};

int
main(int argc, char **argv)
{
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " imagefile\n";
		return -1;
	}
	const char *img_filename = argv[1];

	/* Load and preprocess an image file using OpenCV */
	Mat img;
	if (load_image(img_filename, img) != 0) {
		cerr << "Error loading image: " << img_filename << endl;
	}

	/* Setup and load mxnet model and synset(maps output index to label) */
	PredictorHandle ph = load_model("model/resnet-152-symbol.json",
	                                "model/resnet-152-0000.params");
	auto synset = load_synset("model/synset.txt");

	/* Figure out how much memory we need and */
	/* allocate enough for the output of the neural network */
	mx_uint *shape;
	mx_uint shape_ndim;
	MXPredGetOutputShape(ph, 0, &shape, &shape_ndim);
	assert(shape_ndim == 2);
	const auto input_size = shape[1];
	auto output = vector<float>(input_size);
	MXPredSetInput(ph, "data", (float *)img.data, IMSIZE * IMSIZE * 3);

	MXPredForward(ph);
	MXPredGetOutput(ph, 0, output.data(), input_size);

	/* Assign each output probability to the corresponding label of the
	 * synset */
	vector<PredictedClass> result;
	for (size_t i = 0; i < shape[1]; i++) {
		result.push_back(PredictedClass{
			probability : output[i],
			description : synset[i]
		});
	}

	/* Sort output by probability */
	sort(result.begin(), result.end(),
	     [](const PredictedClass &a, const PredictedClass &b) {
		     return b.probability < a.probability;
	     });

	/* Display top 30 labels */
	for (size_t i = 0; i < 30; i++) {
		printf("%f: %s\n", result[i].probability,
		       result[i].description.c_str());
	}

	MXPredFree(ph);
	return 0;
}
