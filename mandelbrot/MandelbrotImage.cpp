#include <SDL2/SDL.h>
#include <iostream>

constexpr Uint32 WIDTH = 1200;
constexpr Uint32 HEIGHT = 800;

constexpr Uint32 TEX_WIDTH = WIDTH;
constexpr Uint32 TEX_HEIGHT = HEIGHT;

constexpr float MOVE_SPEED = 0.05;
constexpr float ZOOM_SPEED = 1.1;
constexpr float FAST_ZOOM_SPEED = 1.5;

Uint32 iterations = 100;
float threshold = 4;

void mandelbrot(Uint32* pixel, double x, double y) {
    double re = 0;
    double im = 0;
    Uint32 iteration = 0;
    // iterate the Mandelbrot sequence for the starting value x + yi
    while(re * re + im * im <= threshold && iteration < iterations) {
        double temp = re * re - im * im + x;
        im = 2 * re * im + y;
        re = temp;
        iteration++;
    }

    // set the pixel color using the iterations it took
    char r = iteration * 255 / iterations;
    char g = r;
    char b = r;
    *pixel = r | g << 8 | b << 16;
}

int main() {
    std::cout << "Interactive visualization of the Mandelbrot set\n";
    std::cout << "The program uses OpenMP to parallelize the computations for each pixel. Moving the image, zooming and \n";
    std::cout << "changing the iteration and divergence parameters are supported. The iterations parameter determines\n";
    std::cout << "how many steps of the sequence are computed before a starting value is labeled as converging, while\n";
    std::cout << "the threshold parameter defines a boundary after which a starting value is labeled as diverging.\n";
    std::cout << "Controls:\n";
    std::cout << "   Moving:\t\t\tarrow keys\n";
    std::cout << "   Zoom in:\t\t\tspace\n";
    std::cout << "   Zoom out:\t\t\t(left) shift\n";
    std::cout << "   Zoom in (fast):\t\t(left) control\n";
    std::cout << "   Zoom out (fast):\t\t(left) alt\n";
    std::cout << "   Increase iterations:\t\ti\n";
    std::cout << "   Decrease iterations:\t\tu\n";
    std::cout << "   Increase threshold:\t\tt\n";
    std::cout << "   Decrease threshold:\t\tr" << std::endl;

    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Mandelbrot set", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // initialize the texture to show the Mandelbrot set on
    SDL_Texture* texture = SDL_CreateTexture (renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    Uint32 format;
    SDL_QueryTexture(texture, &format, nullptr, nullptr, nullptr);
    SDL_PixelFormat pixelFormat;
    pixelFormat.format = format;

    // define the rects for copying pixel data
    SDL_Rect from;
    from.x = 0;
    from.y = 0;
    from.w = TEX_WIDTH;
    from.h = TEX_HEIGHT;
    SDL_Rect to;
    to.x = 0;
    to.y = 0;
    to.w = WIDTH;
    to.h = HEIGHT;

    // define the pixels array
    Uint32* pixels;
    int pitch;

    // initialize scale and position
    double scale = 3;
    double xOffset = 0;
    double yOffset = 0;

    // run the drawing loop
    bool running = true;
    SDL_Event event;
    while(running) {
        // check if the user has closed the window
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            }
        }

        // parse the user input
        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        if(keyboardState[SDL_SCANCODE_SPACE]) {
            scale /= ZOOM_SPEED;
        }
        if(keyboardState[SDL_SCANCODE_LSHIFT]) {
            scale *= ZOOM_SPEED;
        }
        if(keyboardState[SDL_SCANCODE_LALT]) {
            scale /= FAST_ZOOM_SPEED;
        }
        if(keyboardState[SDL_SCANCODE_LCTRL]) {
            scale *= FAST_ZOOM_SPEED;
        }
        if(keyboardState[SDL_SCANCODE_LEFT]) {
            xOffset -= MOVE_SPEED * scale;
        }
        if(keyboardState[SDL_SCANCODE_UP]) {
            yOffset -= MOVE_SPEED * scale;
        }
        if(keyboardState[SDL_SCANCODE_RIGHT]) {
            xOffset += MOVE_SPEED * scale;
        }
        if(keyboardState[SDL_SCANCODE_DOWN]) {
            yOffset += MOVE_SPEED * scale;
        }
        if(keyboardState[SDL_SCANCODE_I]) {
            iterations += 1;
        }
        if(keyboardState[SDL_SCANCODE_U]) {
            iterations -= 1;
        }
        if(keyboardState[SDL_SCANCODE_T]) {
            threshold += 0.1;
        }
        if(keyboardState[SDL_SCANCODE_R]) {
            threshold -= 0.1;
        }

        // prepare the texture for writing
        SDL_LockTexture(texture, nullptr, (void**) &pixels, &pitch);

        // update the pixel values
        #pragma omp parallel for collapse(2)
        for(auto x = 0; x < TEX_WIDTH; x++) {
            for(auto y = 0; y < TEX_HEIGHT; y++) {
                double xPos = ((double) x / TEX_WIDTH * 2 - 1) * scale + xOffset;
                double yPos = ((double) y / TEX_HEIGHT * 2 - 1) * scale + yOffset;
                mandelbrot(pixels + y * (pitch / 4) + x, xPos, yPos);
            }
        }

        // release the texture and draw the current image
        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, &from, &to);
        SDL_RenderPresent(renderer);

    }

    // free SDL resources
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}