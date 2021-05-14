#pragma once

GLuint create_2D_vec4_texture(int width, int height, const float* data, GLint sample_method, bool gen_mipmap = false)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);  
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sample_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sample_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
	if (gen_mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);
	return texture;
}

GLuint create_2D_float_texture(int width, int height, const float* data, GLint sample_method)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);  
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sample_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sample_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
	// glGenerateMipmap(GL_TEXTURE_2D);
	return texture;
}

GLuint create_2D_float_texture_array(int width, int height, int n_layer, float* data, GLint sample_method)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	// we use nestest interpolation since this texture is used for data retrieval
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, sample_method);
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, sample_method);
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, GL_R32F, width, height, n_layer, 0, GL_RED, GL_FLOAT, data);
	return texture;
}

void upload_albedo_map(const char* filename)
{
	int width, height, nrChannels;
	float *data = stbi_loadf(filename, &width, &height, &nrChannels, 3);
	if (!data || nrChannels!=3)
		throw "load texture image failed";
	glActiveTexture(GL_TEXTURE20);
	create_2D_vec4_texture(width, height, data, GL_LINEAR, true);
	stbi_image_free(data);
}