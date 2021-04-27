#pragma once

namespace display_texture
{
	GLuint program;
	GLuint vao, vbo;

	void init()
	{
		int positionsSize = 6 *  sizeof(m_vec3);
		m_vec3 positions[] = {{-1,-1,0}, {1,-1,0}, {-1,1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};

		// upload geometry to opengl
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
		unsigned char *buffer = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		assert(buffer);
		memcpy(buffer, positions, positionsSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		const char *attribs[] =
		{
			"a_position"
		};

		const char *vp = R"(
			#version 410 core
			in vec3 a_position;
			out vec3 f_position;

			void main()
			{
    			gl_Position = vec4(a_position, 1.0);
			    f_position = vec3(a_position.x*0.5+0.5, a_position.y*0.5+0.5, 0.0);
			}

		)";

		const char *fp = R"(

			#version 410 core

			in vec3 f_position;
			out vec4 o_color;
			// uniform samplerCubeArray u_map;
			// uniform sampler2D u_map;

			void main()
			{
			    // o_color = texture(u_map, vec4(normalize(vec3(f_position.xy, 1.0)), 0.0));
			    o_color = vec4(f_position, 0.0);
			}

		)";

		program = s_loadProgram(vp, fp, attribs, 1);
		if (!program)
		{
			throw "Error loading mesh shader";
		}
	}

	static void draw()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//glDisable(GL_CULL_FACE);

		// mesh
		glUseProgram(program);
		// textures
		// glUniform1i(glGetUniformLocation(program, "u_map"), 0);
		// vertices
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

