#include "viewer.h"
#include <filesystem>

Options getOptions(int argc, char* argv[]) {
    Options options;
    options.windowTitle = "PS Viewer";
    options.windowWidth = 1920;
    options.windowHeight = 1080;
    options.windowResizable = false;
    options.vSync = true;
    options.msaa = true;
    options.glVersion = {3, 3};
    options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    options.assetRootDir = "../../../../media/";
    options.argc = argc;
    options.argv = argv;
    return options;
}

int main(int argc, char* argv[]) {
    Options options = getOptions(argc, argv);
    std::cout << std::filesystem::current_path() << std::endl;
    try {
        Viewer viewer(options);
        viewer.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}