//============================================================================
// Name        : hahaha.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <map>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <audiosystem.h>
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif
#include <texture.h>
#include <shader.h>
#include <plane.h>
#include <model.h>
#include <plane.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
using namespace std;

#define SET_SHADOW_RES 0
#define SET_PSHADOW_RES 1

glm::vec3 plightPos[] = {
    glm::vec3(0.0f, 0.0f, -3.0f),
    glm::vec3(5.0f, 5.0f, -5.0f),
    glm::vec3(-5.0f, 2.0f, -5.0f),
    glm::vec3(0.0f, -2.0f, -10.0f),
	glm::vec3(3.0f, -2.0f, -10.0f),
	glm::vec3(2.0f, 1.5f, -5.0f),
};

constexpr size_t plight_num = sizeof(plightPos) / sizeof(plightPos[0]);

GLFWwindow* window;
GLFWmonitor *monitor;
const GLFWvidmode *monitor_mode;
AudioSystem audio;
unsigned int VAO, VAO1, VAO2, VAO3, VAO4, VAO5;
unsigned int VBO, VBO1, VBO2, VBO3, VBO4, VBO5;
unsigned int FBO, RBO, colorbuffer;
unsigned int FBO_MSAA, colorbuffer_MSAA, RBO_MSAA;
unsigned int FBO_depth, FBO_pshadow;
unsigned int depthMap;
unsigned int depthCubemap;
unsigned int UBO;
int SHADOW_HEIGHT ,SHADOW_WIDTH;
int PSHADOW_HEIGHT, PSHADOW_WIDTH;

unsigned int quadVAO = 0;
unsigned int quadVBO;

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

//For Audio Calculations
glm::vec3 camPrevPos = camPos;
glm::vec3 camVelocity = glm::vec3(0.0f);

float deltatime = 0.0f;
float lasttime = 0.0f;

int lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
float fov = 45.0f;

bool toggleFlash = false;
bool toggleMenu = false;
int flashlight_state = 0;
bool debugPshadow = true;
bool fullscreen = false;
bool isMuted = true;
bool debug_normalmapping = true;
bool debug_parallaxmapping = true;
bool debug_hdr = false;
float hdr_exposure = 1.0;
bool debug_fb_srgb = true;

glm::vec3 cubePositions[] = {
	glm::vec3(2.0f,   2.0f, -4.0f),
	glm::vec3(2.0f,   5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f,  -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f,  -2.0f, -2.5f),
	glm::vec3(1.5f,   2.0f, -2.5f),
	glm::vec3(1.5f,   0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f),
	glm::vec3(-6.0f, -2.0f, -22.0f)
};

unsigned int loadCubemap(std::vector<std::string> faces);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void calcCamVelocity(){
	if (deltatime > 0.0f)
		camVelocity = (camPos - camPrevPos) / deltatime;

	camPrevPos = camPos;
}

void toggleFullscreen(GLFWwindow* window) {
	if (fullscreen) {
		glfwSetWindowMonitor(window, monitor, 0, 0, monitor_mode->width, monitor_mode->height, monitor_mode->refreshRate);
	} else {
		glfwSetWindowMonitor(window, nullptr, 100, 100, 800, 600, 0);
	}
}

void processinput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        std::cout << "ESC Pressed\n";
        glfwSetWindowShouldClose(window, true);
    }

    static bool isF2pressed = false;
    int F2STATE = glfwGetKey(window, GLFW_KEY_F2);
    if (F2STATE == GLFW_PRESS && !isF2pressed) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        std::cout << "Width : " << width << " Height : " << height << "\n";
        isF2pressed = true;
    }
    else if (F2STATE == GLFW_RELEASE) {
        isF2pressed = false;
    }

    static bool isF3pressed = false;
    int F3STATE = glfwGetKey(window, GLFW_KEY_F3);
    int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
    if (F3STATE == GLFW_PRESS && !isF3pressed) {
        if (cursorMode == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            //std::cout << "Cursor Unlocked\n";
        }
        else if (cursorMode == GLFW_CURSOR_NORMAL){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursor(window, nullptr);
            //std::cout << "Cursor Locked\n";
        }
        isF3pressed = true;
    }
    else if (F3STATE == GLFW_RELEASE) {
        isF3pressed = false;
    }

    static bool isF4pressed = false;
    bool isMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
    int F4STATE = glfwGetKey(window, GLFW_KEY_F4);
    if (F4STATE == GLFW_PRESS && !isF4pressed) {
        if (isMaximized)
            glfwRestoreWindow(window);
        else
            glfwMaximizeWindow(window);
        isF4pressed = true;
    }
    else if (F4STATE == GLFW_RELEASE) {
        isF4pressed = false;
    }

    static bool isF5pressed = false;
    int F5STATE = glfwGetKey(window, GLFW_KEY_F5);
    if (F5STATE == GLFW_PRESS && !isF5pressed) {
        if (!toggleMenu)
            toggleMenu = true;
        else
            toggleMenu = false;
        isF5pressed = true;
    }
    else if (F5STATE == GLFW_RELEASE) {
        isF5pressed = false;
    }

	static bool isF6pressed = false;
	int F6STATE = glfwGetKey(window, GLFW_KEY_F6);
	if (F6STATE == GLFW_PRESS && !isF6pressed) {
		if (!debugPshadow)
			debugPshadow = true;
		else
			debugPshadow = false;
		isF6pressed = true;
	} else if (F6STATE == GLFW_RELEASE) {
		isF6pressed = false;
	}

	static bool isF7pressed = false;
	int F7STATE = glfwGetKey(window, GLFW_KEY_F7);
	if (F7STATE == GLFW_PRESS && !isF7pressed) {
		if (!fullscreen)
			fullscreen = true;
		else
			fullscreen = false;
		toggleFullscreen(window);
		isF7pressed = true;
	} else if (F7STATE == GLFW_RELEASE) {
		isF7pressed = false;
	}

	static bool isF8pressed = false;
	int F8STATE = glfwGetKey(window, GLFW_KEY_F8);
	if (F8STATE == GLFW_PRESS && !isF8pressed) {
		if (!isMuted)
			isMuted = true;
		else
			isMuted = false;
		isF8pressed = true;
	} else if (F8STATE == GLFW_RELEASE) {
		isF8pressed = false;
	}

    float camSpeed = 5.0f * deltatime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos -= glm::cross(camFront, glm::normalize(glm::cross(camFront, camUp))) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camPos += glm::cross(camFront, glm::normalize(glm::cross(camFront, camUp))) * camSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camPos += camSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camPos -= camSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camPos -= (camSpeed * camFront) * 0.75f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camPos += (camSpeed * camFront) * 0.75f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed * 0.75f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed * 0.75f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camPos += glm::cross(camFront, glm::normalize(glm::cross(camFront, camUp))) * camSpeed * 0.75f;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            camPos -= glm::cross(camFront, glm::normalize(glm::cross(camFront, camUp))) * camSpeed * 0.75f;
    }

    calcCamVelocity();

    static bool isFpressed = false;
    int FSTATE = glfwGetKey(window, GLFW_KEY_F);
    if (FSTATE == GLFW_PRESS && !isFpressed) {
        flashlight_state = (flashlight_state + 1) % 3;
        isFpressed = true;
    }
    else if (FSTATE == GLFW_RELEASE) {
        isFpressed = false;
    }

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset * 1.5f;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

