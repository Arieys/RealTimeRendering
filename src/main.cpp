#include "viewer.h"
#include "config.h"
#include <filesystem>
#include <string>

Options getOptions(int argc, char* argv[]) {
    Options options;
    options.windowTitle = "Rendering Engine";
    options.windowWidth = 1920;
    options.windowHeight = 1080;
    options.windowResizable = false;
    options.vSync = true;
    options.glOptions.msaa = true;
    options.glOptions.glVersion = {4, 6};
    options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    options.assetRootDir = ROOT_PATH + "/media/"s;
    options.argc = argc;
    options.argv = argv;
    options.glOptions.glInfoDbg = false;
    options.glOptions.glWarnDbg = true;
    return options;
}

int main(int argc, char* argv[]) {
    Options options = getOptions(argc, argv);
    std::cout << "current path = " << std::filesystem::current_path() << std::endl;
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