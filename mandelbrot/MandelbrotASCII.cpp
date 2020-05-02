#include <string>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace std;

const string CHARS = " .:-=+*#%@$";
const int RESERVED_ROWS = 3;

constexpr float MOVE_SPEED = 0.05;
constexpr float ZOOM_SPEED = 1.1;
constexpr float FAST_ZOOM_SPEED = 1.5;

unsigned int iterations = 1000;
float threshold = 4;

char mandelbrot(double x, double y) {
    double re = 0;
    double im = 0;
    unsigned int iteration = 0;
    // iterate the Mandelbrot sequence for the starting value x + yi
    while(re * re + im * im <= threshold && iteration < iterations) {
        double temp = re * re - im * im + x;
        im = 2 * re * im + y;
        re = temp;
        iteration++;
    }
    // return an ASCII character depending on the number of iterations
    return CHARS[(int) ((float) iteration / (iterations + 1) * CHARS.length())];
}

int main() {
    // get the terminal size
    struct winsize dims;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &dims);

    // initialize scale and position
    double scale = 1;
    double xOffset = -0.5;
    double yOffset = 0;

    // init ASCII char array
    char mandelbrotChars[(dims.ws_row - RESERVED_ROWS) * (dims.ws_col + 1)];
    for(int i = dims.ws_col; i < (dims.ws_row - RESERVED_ROWS) * (dims.ws_col + 1); i += dims.ws_col) {
        mandelbrotChars[i] = '\n';
    }

    // run the drawing loop
    bool firstIt = true;
    while(true) {
        string input = " ";
        if(!firstIt) {
            // get user input
            cin >> input;
        }
        firstIt = false;

        // iterate over the input
        int repeats = 1;
        string num = "";
        for(int i = 0; i < input.length(); i++) {
            // check for numerical input
            if(input[i] >= '0' && input[i] <= '9') {
                num += input[i];
                continue;
            }

            // parse the numerical input to an integer
            if(num.length() > 0) {
                repeats = stoi(num);
            }

            // apply the user action repeats times
            for(int j = 0; j < repeats; j++) {
                switch(input[i]) {
                    case '+':
                        scale /= ZOOM_SPEED;
                        break;
                    case '-':
                        scale *= ZOOM_SPEED;
                        break;
                    case 'w':
                        yOffset -= MOVE_SPEED * scale;
                        break;
                    case 'a':
                        xOffset -= MOVE_SPEED * scale;
                        break;
                    case 's':
                        yOffset += MOVE_SPEED * scale;
                        break;
                    case 'd':
                        xOffset += MOVE_SPEED * scale;
                        break;
                    case 'i':
                        iterations += 10;
                        break;
                    case 'u':
                        iterations -= 10;
                        break;
                    case 't':
                        threshold += 1;
                        break;
                    case 'r':
                        threshold -= 1;
                        break;
                }

                // update the char array
                #pragma omp parallel for collapse(2)
                for(unsigned int y = 0; y < dims.ws_row - RESERVED_ROWS; y++) {
                    for(unsigned int x = 0; x < dims.ws_col; x++) {
                        double xPos = ((double) x / dims.ws_col * 2 - 1) * scale + xOffset;
                        double yPos = ((double) y / (dims.ws_row - RESERVED_ROWS) * 2 - 1) * scale + yOffset;
                        mandelbrotChars[y * dims.ws_col + x] = mandelbrot(xPos, yPos);
                    }
                }

                // draw Mandelbrot set and instructions
                system("clear");
                cout << mandelbrotChars << endl;
                cout << "up: w, down: s, left: a, right: d, +zoom: +, -zoom: -, +it: i, -it: u, +thresh: t, -thresh: r";
                cout << " (repeat actions with a number prefix, e.g. 10+5a)" << endl;
                // wait for 10 milliseconds so the terminal has time to print everything
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            num = "";
            repeats = 1;
        }
    }
    return 0;
}