glm::vec3 extractPosition(const glm::mat4& modelMatrix) {
    return glm::vec3(modelMatrix[3]); // Directly get the translation component
}

bool init0(){
	cout << "!!!Hello World!!!" << endl;
	std::cout << "Aloooha! whattup, gng?\n";

	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != nullptr) {
		std::cout << "Current working directory: " << cwd << std::endl;
	}

	//--------------------------------------------------//
	//            SETTING UP GLFW & GLAD                //
	//--------------------------------------------------//

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	monitor = glfwGetPrimaryMonitor();
	monitor_mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(800, 600, "hahaha2", NULL, NULL);
	if (window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		exit(-1);
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//----------------------------------------------//
	//            SETTING UP AUDIO                  //
	//----------------------------------------------//

	audio.init();
	audio.setDistanceModel("linear");

	return true;
}

bool isOn = init0();

//================================================

//----------------------------------------------//
//            SETTING UP SHADERS                //
//----------------------------------------------//

Shader shader00("shaders/vtxShader00.vs", "shaders/frgShader00.fs");
Shader shader01("shaders/framebuffer.vs", "shaders/framebuffer.fs");
Shader lightshader("shaders/lightcube.vs", "shaders/lightcube.fs");
Shader lightshaderobj("shaders/lightobj.vs", "shaders/lightobj.fs");
Shader lightshaderobj_instanced("shaders/lightobj_instanced.vs", "shaders/lightobj.fs");
//lightshaderobj.loadGeometry("shaders/lightobjexplode.gs");
Shader singlecolor("shaders/lightobj.vs", "shaders/singlecolor.fs");
Shader transparent("shaders/vtxShader00.vs", "shaders/transparent.fs");
Shader skyboxshader("shaders/skybox.vs", "shaders/skybox.fs");
Shader envmapping("shaders/envmapping.vs", "shaders/envmapping.fs");
Shader depthshader("shaders/depth_shader.vs", "shaders/depth_shader.fs");
Shader depthquad("shaders/depth_quad.vs", "shaders/depth_quad.fs");
Shader pshadow_depth("shaders/pshadow_depth.vs", "shaders/pshadow_depth.fs", "shaders/pshadow_depth.gs");


//----------------------------------------------//
//            SETTING UP TEXTURES               //
//----------------------------------------------//

/*
	__HDR DEBUG NOTE__
	SWITCH GAMMA CORRECTION ON FOR DIFFUSE/ALBEDO TEXTURES
	EVERYTHING ELSE STAYS OFF!!!
*/

Texture kiryutxt("textures/kiryu.png", false, true);
Texture majimatxt("textures/majima.png", false, true);
Texture container2txt("textures/container2.png", false, true);
Texture container2txt_specular("textures/container2_specular.png", false, false);
Texture naonao("textures/nao_cropped.jpg", true, true, 1);
Texture windowtxt("textures/window01.png", false, true);
Texture bricktxtnorm("textures/brickwall_normal.jpg", false, false);
Texture bricktxt("textures/brickwall.jpg", false, true);
Texture bricktxtdisp("textures/brickwall_disp.png", false, false);
Texture whitetxt("textures/white.png", false, false);
Texture bricks2("textures/bricks2.jpg", false, true);
Texture bricks2_normal("textures/bricks2_normal.jpg", false, false);
Texture bricks2_disp("textures/bricks2_disp.jpg", false, false);
Texture toybox_diff("textures/wood.png", false, true);
Texture toybox_norm("textures/toy_box_normal.png", false, false);
Texture toybox_disp("textures/toy_box_disp.png", false, false);

std::vector<std::string> faces
{
    "textures/skybox01/right.jpg",
    "textures/skybox01/left.jpg",
    "textures/skybox01/top.jpg",
    "textures/skybox01/bottom.jpg",
    "textures/skybox01/front.jpg",
    "textures/skybox01/back.jpg"
};
unsigned int skybox = loadCubemap(faces);

//------------------------------------//
//           MODEL LOADING            //
//------------------------------------//

Model rock("models/rock/rock.obj", FLIPPED_0);
Model planet("models/planet/planet.obj", FLIPPED_0);
Model backpack("models/backpack/backpack.obj", FLIPPED_D);
Model oillamp("models/oil_can_SF/oil_can_SF.obj", FLIPPED_0);
Plane plane00(150);

//----------------------------------------//
//              MISCELLANEOUS             //
//----------------------------------------//

