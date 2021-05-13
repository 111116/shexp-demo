/***********************************************************
* A spherical harmonics computation example and playground *
* no warranty implied | use at your own risk               *
* author: Andreas Mantler (ands) | last change: 13.04.2018 *
*                                                          *
* License:                                                 *
* This software is in the public domain.                   *
* Where that dedication is not recognized,                 *
* you are granted a perpetual, irrevocable license to copy *
* and modify this file however you want.                   *
***********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

extern "C"
{
	#define YO_IMPLEMENTATION
	#define YO_NOIMG
	#include "yocto_obj.h"
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_impl_glfw_gl3.cpp"

#include "m_math.h"
#include "s_shader.h"

#include "consolelog.hpp"
#include "sh/sh.hpp"
#include "loadlut.hpp"
#include "loadspheres.hpp"
#include "loadgamma.hpp"
#include "shlut.hpp"
#include "display_texture.hpp"
#include "receiver_cluster.hpp"
#include "arealight.hpp"


const float FOV = 30.0f;
const char obj_file[] = "../res/hifreq_fixed.obj";
const char light_obj_file[] = "../res/light.obj";
// const char sphere_file[] = "../res/fence4-200.sph";
// const char sphere_file[] = "../res/ball-200.sph";
const char sphere_file[] = "../res/hifreq_scene.sph";
const char sh_light_file[] = "../res/andi.shrgb";
const char vert_shader_path[] = "../res/vert_sh3.glsl";
// const char vert_shader_path[] = "../res/vert_show_clusterid.glsl";
const char frag_shader_path[] = "../res/frag.glsl";
// const char frag_shader_path[] = "../res/frag_show_normal.glsl";
// const char frag_shader_path[] = "../res/frag_show_tessellation.glsl";
m_vec3 obj_color[10] = {{1,1,1},{0.5,1,0.5},{1,0,0},{0,0,1}};
m_vec3 light_color = {1,1,1};

typedef struct
{
	struct
	{
		GLuint program;
		GLint u_view;
		GLint u_projection;

		GLuint LHtexture;
		GLuint vao, vbo;
		int vertices;

		float max_magn;
		int gammasize;
	} mesh;
} scene_t;


std::string readfile(const char filename[]) {
	std::stringstream sstream;
	sstream << std::ifstream(filename).rdbuf();
	if (!sstream)
		throw "file not found";
	std::string sourceString = sstream.str();
	return sourceString;
}


static void initScene(scene_t *scene)
{
	SphereTree hierarchy = load_sphere_hierarchy(sphere_file);
	loadlut(3, scene->mesh.max_magn);
	build_sh_lut();
	scene->mesh.gammasize = upload_gamma(shorder);
	console.log("gamma size:", scene->mesh.gammasize);

	// mesh
	console.log("loading scene...");
	yo_scene *yo = yo_load_obj(obj_file, true, false);
	if (!yo || !yo->nshapes)
		throw "Error loading obj file";
	console.log(yo->nshapes, "shapes");

	scene->mesh.vertices = 0;
	for (int i = 0; i < yo->nshapes; i++)
		scene->mesh.vertices += yo->shapes[i].nelems * 3;

	// alloc host space for attributes
	m_vec3 *positions = (m_vec3*)calloc(scene->mesh.vertices, sizeof(m_vec3));
	m_vec3 *normals   = (m_vec3*)calloc(scene->mesh.vertices, sizeof(m_vec3));
	int *objids       = (int*)calloc(scene->mesh.vertices, sizeof(int));
	int *clusterids   = (int*)calloc(scene->mesh.vertices, sizeof(int));
	int *sphcnt   = (int*)calloc(scene->mesh.vertices, sizeof(int));
	float *LH = (float*)calloc(scene->mesh.vertices, N_COEFFS * sizeof(float));
	// total size of each attribute
	size_t positionsSize = scene->mesh.vertices * sizeof(m_vec3);
	size_t normalsSize   = scene->mesh.vertices * sizeof(m_vec3);
	size_t objidsSize    = scene->mesh.vertices * sizeof(int);
	size_t clusteridsSize   = scene->mesh.vertices * sizeof(int);
	size_t sphcntSize   = scene->mesh.vertices * sizeof(int);
	size_t LHSize = scene->mesh.vertices * N_COEFFS * sizeof(float);

	// fill positions & normals from scene
	int n = 0;
	for (int i = 0; i < yo->nshapes; i++)
	{
		yo_shape *shape = yo->shapes + i;
		for (int j = 0; j < shape->nelems * 3; j++)
		{
			positions[n + j] = *(m_vec3*)&shape->pos[shape->elem[j] * 3];
			normals[n + j] = *(m_vec3*)&shape->norm[shape->elem[j] * 3];
			objids[n + j] = i;
		}
		n += shape->nelems * 3;
	}
	yo_free_scene(yo);
	// cluster receiver
	cluster_points(scene->mesh.vertices, reinterpret_cast<vec3f*>(positions), clusterids);
	cluster_preprocess(scene->mesh.vertices, reinterpret_cast<vec3f*>(positions), clusterids, hierarchy, sphcnt);
	// preprocess lighting
	calculateLH(scene->mesh.vertices, reinterpret_cast<vec3f*>(positions), reinterpret_cast<vec3f*>(normals), LH, light_obj_file);
	// because openGL doesn't support array as attribute, we upload it as texture
	uploadLH(scene->mesh.vertices, LH);

	console.log("renderer initializing...");
	// upload geometry to opengl
	glGenVertexArrays(1, &scene->mesh.vao);
	glBindVertexArray(scene->mesh.vao);
	// upload attribute data
	glGenBuffers(1, &scene->mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, scene->mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, positionsSize + normalsSize + objidsSize + clusteridsSize + sphcntSize, NULL, GL_STATIC_DRAW);
	unsigned char *buffer = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	assert(buffer);
	memcpy(buffer, positions, positionsSize);
	memcpy(buffer + positionsSize, objids, objidsSize);
	memcpy(buffer + positionsSize, normals, normalsSize);
	memcpy(buffer + positionsSize + normalsSize, objids, objidsSize);
	memcpy(buffer + positionsSize + normalsSize + objidsSize, clusterids, clusteridsSize);
	memcpy(buffer + positionsSize + normalsSize + objidsSize + clusteridsSize, sphcnt, sphcntSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	// set attribute pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)positionsSize);
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)(positionsSize + normalsSize));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, 0, (void*)(positionsSize + normalsSize + objidsSize));
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_INT, 0, (void*)(positionsSize + normalsSize + objidsSize + clusteridsSize));

	// free host space
	free(positions);
	free(normals);
	free(objids);
	free(clusterids);
	free(sphcnt);
	free(LH);

	const char *attribs[] =
	{
		"a_position",
		"a_normal",
		"a_objid",
		"a_clusterid",
		"a_sphcnt"
	};
	scene->mesh.program = s_loadProgram(readfile(vert_shader_path).c_str(), readfile(frag_shader_path).c_str(), attribs, 5);
	if (!scene->mesh.program)
		throw "Error loading mesh shader";
	scene->mesh.u_view = glGetUniformLocation(scene->mesh.program, "u_view");
	scene->mesh.u_projection = glGetUniformLocation(scene->mesh.program, "u_projection");
}

static void drawScene(scene_t *scene, float *view, float *projection)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glDisable(GL_CULL_FACE);

	// mesh
	glUseProgram(scene->mesh.program);
	glUniformMatrix4fv(scene->mesh.u_projection, 1, GL_FALSE, projection);
	glUniformMatrix4fv(scene->mesh.u_view, 1, GL_FALSE, view);
	// textures
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_sh_lut"), 3);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_log_lut"), 1);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_ab_lut"), 2);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_sphere"), 4);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_ratio"), 5);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_sparse"), 15);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "u_LH"), 0);
	// variables
	glUniform1f(glGetUniformLocation(scene->mesh.program, "max_magn"), scene->mesh.max_magn);
	glUniform1i(glGetUniformLocation(scene->mesh.program, "gammasize"), scene->mesh.gammasize);
	// color
	glUniform3fv(glGetUniformLocation(scene->mesh.program, "objcolor"), 10, &obj_color[0].x);
	// vertices
	glBindVertexArray(scene->mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, scene->mesh.vertices);
}

static void destroyScene(scene_t *scene)
{
	// mesh
	glDeleteProgram(scene->mesh.program);
	glDeleteVertexArrays(1, &scene->mesh.vao);
	glDeleteBuffers(1, &scene->mesh.vbo);
	glDeleteTextures(1, &scene->mesh.LHtexture);
}

static void fpsCameraViewMatrix(GLFWwindow *window, float *view, bool ignoreInput);
void saveImage(const char filepath[], GLFWwindow* w);

static void error_callback(int error, const char *description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char* argv[])
{
	bool single_image_mode = (argc>1);
	if (single_image_mode) {
		console.warn("single image mode");
	}
	try {
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow *window = glfwCreateWindow(800, 800, "Spherical Harmonics Playground", NULL, NULL);
	if (!window) return 1;
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(0); // vsync disabled

	ImGui_ImplGlfwGL3_Init(window, true);

	scene_t scene = {0};
	initScene(&scene);
	// display_texture::init();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			destroyScene(&scene);
			initScene(&scene);
		}
		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_FirstUseEver);
		static bool show_another_window = true;
		ImGui::Begin("Coefficients", &show_another_window);
		// for (int i = 0; i < 9; i++)
		// {
		// 	char name[] = "[?]";
		// 	name[1] = i + '0';
		// 	m_vec3 remapped = m_scale3(m_add3(scene.mesh.coefficients[i], m_v3(1.0f, 1.0f, 1.0f)), 0.5f);
		// 	ImGui::ColorEdit3(name, &remapped.x);
		// 	scene.mesh.coefficients[i] = m_sub3(m_scale3(remapped, 2.0f), m_v3(1.0f, 1.0f, 1.0f));
		// }
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glViewport(0, 0, w, h);
		float view[16], projection[16];
		fpsCameraViewMatrix(window, view, ImGui::IsAnyItemActive());
		m_perspective44(projection, FOV, (float)w / (float)h, 0.01f, 100.0f);
		drawScene(&scene, view, projection);
		// display_texture::draw();
		if (!single_image_mode) {
			ImGui::Render();
		}
		glfwSwapBuffers(window);
		if (single_image_mode) {
			saveImage("window_save.png", window);
			break;
		}
	}

	destroyScene(&scene);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	}
	catch (const char* e) {
		console.error(e);
		return 1;
	}
}



void saveImage(const char filepath[], GLFWwindow* w) {
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}



static void fpsCameraViewMatrix(GLFWwindow *window, float *view, bool ignoreInput)
{
	// initial camera config
	static float position[] = { 3.0f, 2.0f, 5.8f };
	static float rotation[] = { -13.5766f, 27.4445f };

	// mouse look
	static double lastMouse[] = { 0.0, 0.0 };
	double mouse[2];
	glfwGetCursorPos(window, &mouse[0], &mouse[1]);
	if (!ignoreInput && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		rotation[0] += (float)(mouse[1] - lastMouse[1]) * -0.2f;
		rotation[1] += (float)(mouse[0] - lastMouse[0]) * -0.2f;
	}
	lastMouse[0] = mouse[0];
	lastMouse[1] = mouse[1];

	float rotationY[16], rotationX[16], rotationYX[16];
	m_rotation44(rotationX, rotation[0], 1.0f, 0.0f, 0.0f);
	m_rotation44(rotationY, rotation[1], 0.0f, 1.0f, 0.0f);
	m_mul44(rotationYX, rotationY, rotationX);

	// keyboard movement (WSADEQ)
	float speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 0.1f : 0.01f;
	float movement[3] = { 0 };
	if (!ignoreInput)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement[2] -= speed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement[2] += speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement[0] -= speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement[0] += speed;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) movement[1] -= speed;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) movement[1] += speed;
	}

	float worldMovement[3];
	m_transform44(worldMovement, rotationYX, movement);
	position[0] += worldMovement[0];
	position[1] += worldMovement[1];
	position[2] += worldMovement[2];

	// construct view matrix
	float inverseRotation[16], inverseTranslation[16];
	m_transpose44(inverseRotation, rotationYX);
	m_translation44(inverseTranslation, -position[0], -position[1], -position[2]);
	m_mul44(view, inverseRotation, inverseTranslation); // = inverse(translation(position) * rotationYX);

	// output camera parameters
	// console.log("position:", position[0], position[1], position[2]);
	// console.log("rotation:", rotation[0], rotation[1]);
	// console.log("view");
	// for (int i=0; i<16; ++i)
	// 	console.log(view[i]);
}
