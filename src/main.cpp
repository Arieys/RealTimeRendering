#define NORMAL_RUN 0x00000000
#define COMPUTE_SHADER_DEBUG_RUN 0x00000001

#define RUNMODE NORMAL_RUN

#if RUNMODE == NORMAL_RUN
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
    options.glOptions.glVersion = { 4, 6 };
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

#elif RUNMODE == COMPUTE_SHADER_DEBUG_RUN

#include <stdlib.h>  
#include <glad/gl.h>  
#include <GLFW/glfw3.h> 
#include <stdexcept>
#include <iostream>  
#include <vector>  
#include "base/glsl_program.h"
#include <chrono>  

const int ARRAY_SIZE = 10240000;
const int THREAD_GROUP_SIZE = 64;

void errorCallback(int error, const char* description)
{
    std::cout << "GLFW Error: " << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main()
{
    // ��ʼ��GLFW  
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    // ����GLFW����ص�����  
    glfwSetErrorCallback(errorCallback);

    // ��������  
    GLFWwindow* window = glfwCreateWindow(800, 600, "Compute Shader Array Sum", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }

    // ���ü��̻ص�����  
    glfwSetKeyCallback(window, keyCallback);

    // ����OpenGL������  
    glfwMakeContextCurrent(window);

    // load OpenGL library functions
#ifdef __EMSCRIPTEN__
    // Emscripten link OpenGL ES library statically
#elif USE_GLES
    if (!gladLoadGLES2(glfwGetProcAddress)) {
        throw std::runtime_error("glad initialization OpenGL/ES2 failure");
    }
#else
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("glad initialization OpenGL failure");
    }
#endif

    // ���������������ͽ������  
    std::vector<int> inputArray(ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        inputArray[i] = static_cast<int>(i);
    }
    std::vector<int> sumResult(1, 0.0f);

    // �����������黺����  
    GLuint inputBuffer;
    glGenBuffers(1, &inputBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ARRAY_SIZE * sizeof(int), inputArray.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // ������ͽ��������  
    GLuint sumBuffer;
    glGenBuffers(1, &sumBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), sumResult.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sumBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // ����Compute Shader����  
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* computeShaderSource = R"(  
        #version 460  
  
        layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;  
  
        layout(std430, binding = 0) buffer InputBuffer {  
            int inputArray[];  
        };  
  
        layout(std430, binding = 1) buffer SumBuffer {  
            int sumResult[];  
        };  

        shared int sharedSum[64];  
  
        void main()  
        {  
            uint index = gl_LocalInvocationID.x;  
            uint globalIndex = gl_GlobalInvocationID.x +  
                  gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x +  
                  gl_GlobalInvocationID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_WorkGroupSize.x * gl_WorkGroupSize.y;  

            sharedSum[index] = inputArray[globalIndex];     
  
            barrier(); 

            for(int l = 32; l >=1; l/=2)
            {
                if(index + l < 2 * l)
                {
                    sharedSum[index] = sharedSum[index+l] + sharedSum[index];
                }
                barrier(); 
            }

/*            // ÿ���߳̽��������ݴ洢�������ڴ�  
            if (globalIndex < inputArray.length())  
            {  
                int inputValue = inputArray[globalIndex];  
                atomicAdd(sharedSum, inputValue); 
            }  
  
            barrier(); */ 

            // ��һ���߳̽������ڴ��е�ֵ�ۼӵ�ȫ�ֻ�����  
            if (index == 0)  
            {  
                atomicAdd(sumResult[0], sharedSum[0]);  
            }
        }  
    )";
    // ��ȡOpenGL�汾��Ϣ  
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    // ����汾��Ϣ  
    std::cout << "OpenGL�汾��" << majorVersion << "." << minorVersion << std::endl;


    glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
    glCompileShader(computeShader);

    // ��������������  
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    // ��ȡÿ��������Ĵ�С  
    GLint workGroupSize[3];
    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);

    // ������  
    std::cout << "ÿ��wrap��Kernel����" << workGroupSize[0] << " * " << workGroupSize[1] << " * " << workGroupSize[2] << std::endl;

    // ʹ�ü���������  
    glUseProgram(computeProgram);

    // �����߳��������߳����С  
    int numThreadGroups = (ARRAY_SIZE + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
    std::cout << "numThreadGroups = " << numThreadGroups << std::endl;

    // ����Timer Query Objects  
    GLuint query;
    glGenQueries(1, &query);

    // ������ʱ��  
    glBeginQuery(GL_TIME_ELAPSED, query);

    // ִ��Compute Shader  
    glDispatchCompute(numThreadGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // ֹͣ��ʱ��  
    glEndQuery(GL_TIME_ELAPSED);

    // �ȴ���ʱ�����  
    GLint available = 0;
    while (!available) {
        glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
    }

    // ��ȡ��ʱ�����  
    GLuint64 elapsedTime;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedTime);

    // �������ʱ�䣨�Ժ���Ϊ��λ��  
    std::cout << "Compute Shader����ʱ�䣺" << elapsedTime / 1000000.0 << "����" << std::endl;

    // ����ͽ����������ȡ���  
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sumBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sumResult.size() * sizeof(int), sumResult.data());

    // ��ӡ��ͽ��  
    std::cout << "Result size = " << sumResult.size() << std::endl;
    for (int i = 0; i < sumResult.size(); i++)
    {
        std::cout << "Result[" << i << "] = " << sumResult[i] << std::endl;
    }


    // ������Դ  
    glDeleteProgram(computeProgram);
    glDeleteShader(computeShader);
    glDeleteBuffers(1, &inputBuffer);
    glDeleteBuffers(1, &sumBuffer);

    // �رմ��ں�����GLFW  
    glfwDestroyWindow(window);
    glfwTerminate();

    //cpu
    // ����������Ͳ�ͳ��ʱ��  
    std::cout << "CPU CALCULATION TIME : " << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    int sum = 0;
    for (int i = 0; i < inputArray.size(); ++i)
    {
        sum += inputArray[i];
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // ��΢��ת��Ϊ����  
    std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    // ��������ʱ��  
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Time: " << milliseconds.count() << " milliseconds" << std::endl;

    return 0;
}
#endif