bool isSkyboxOn = true, isWireframeOn = false, isBlinn = true, isVsyncOn = true;
float f = 0.694f, vol = 1.0f;
static int theme_current = 2;
static int filter_current = 0;
static int shadow_current = 1;
static int pshadow_current = 0;
static int msaa_list = 2;
static int msaa_current = 4;
const char* themes[] = { "Classic", "Dark", "Light" };
const char *shadow_res[] = {"1028", "2048", "4096"};
const char *pshadow_res[] = {"1028", "2048", "4096"};
const char* filters[] = { "Default", "Grayscale", "Negative",
	"Kernel", "Blur", "Edge", "Blurred Grayscale", "Half-Color" };
const char* msaa[] = { "Disabled", "x2", "x4", "x8", "x16" };

glm::vec3 sun(-30.2f, 70.0f, -80.3f);

Texture rocktxt("models/rock/rock.png", false, false);
Texture blacktxt("textures/Solid_black.png", false, false);

//================================================

void rendershadow(Shader &shader);
void shadowResolution(int type, int res);
void renderQuad();

int main() {

	double fpslastTime = glfwGetTime();
	int nbFrames = 0;


	//----------------------------------------------//
	//            SETTING UP IMGUI                  //
	//----------------------------------------------//

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	//----------------------------------------------//
	//            SETTING UP TEXTURES               //
	//----------------------------------------------//

	lightshaderobj.use();
	lightshaderobj.setInt("material.texture_diffuse", 0);
	lightshaderobj.setInt("material.texture_specular", 1);

    //---------------------------------------------//
    //           BUFFERS SETUP & VAO1              //
    //---------------------------------------------//

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO1);

    float vertices[] = {
		 0.9f,  0.9f, 0.0f, 1.0f, 1.0f,  // top right
		 0.9f,  0.0f, 0.0f, 1.0f, 0.0f,  // bottom right
		-0.2f,  0.0f, 0.0f, 0.0f, 0.0f,  // bottom left
		-0.2f,  0.9f, 0.0f, 0.0f, 1.0f   // top left
	};

	unsigned int indices[] = {
		// note that we start from 0!
		0, 1, 3, // first triangle
		1, 2, 3 // second triangle
	};

	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAO1);
	glGenVertexArrays(1, &VAO2);
	glGenVertexArrays(1, &VAO3);
	glGenVertexArrays(1, &VAO4);
	glGenVertexArrays(1, &VAO5);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	//---------------------//
	//        VAO2         //
	//---------------------//

	glBindVertexArray(VAO1);

	float triangle[] = {

		//VERTICES            //NORMAL            //TEXCOORDS
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f

	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	//------------------------------------//
	//        VAO3 (LIGHT SOURCE)         //
	//------------------------------------//

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	//------------------------------------//
	//        VAO4 (TRANSPARENT)          //
	//------------------------------------//

	float transparentVertices[] = {
		// positions         // texCoords (swapped y coordinates because texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	std::vector<glm::vec3> windowspos
	{
		glm::vec3(6.5f, 0.0f, -14.48f),
		glm::vec3(7.0f, 0.0f, -13.51f),
		glm::vec3(6.1f, 0.0f, -11.7f),
		glm::vec3(7.3f, 0.0f, -10.3f)
	};

	glBindVertexArray(VAO3);
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
	glBindVertexArray(0);

	//------------------------------------//
	//        VAO4 (STATIC QUAD)          //
	//------------------------------------//

	float quadVertices[] = {
		 //positions  //texCoords
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f
	};

	glBindVertexArray(VAO4);
	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	//-------------------------------//
	//         VAO5 (SKYBOX)         //
	//-------------------------------//

	const int cols = 8;
	const int rows = 36;
	const int bsize = rows * cols;
	float skyboxVertices[bsize];

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			int index = i * cols + j;
			if (j < cols - 2) {
				skyboxVertices[index] = triangle[index] * 2.0f;
			}
			else {
				skyboxVertices[index] = triangle[index];
			}
		}
	}

	glBindVertexArray(VAO5);
	glGenBuffers(1, &VBO4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	//----------------------------------------------//
	//            FRAMEBUFFER (POST FX)             //
	//----------------------------------------------//

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &colorbuffer);
	glBindTexture(GL_TEXTURE_2D, colorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::FBO_FX::Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//----------------------------------------------------//
	//            FRAMEBUFFER (MULTISAMPLING)             //
	//----------------------------------------------------//

	glGenFramebuffers(1, &FBO_MSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_MSAA);

	glGenTextures(1, &colorbuffer_MSAA);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorbuffer_MSAA);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, 800, 600, GL_TRUE);
	glBindTexture(GL_TEXTURE_BINDING_2D_MULTISAMPLE, 0);

	glGenRenderbuffers(1, &RBO_MSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO_MSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorbuffer_MSAA, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_MSAA);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::FBO_MSAA::Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//------------------------------------------------//
	//            FRAMEBUFFER (DEPTH MAP)             //
	//------------------------------------------------//

	glGenFramebuffers(1, &FBO_depth);

	SHADOW_HEIGHT = 2048, SHADOW_WIDTH = 2048;

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
			SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO_depth);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//----------------------------------------------------//
	//             FRAMEBUFFER (POINT SHADOWS)            //
	//----------------------------------------------------//

	PSHADOW_HEIGHT = 1024, PSHADOW_WIDTH = 1024;

	glGenFramebuffers(1, &FBO_pshadow);
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				PSHADOW_WIDTH, PSHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_pshadow);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	    //GL_TEXTURE_CUBE_MAP_POSITIVE_X, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//----------------------------------------//
	//             UNIFORM BUFFERS            //
	//----------------------------------------//

	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

	unsigned int env_index = glGetUniformBlockIndex(envmapping.ID, "matrices");
	unsigned int lobj_index = glGetUniformBlockIndex(lightshaderobj.ID, "matrices");
	unsigned int lobji_index = glGetUniformBlockIndex(lightshaderobj_instanced.ID, "matrices");
	glUniformBlockBinding(envmapping.ID, env_index, 0);
	glUniformBlockBinding(lightshaderobj.ID, lobj_index, 0);
	glUniformBlockBinding(lightshaderobj_instanced.ID, lobji_index, 0);

	//-----------------------------------//
	//           VAO6 (ASTROIDS)         //
	//-----------------------------------//

	unsigned int amount = 2000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime());
	float radius = 50.0f;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);

		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(0.0f, 75.0f, 0.0f));
		model = glm::translate(model, glm::vec3(x, y, z));

		//Scale between 0.05 and 0.25
		float scale = (rand() % 20) / 100.0f + 0.05f;
		model = glm::scale(model, glm::vec3(scale));

		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		modelMatrices[i] = model;
	}

	glGenBuffers(1, &VBO5);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rock.meshes.size(); i++)
	{
		unsigned int VAO = rock.meshes[i].VAO;
		glBindVertexArray(VAO);

		std::size_t v4s = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * v4s, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * v4s, (void*) (1*v4s));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * v4s, (void*) (2*v4s));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * v4s, (void*) (3*v4s));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

	//-------------------------------------------//
	//            LOADING AUDIO                  //
	//-------------------------------------------//

	audio.loadSound("aud00", "audio/kangaenaiyounisuru.wav", true);
	audio.loadSound("aud01", "audio/honjitsu_.wav", true);
	audio.loadSound("aud02", "audio/24jikan.wav", true);
	audio.playSound("aud00", 1.0f);
	audio.playSound("aud01", 1.0f);
	audio.playSound("aud02", 1.0f);

    //------------------------------//
    //         RENDER LOOP          //
    //------------------------------//

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		double currentTime = glfwGetTime();
		char title[256];
		nbFrames++;
		if (currentTime - fpslastTime >= 1.0) { // If last print was more than 1 sec ago
			sprintf(title, "FPS : %d", nbFrames);
			//glfwSetWindowTitle(window, title);
			nbFrames = 0;
			fpslastTime += 1.0;
		}

		int wheight, wwidth;
		glfwGetWindowSize(window, &wwidth, &wheight);
		if (wheight == 0) wheight = 1;
		float aspect = (float)wwidth / (float)wheight;

		if(isVsyncOn)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		if (debug_fb_srgb)
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);

		//plightPos[0].z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

		glBindTexture(GL_TEXTURE_2D, colorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wwidth, wheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wwidth, wheight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, colorbuffer_MSAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa_current, GL_RGBA16F, wwidth, wheight, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO_MSAA);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa_current, GL_DEPTH24_STENCIL8, wwidth, wheight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		float currentFrame = glfwGetTime();
		deltatime = currentFrame - lasttime;
		lasttime = currentFrame;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		audio.mute(isMuted);
		audio.setMasterVolume(vol);
		audio.setListenerPos(camPos.x, camPos.y, camPos.z);
		audio.setListenerOrienv(camFront, camUp);

		//For input handling
		if (!io.WantCaptureMouse) {
			processinput(window);
		}

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(camPos, camPos + camFront, camUp);
		projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

		//////////////////////////////////////////////////////////////////////////////////////////////
		//DIR SHADOW DEPTH CALCULATION
		//////////////////////////////////////////////////////////////////////////////////////////////

		float near_plane = 1.0f, far_plane = 200.0f;//7.5f;
		glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
		glm::vec3 lightDir = glm::normalize(glm::vec3(0.01f, -1.0f, 0.0f));
		glm::mat4 lightView = glm::lookAt(-lightDir * 20.0f, glm::vec3(0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		depthshader.use();
		depthshader.setMat4("LightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_depth);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		rendershadow(depthshader);
		//glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, wwidth, wheight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBindTexture(GL_TEXTURE_2D, depthMap);

		//////////////////////////////////////////////////////////////////////////////////////////////
		//POINT SHADOWS DEPTH CALCULATION
		//////////////////////////////////////////////////////////////////////////////////////////////

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		near_plane = 1.0f;
		far_plane = 25.0f;

		glm::mat4 pshadowProj = glm::perspective(glm::radians(90.0f),
				(float) PSHADOW_WIDTH / (float) PSHADOW_HEIGHT, near_plane, far_plane);

		std::vector<glm::mat4> pshadowTransforms;

		pshadowTransforms.clear();
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		pshadowTransforms.push_back(pshadowProj * glm::lookAt(plightPos[0], plightPos[0] + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

		glViewport(0, 0, PSHADOW_WIDTH, PSHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_pshadow);
		glClear(GL_DEPTH_BUFFER_BIT);
		pshadow_depth.use();

		for(int i = 0; i < 6; ++i)
		{
			pshadow_depth.setMat4("shadowMatrices[" + std::to_string(i) + "]", pshadowTransforms[i]);
		}

		pshadow_depth.setFloat("far_plane", far_plane);
		pshadow_depth.setVec3("lightPos", plightPos[0]);

		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LEQUAL);
		if(debugPshadow)
			/*std::cout << "viewport: " << PSHADOW_WIDTH << "x" << PSHADOW_HEIGHT << std::endl;
			GLint vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			std::cout << "actual viewport: " << vp[0] << " " << vp[1] << " " << vp[2] << " " << vp[3] << std::endl;
			GLint drawFBO;
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);
			std::cout << "bound FBO: " << drawFBO << " expected: " << FBO_pshadow << std::endl;*/
			rendershadow(pshadow_depth);
		//glDepthFunc(GL_LESS);

		/*float pixel = 0.0f;
		glReadPixels(512, 512, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixel);
		std::cout << "depth pixel: " << pixel << std::endl;

		GLint numShaders = 0;
		glGetProgramiv(pshadow_depth.ID, GL_ATTACHED_SHADERS, &numShaders);
		std::cout << "attached shaders: " << numShaders << std::endl;*/

		/*
		//test==============
		pshadow_depth.setFloat("far_plane", far_plane);
		pshadow_depth.setVec3("lightPos", plightPos[0]);

		GLenum faces[6] = {
		    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};

		for (int face = 0; face < 6; ++face)
		{
		    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		        faces[face], depthCubemap, 0);
		    glClear(GL_DEPTH_BUFFER_BIT);
		    pshadow_depth.setMat4("lightSpaceMatrix", pshadowTransforms[face]);
		    //std::cout << "lightSpaceMatrix[0][0]: " << pshadowTransforms[face][0][0] << std::endl;
		    if(debugPshadow)
		    	rendershadow(pshadow_depth);
		}

		for (int face = 0; face < 6; ++face)
		{
		    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, faces[face], depthCubemap, 0);
		    float pixel = 1.0f;
		    glReadPixels(512, 512, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixel);
		    //std::cout << "face " << face << " depth: " << pixel << std::endl;
		}
		//================================
		*/

		/*glm::mat4 m = pshadowTransforms[0];
		printf("face0 matrix:\n");
		printf("%.3f %.3f %.3f %.3f\n", m[0][0], m[1][0], m[2][0], m[3][0]);
		printf("%.3f %.3f %.3f %.3f\n", m[0][1], m[1][1], m[2][1], m[3][1]);
		printf("%.3f %.3f %.3f %.3f\n", m[0][2], m[1][2], m[2][2], m[3][2]);
		printf("%.3f %.3f %.3f %.3f\n", m[0][3], m[1][3], m[2][3], m[3][3]);*/
		//==================

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, wwidth, wheight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		//////////////////////////////////////////////////////////////////////////////////////////////


		//For rendering commands
		if (msaa_current != 0)
			glBindFramebuffer(GL_FRAMEBUFFER, FBO_MSAA);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (isWireframeOn)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container2txt.ID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, container2txt_specular.ID);

		lightshaderobj.use();
		lightshaderobj.setFloat("far_plane", far_plane);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		lightshaderobj.setInt("pDepthMap" , 5); // fix

		lightshaderobj.setBool("blinn", isBlinn);
		lightshaderobj.setMat4("model", model);
		lightshaderobj.setVec3("viewPos", camPos);
		lightshaderobj.setFloat("material.shininess", 32.0f);
		lightshaderobj.setFloat("time", sin(glfwGetTime()) * 4.0f);
		lightshaderobj.setMat4("LightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		lightshaderobj.setInt("shadowMap", 4);
		lightshaderobj.setInt("normalMap", 7);
		lightshaderobj.setBool("useNormalMap", false);
		lightshaderobj.setBool("useParallaxMap", false);
		lightshaderobj.setInt("parallaxDepthMap", 8);
		lightshaderobj.setInt("debug_parallaxDepthMap", 9);
		lightshaderobj.setFloat("height_scale", 0.1f);

		lightshaderobj.setVec3f("dirlight.direction", 0.01f, -1.0f, 0.0f);
		lightshaderobj.setVec3f("dirlight.ambient",  0.00f, 0.00f, 0.00f);//*/0.01f, 0.01f, 0.01f); //0.01
		lightshaderobj.setVec3f("dirlight.diffuse",  0.2f, 0.2f, 0.2f); // .2
		lightshaderobj.setVec3f("dirlight.specular", 0.0f, 0.0f, 0.0f);

		lightshaderobj.setFloat("pLight[0].constant", 1.0f);
		lightshaderobj.setFloat("pLight[0].linear", 0.09f);
		lightshaderobj.setFloat("pLight[0].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[0].ambient", 0.05f, 0.05f, 0.05f); // default 0.05
		lightshaderobj.setVec3f("pLight[0].diffuse", 0.6f, 0.6f, 0.6f);
		lightshaderobj.setVec3f("pLight[0].specular", 0.3f, 0.3f, 0.3f);
		lightshaderobj.setVec3("pLight[0].position", plightPos[0]);

		lightshaderobj.setFloat("pLight[1].constant", 1.0f);
		lightshaderobj.setFloat("pLight[1].linear", 0.09f);
		lightshaderobj.setFloat("pLight[1].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[1].ambient", 0.05f, 0.05f, 0.05f);
		lightshaderobj.setVec3f("pLight[1].diffuse", 0.8f, 0.8f, 0.8f);
		lightshaderobj.setVec3f("pLight[1].specular", 0.3f, 0.3f, 0.3f);
		lightshaderobj.setVec3("pLight[1].position", plightPos[1]);

		lightshaderobj.setFloat("pLight[2].constant", 1.0f);
		lightshaderobj.setFloat("pLight[2].linear", 0.09f);
		lightshaderobj.setFloat("pLight[2].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[2].ambient", 0.05f, 0.05f, 0.05f);
		lightshaderobj.setVec3f("pLight[2].diffuse", 0.4f, 0.0f, 1.0f);
		lightshaderobj.setVec3f("pLight[2].specular", 0.4f, 0.0f, 1.0f);
		lightshaderobj.setVec3("pLight[2].position", plightPos[2]);

		lightshaderobj.setFloat("pLight[3].constant", 1.0f);
		lightshaderobj.setFloat("pLight[3].linear", 0.09f);
		lightshaderobj.setFloat("pLight[3].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[3].ambient", 0.05f, 0.05f, 0.05f);
		lightshaderobj.setVec3f("pLight[3].diffuse", 0.5f, 0.0f, 0.0f);
		lightshaderobj.setVec3f("pLight[3].specular", 0.6f, 0.6f, 0.6f);
		lightshaderobj.setVec3("pLight[3].position", plightPos[3]);

		lightshaderobj.setFloat("pLight[4].constant", 1.0f);
		lightshaderobj.setFloat("pLight[4].linear", 0.09f);
		lightshaderobj.setFloat("pLight[4].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[4].ambient", 0.05f, 0.05f, 0.05f);
		lightshaderobj.setVec3f("pLight[4].diffuse", 0.5f, 0.5f, 0.5f);
		lightshaderobj.setVec3f("pLight[4].specular", 0.6f, 0.6f, 0.6f);
		lightshaderobj.setVec3("pLight[4].position", plightPos[4]);

		lightshaderobj.setFloat("pLight[5].constant", 1.0f);
		lightshaderobj.setFloat("pLight[5].linear", 0.09f);
		lightshaderobj.setFloat("pLight[5].quadratic", 0.032f);
		lightshaderobj.setVec3f("pLight[5].ambient", 0.05f, 0.05f, 0.05f);
		lightshaderobj.setVec3f("pLight[5].diffuse", 0.5f, 0.5f, 0.5f);
		lightshaderobj.setVec3f("pLight[5].specular", 0.6f, 0.6f, 0.6f);
		lightshaderobj.setVec3("pLight[5].position", plightPos[5]);

		lightshaderobj.setFloat("spotlight.constant", 1.0f);
		lightshaderobj.setFloat("spotlight.linear", 0.45f);
		lightshaderobj.setFloat("spotlight.quadratic", 0.0075f);
		lightshaderobj.setVec3f("spotlight.ambient", 0.0f, 0.0f, 0.0f);
		lightshaderobj.setVec3f("spotlight.diffuse", 0.0f, 0.0f, 0.0f);
		lightshaderobj.setVec3f("spotlight.specular", 0.0f, 0.0f, 0.0f);
		lightshaderobj.setVec3("spotlight.position", camPos);
		lightshaderobj.setVec3("spotlight.direction", camFront);
		lightshaderobj.setFloat("spotlight.cutoff", glm::cos(glm::radians(12.5f)));
		lightshaderobj.setFloat("spotlight.outercutoff", glm::cos(glm::radians(17.5f)));

		if (flashlight_state == 1) {
			lightshaderobj.setVec3f("spotlight.ambient",  0.2f, 0.2f, 0.2f);
			lightshaderobj.setVec3f("spotlight.diffuse",  1.0f, 1.0f, 1.0f);
			lightshaderobj.setVec3f("spotlight.specular", 1.0f, 1.0f, 1.0f);
		}
		else if (flashlight_state == 2) {
			lightshaderobj.setVec3f("spotlight.ambient",  0.05f, 0.05f, 0.05f);
			lightshaderobj.setVec3f("spotlight.diffuse",  0.4f, 0.4f, 0.4f);
			lightshaderobj.setVec3f("spotlight.specular", 0.5f, 0.5f, 0.5f);
		}
		else if (flashlight_state == 0){
			lightshaderobj.setVec3f("spotlight.ambient",  0.0f, 0.0f, 0.0f);
			lightshaderobj.setVec3f("spotlight.diffuse",  0.0f, 0.0f, 0.0f);
			lightshaderobj.setVec3f("spotlight.specular", 0.0f, 0.0f, 0.0f);
		}

		glBindVertexArray(0);
		glBindVertexArray(VAO1);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		int cubenum = sizeof(cubePositions) / sizeof(cubePositions[0]);

		for (unsigned int i = 0; i < cubenum; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, (cubePositions[i]));
			float angle = 20.0f * i;
			if (i % 3 == 0){
				angle = glfwGetTime() * 90.0f;
			}
			model = glm::rotate(model, glm::radians(/*(float)sin(glfwGetTime()) * 180*/angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lightshaderobj.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//nmap_object
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -4.0f));
		model = glm::rotate(model, glm::radians(60.0f),
							glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(10.0f),
							glm::vec3(1.0f, 0.0f, 0.0f));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bricktxt.ID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, whitetxt.ID);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, bricktxtnorm.ID);
		lightshaderobj.setMat4("model", model);
		if (debug_normalmapping)
			lightshaderobj.setBool("useNormalMap", true);
		renderQuad();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(3.0f, 1.0f, -6.0f));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bricks2.ID);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, bricks2_normal.ID);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, bricks2_disp.ID);
		lightshaderobj.setMat4("model", model);
		if (debug_parallaxmapping)
			lightshaderobj.setBool("useParallaxMap", true);
		renderQuad();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 1.0f, -8.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, glm::radians(45.0f),
							glm::vec3(0.0f, -1.0f, 0.0f));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, toybox_diff.ID);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, toybox_norm.ID);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, toybox_disp.ID);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, toybox_disp.ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		lightshaderobj.setMat4("model", model);
		if (debug_parallaxmapping)
			lightshaderobj.setBool("useParallaxMap", true);
		renderQuad();
		lightshaderobj.setFloat("height_scale", 0.1f);

		lightshaderobj.setBool("useNormalMap", false);
		lightshaderobj.setBool("useParallaxMap", false);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 73.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 96.0f), glm::vec3(0.3f, 0.7f, 0.0f));
		lightshaderobj.setMat4("model", model);
		planet.Draw(lightshaderobj);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-6.0f, 1.0f, -6.0f));
		model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		lightshaderobj.setMat4("model", model);
		backpack.Draw(lightshaderobj);

		model = glm::translate(model, glm::vec3(-7.0f, 1.0f, -9.0f));
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		lightshaderobj.setMat4("model", model);
		oillamp.Draw(lightshaderobj);

		model = glm::translate(model, glm::vec3(-30.0f, -11.7f, -30.0f));
		lightshaderobj.setMat4("model", model);
		lightshaderobj.setFloat("material.shininess", 128.0f);
		plane00.draw(lightshaderobj);

		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glBindVertexArray(VAO1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, naonao.ID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
		lightshaderobj.setMat4("model", model);
		audio.updateSoundPos("aud00", extractPosition(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		//glDisable(GL_DEPTH_TEST);
		singlecolor.use();
		model = glm::scale(model, glm::vec3(1.05f, 1.05f, 1.05f));
		singlecolor.setMat4("view", view);
		singlecolor.setMat4("projection", projection);
		singlecolor.setMat4("model", model);
		singlecolor.setVec3f("color", sin(glfwGetTime()) * 0.4 + 0.6, cos(glfwGetTime()) * 0.5 + 0.5, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		//glEnable(GL_DEPTH_TEST);

		glBindVertexArray(VAO3);
		transparent.use();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f));
		transparent.setMat4("view", view);
		transparent.setMat4("model", model);
		transparent.setMat4("projection", projection);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, kiryutxt.ID);
		transparent.setInt("texture1", 1);
		audio.updateSoundPos("aud01", extractPosition(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		model = glm::translate(model, glm::vec3(20.0f, 0.0f, 20.0f));
		model = glm::translate(model, glm::vec3((float)sin(glfwGetTime()) * 5, 0.0f, (float)cos(glfwGetTime()) * 5));
		model = glm::scale(model, glm::vec3(1.3f, 3.0f, 1.0f));
		transparent.setMat4("model", model);
		audio.updateSoundPos("aud02", extractPosition(model));
		glBindTexture(GL_TEXTURE_2D, majimatxt.ID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		/*lightshaderobj_instanced.use();
		lightshaderobj_instanced.setInt("material.texture_diffuse", 0);
		lightshaderobj_instanced.setInt("material.texture_specular", 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rocktxt.ID);//rock.textures_loaded[0].id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blacktxt.ID);
		for (unsigned int i = 0; i < rock.meshes.size(); i++)
		{
			glBindVertexArray(rock.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()),
				GL_UNSIGNED_INT, 0, amount);
			glBindVertexArray(0);
		}*/

		glBindVertexArray(VAO2);

		int plightnum = sizeof(plightPos) / sizeof(plightPos[0]);

		lightshader.use();
		lightshader.setMat4("projection", projection);
		lightshader.setMat4("view", view);

		for (int i = 0; i < plightnum; i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, plightPos[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			lightshader.setMat4("model", model);
			if (i == 2) {
				lightshader.setBool("colored", true);
				lightshader.setVec3f("color", 0.4f, 0.0f, 1.0f);
			} else {
				lightshader.setBool("colored", false);
			}
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		model = glm::mat4(1.0f);
		model = glm::translate(model, sun);
		model = glm::scale(model, glm::vec3(2.0f));
		lightshader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

		glBindVertexArray(VAO1);
		envmapping.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 7.0f, -20.0f));
		envmapping.setMat4("model", model);
		envmapping.setVec3("camPos", camPos);
		envmapping.setInt("skybox", 0);
		envmapping.setInt("mode", 1);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDisable(GL_PROGRAM_POINT_SIZE);
		model = glm::translate(model, glm::vec3(0.0f, 6.0f, 5.0f));
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		envmapping.setMat4("model", model);
		envmapping.setInt("mode", 2);
		backpack.Draw(envmapping);

		glDepthFunc(GL_LEQUAL);

		skyboxshader.use();
		glBindVertexArray(VAO5);
		glm::mat4 skyview = glm::mat4(glm::mat3(view));
		skyboxshader.setMat4("view", skyview);
		skyboxshader.setMat4("projection", projection);
		if (isSkyboxOn)
			glDrawArrays(GL_TRIANGLES, 0, 36);

		glDepthFunc(GL_LESS);

		glBindVertexArray(VAO3);

		shader00.use();
		shader00.setMat4("view", view);
		shader00.setMat4("projection", projection);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, windowtxt.ID);
		shader00.setInt("ourTexture", 1);

		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windowspos.size(); i++) {
			float distance = glm::length(camPos - windowspos[i]);
			sorted[distance] = windowspos[i];
		}

		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			shader00.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (msaa_current != 0) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO_MSAA);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
			glBlitFramebuffer(0, 0, wwidth, wheight, 0, 0, wwidth, wheight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		shader01.use();
		shader01.setFloat("width", (float)wwidth);
		shader01.setInt("fbmode", filter_current);
		shader01.setBool("hdr", debug_hdr);
		shader01.setFloat("exposure", hdr_exposure);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorbuffer);
		glBindVertexArray(VAO4);
		//glEnable(GL_FRAMEBUFFER_SRGB);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDisable(GL_FRAMEBUFFER_SRGB);


		if (toggleMenu) {
			ImGui::SetNextWindowBgAlpha(f);
			ImGui::Begin("Menu");
			ImGui::Text("Resolution: %dx%d", wwidth, wheight);
			ImGui::Text(title);
			ImGui::Checkbox("Skybox", &isSkyboxOn);
			ImGui::Checkbox("Wireframe Mode", &isWireframeOn);
			ImGui::Checkbox("Blinn-Phong", &isBlinn);
			ImGui::Checkbox("Mute", &isMuted);
			ImGui::Checkbox("Vsync", &isVsyncOn);
			ImGui::Checkbox("debug_pshadow", &debugPshadow);
			ImGui::Checkbox("debug_normalmapping", &debug_normalmapping);
			ImGui::Checkbox("debug_parallaxmapping", &debug_parallaxmapping);
			ImGui::Checkbox("GL_FRAMEBUFFER_SRGB", &debug_fb_srgb);
			ImGui::Checkbox("debug_hdr", &debug_hdr);
			if (ImGui::Combo("Themes", &theme_current, themes, IM_ARRAYSIZE(themes))) {
				switch (theme_current) {
				case 0: ImGui::StyleColorsClassic(); break;
				case 1: ImGui::StyleColorsDark(); break;
				case 2: ImGui::StyleColorsLight(); break;
				}
			}
			if (ImGui::Combo("Filters", &filter_current, filters, IM_ARRAYSIZE(filters))) {
				switch (filter_current) {
				case 0: filter_current = 0; break; case 1: filter_current = 1; break; case 2: filter_current = 2; break;
				case 3: filter_current = 3; break; case 4: filter_current = 4; break; case 5: filter_current = 5; break;
				case 6: filter_current = 6; break; case 7: filter_current = 7; break;
				}
			}
			if (ImGui::Combo("MSAA", &msaa_list, msaa, IM_ARRAYSIZE(msaa))) {
				switch (msaa_list) {
				case 0: msaa_current = 0; break; case 1: msaa_current = 2; break; case 2: msaa_current = 4; break;
				case 3: msaa_current = 8; break; case 4: msaa_current = 16; break;
				}
			}
			if (ImGui::Combo("Shadow Map", &shadow_current, shadow_res, IM_ARRAYSIZE(shadow_res))) {
				switch (shadow_current) {
				case 0: shadow_current = 0; shadowResolution(SET_SHADOW_RES, 1024); break;
				case 1: shadow_current = 1;	shadowResolution(SET_SHADOW_RES, 2048); break;
				case 2: shadow_current = 2; shadowResolution(SET_SHADOW_RES, 4096); break;
				}
			}
			if (ImGui::Combo("PShadow Map", &pshadow_current, pshadow_res, IM_ARRAYSIZE(pshadow_res))) {
				switch (pshadow_current) {
				case 0: pshadow_current = 0; shadowResolution(SET_PSHADOW_RES, 1024); break;
				case 1: pshadow_current = 1; shadowResolution(SET_PSHADOW_RES, 2048); break;
				case 2: pshadow_current = 2; shadowResolution(SET_PSHADOW_RES, 4096); break;
				}
			}
			ImGui::SliderFloat("hdr_exposure", &hdr_exposure, 0.0f, 10.0f);
			ImGui::SliderFloat("Opacity", &f, 0.0f, 1.0f);
			ImGui::SliderFloat("Volume", &vol, 0.0f, 1.0f);
			if (ImGui::Button("Hide"))
				toggleMenu = false;
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//Event polling and Buffer Swapping
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO1);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &VBO3);
	glDeleteBuffers(1, &VBO4);
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VAO1);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteVertexArrays(1, &VAO3);
	glDeleteVertexArrays(1, &VAO4);
	glDeleteVertexArrays(1, &VAO5);
	glDeleteFramebuffers(1, &FBO);
	glDeleteFramebuffers(1, &FBO_MSAA);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	audio.shutdown();
	glfwTerminate();
	return 0;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int cubemapID;
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

	/*
	__HDR DEBUG NOTE__
	SWITCH GAMMA CORRECTION ON FOR DIFFUSE/ALBEDO TEXTURES
	EVERYTHING ELSE STAYS OFF!!!
	*/

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		GLenum cmformat = GL_RGB;
		if (nrChannels == 1)
			cmformat = GL_RED;
		else if (nrChannels == 3)
			cmformat = GL_RGB;
		else if (nrChannels == 4)
			cmformat = GL_RGBA;

        if (data) {
            //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
            //             width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            
			if (nrChannels == 3)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, 
					width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 4)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB_ALPHA,
					width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			
			stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap failed to load at path: " << faces[i]
                << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemapID;
}

