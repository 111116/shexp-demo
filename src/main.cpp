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

#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_impl_glfw_gl3.cpp"

#include "m_math.h"
#include "s_shader.h"

#include "consolelog.hpp"
#include "sh.hpp"
#include "LHcubemap.hpp"

typedef struct
{
	struct
	{
		GLuint program;
		GLint u_view;
		GLint u_projection;
		GLint u_coefficients;
		GLint u_LHcubemap;

		GLuint LHtexture;
		GLuint vao, vbo;
		int vertices;

		m_vec3 coefficients[9];
	} mesh;
} scene_t;

static int initScene(scene_t *scene)
{

	scene->mesh.coefficients[0] = m_vec3{0.735427, 0.613381, 0.58883};
	scene->mesh.coefficients[1] = m_vec3{0.102978, 0.138365, 0.166129};
	scene->mesh.coefficients[2] = m_vec3{0.508975, 0.420545, 0.433161};
	scene->mesh.coefficients[3] = m_vec3{-0.0336926, -0.254128, -0.414039};
	scene->mesh.coefficients[4] = m_vec3{0.00985182, -0.113192, -0.184917};
	scene->mesh.coefficients[5] = m_vec3{0.103119, 0.141603, 0.168878};
	scene->mesh.coefficients[6] = m_vec3{-0.0445019, -0.06456, -0.0479999};
	scene->mesh.coefficients[7] = m_vec3{0.036427, -0.21701, -0.410711};
	scene->mesh.coefficients[8] = m_vec3{0.114988, 0.113818, 0.116976};

	scene->mesh.LHtexture = buildLHcubemap();

	// mesh
	//yo_scene *yo = yo_load_obj("sphere.obj", true, false);
	yo_scene *yo = yo_load_obj("../res/ball.obj", true, false);
	if (!yo || !yo->nshapes)
	{
		fprintf(stderr, "Error loading obj file\n");
		return 0;
	}

	scene->mesh.vertices = 0;
	for (int i = 0; i < yo->nshapes; i++)
		scene->mesh.vertices += yo->shapes[i].nelems * 3;

	m_vec3 *positions = (m_vec3*)calloc(scene->mesh.vertices, sizeof(m_vec3));
	m_vec3 *normals   = (m_vec3*)calloc(scene->mesh.vertices, sizeof(m_vec3));
	size_t positionsSize = scene->mesh.vertices * sizeof(m_vec3);
	size_t normalsSize   = scene->mesh.vertices * sizeof(m_vec3);

	int n = 0;
	for (int i = 0; i < yo->nshapes; i++)
	{
		yo_shape *shape = yo->shapes + i;
		for (int j = 0; j < shape->nelems * 3; j++)
		{
			positions[n + j] = *(m_vec3*)&shape->pos[shape->elem[j] * 3];
			normals[n + j] = *(m_vec3*)&shape->norm[shape->elem[j] * 3];
		}
		n += shape->nelems * 3;
	}
	yo_free_scene(yo);

	// upload geometry to opengl
	glGenVertexArrays(1, &scene->mesh.vao);
	glBindVertexArray(scene->mesh.vao);

	glGenBuffers(1, &scene->mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, scene->mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, positionsSize + normalsSize, NULL, GL_STATIC_DRAW);
	unsigned char *buffer = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	assert(buffer);
	memcpy(buffer, positions, positionsSize);
	memcpy(buffer + positionsSize, normals, normalsSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)positionsSize);

	free(positions);
	free(normals);

	const char *attribs[] =
	{
		"a_position",
		"a_normal"
	};

	const char *vp = R"(

#version 410 core
in vec3 a_position;
in vec3 a_normal;
uniform mat4 u_view;
uniform mat4 u_projection;
out vec3 v_normal;

void main()
{
    gl_Position = u_projection * (u_view * vec4(a_position, 1.0));
    v_normal = a_normal;
}

)";

	const char *fp = R"(
#version 410 core
in vec3 v_normal;
uniform vec3 u_coefficients[9];
out vec4 o_color;
uniform samplerCubeArray u_LHcubemap;

// TODO: windowing

void main()
{
// for all sphere blockers
// calculate angle & rotation
// accumulate log(occu)
    vec3 n = normalize(v_normal);
    vec3 SHLightResult[9];

    // for (int i=0; i<9; ++i)
	   //  SHLightResult[i] = texture(u_LHcubemap, vec4(n,i)).rgb;

    // vec3 result = vec3(0.0);
    // for (int i = 0; i < 9; ++i)
    //     result += SHLightResult[i];

    vec3 result = 3.5449075 * texture(u_LHcubemap, vec4(n,0)).rgb;
    // L_H dot product with SH_one, yields the integral of L_H

    result = 0.8 / 3.1416 * result; // times brdf
    
    // gamma correction
    float gamma = 2.2;
    result = max(result, 0.0);
    result = pow(result, vec3(1.0/gamma));
    o_color = vec4(result, 1.0);
}
)";

	scene->mesh.program = s_loadProgram(vp, fp, attribs, 2);
	if (!scene->mesh.program)
	{
		fprintf(stderr, "Error loading mesh shader\n");
		return 0;
	}
	scene->mesh.u_view = glGetUniformLocation(scene->mesh.program, "u_view");
	scene->mesh.u_projection = glGetUniformLocation(scene->mesh.program, "u_projection");
	scene->mesh.u_coefficients = glGetUniformLocation(scene->mesh.program, "u_coefficients");
	scene->mesh.u_LHcubemap = glGetUniformLocation(scene->mesh.program, "u_LHcubemap");

	return 1;
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
	glUniform3fv(scene->mesh.u_coefficients, 9, &scene->mesh.coefficients[0].x);
	glUniform1i(scene->mesh.u_LHcubemap, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, scene->mesh.LHtexture);
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

static void fpsCameraViewMatrix(GLFWwindow *window, float *view, bool ignoreInput)
{
	// initial camera config
	static float position[] = { 0.0f, 0.5f, 3.0f };
	static float rotation[] = { 0.0f, 0.0f };

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
}

static void error_callback(int error, const char *description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char* argv[])
{
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
	GLFWwindow *window = glfwCreateWindow(1280, 800, "Spherical Harmonics Playground", NULL, NULL);
	if (!window) return 1;
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	ImGui_ImplGlfwGL3_Init(window, true);

	scene_t scene = {0};
	if (!initScene(&scene))
	{
		fprintf(stderr, "Could not initialize scene.\n");
		return 1;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			destroyScene(&scene);
			if (!initScene(&scene))
			{
				fprintf(stderr, "Could not reinitialize scene.\n");
				break;
			}
		}
		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_FirstUseEver);
		static bool show_another_window = true;
		ImGui::Begin("Coefficients", &show_another_window);
		for (int i = 0; i < 9; i++)
		{
			char name[] = "[?]";
			name[1] = i + '0';
			m_vec3 remapped = m_scale3(m_add3(scene.mesh.coefficients[i], m_v3(1.0f, 1.0f, 1.0f)), 0.5f);
			ImGui::ColorEdit3(name, &remapped.x);
			scene.mesh.coefficients[i] = m_sub3(m_scale3(remapped, 2.0f), m_v3(1.0f, 1.0f, 1.0f));
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glViewport(0, 0, w, h);
		float view[16], projection[16];
		fpsCameraViewMatrix(window, view, ImGui::IsAnyItemActive());
		m_perspective44(projection, 45.0f, (float)w / (float)h, 0.01f, 100.0f);
		drawScene(&scene, view, projection);

		ImGui::Render();
		glfwSwapBuffers(window);
	}

	destroyScene(&scene);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}