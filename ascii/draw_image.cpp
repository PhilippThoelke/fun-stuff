#include <string>
#include <iostream>
#include <sys/ioctl.h>
#include <CImg.h>

using namespace std;
using namespace cimg_library;

int main(int argc, char** argv) {
	// define the set of characters to use
	const string chars = " .:-=+*#%@$";

	// get the image path
	string path = "img.jpg";
	if(argc > 1) {
		path = argv[1];
	}

	// load the image
	CImg<unsigned char> img(path.c_str());

	// get the terminal dimensions
    struct winsize dims;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &dims);

	// print the image as ASCII characters
	float xScale = img.width() / dims.ws_col;
	float yScale = img.height() / dims.ws_row;
	for(int y = 0; y < dims.ws_row; y++) {
		for(int x = 0; x < dims.ws_col; x++) {
			int index = y * yScale * img.width() + x * xScale;
			float value = img[index] / 255.0;
			cout << chars[(int) (value * chars.length())];
		}
		cout << "\n";
	}
	return 0;
}