void rendershadow(Shader &shader){
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);

	glBindVertexArray(VAO1);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	int cubenum = sizeof(cubePositions) / sizeof(cubePositions[0]);

	for (unsigned int i = 0; i < cubenum; i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, (cubePositions[i]));
		float angle = 20.0f * i;
		if (i % 3 == 0){
			angle = glfwGetTime() * 90.0f;
		}
		model = glm::rotate(model, glm::radians(/*(float)sin(glfwGetTime()) * 180*/angle), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
	model = glm::scale(model, glm::vec3(1.05f, 1.05f, 1.05f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 73.0f, 0.0f));
	model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
	model = glm::rotate(model, glm::radians((float)glfwGetTime() * 96.0f), glm::vec3(0.3f, 0.7f, 0.0f));
	shader.setMat4("model", model);
	planet.Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
	model = glm::translate(model, glm::vec3(-6.0f, 1.0f, -6.0f));
	model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setMat4("model", model);
	backpack.Draw(shader);

	model = glm::translate(model, glm::vec3(-7.0f, 1.0f, -9.0f));
	model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setMat4("model", model);
	oillamp.Draw(shader);

	model = glm::translate(model, glm::vec3(-30.0f, -11.7f, -30.0f));
	shader.setMat4("model", model);
	plane00.draw(shader);


	//int plightnum = sizeof(plightPos) / sizeof(plightPos[0]);

	/*for (int i = 0; i < plightnum; i++) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, plightPos[i]);
		model = glm::scale(model, glm::vec3(0.2f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	model = glm::mat4(1.0f);
	model = glm::translate(model, sun);
	model = glm::scale(model, glm::vec3(2.0f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);*/
};

void shadowResolution(int type, int res)
{ 
	if (type == SET_SHADOW_RES) {
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, res,
					 res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		SHADOW_HEIGHT = res, SHADOW_WIDTH = res;
	} else if (type == SET_PSHADOW_RES) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
						 GL_DEPTH_COMPONENT, res, res, 0,
						 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		PSHADOW_HEIGHT = res, PSHADOW_WIDTH = res;
	}
}



void renderQuad() {
	if (quadVAO == 0) {
		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent
			// // bitangent
			pos1.x,		  pos1.y,		pos1.z,		  nm.x,
			nm.y,		  nm.z,			uv1.x,		  uv1.y,
			tangent1.x,	  tangent1.y,	tangent1.z,	  bitangent1.x,
			bitangent1.y, bitangent1.z, pos2.x,		  pos2.y,
			pos2.z,		  nm.x,			nm.y,		  nm.z,
			uv2.x,		  uv2.y,		tangent1.x,	  tangent1.y,
			tangent1.z,	  bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x,		  pos3.y,		pos3.z,		  nm.x,
			nm.y,		  nm.z,			uv3.x,		  uv3.y,
			tangent1.x,	  tangent1.y,	tangent1.z,	  bitangent1.x,
			bitangent1.y, bitangent1.z,

			pos1.x,		  pos1.y,		pos1.z,		  nm.x,
			nm.y,		  nm.z,			uv1.x,		  uv1.y,
			tangent2.x,	  tangent2.y,	tangent2.z,	  bitangent2.x,
			bitangent2.y, bitangent2.z, pos3.x,		  pos3.y,
			pos3.z,		  nm.x,			nm.y,		  nm.z,
			uv3.x,		  uv3.y,		tangent2.x,	  tangent2.y,
			tangent2.z,	  bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x,		  pos4.y,		pos4.z,		  nm.x,
			nm.y,		  nm.z,			uv4.x,		  uv4.y,
			tangent2.x,	  tangent2.y,	tangent2.z,	  bitangent2.x,
			bitangent2.y, bitangent2.z};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
					 GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
							  (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
							  (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
							  (void *)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
							  (void *)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
							  (void *)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}