#version 410 core
in vec3 v_normal;
uniform vec3 u_coefficients[9];
out vec4 o_color;
uniform samplerCubeArray u_LHcubemap;

#define PI (3.14159265359)

#define N 9

// TODO: windowing

float[N] shexp(float[N] f)
{
	// naive linear
	float[9] g;
	float e = exp(f[0]/sqrt(4.0/PI));
	g[0] = sqrt(4.0*PI)*e;
	for (int i = 1; i < N; i++)
	{
		g[i] = f[i]*e;
	}
	return g;
}

void main()
{
// for all sphere blockers
// calculate angle & rotation
// accumulate log(occu)
    vec3 n = normalize(v_normal);
    vec3 SHLightResult[9];

    // for (int i=0; i<9; ++i)
	   //  SHLightResult[i] = texture(u_LHcubemap, vec4(n,i)).rgb;
  //   logv;
  //   for (int i=0; i<nspheres; ++i) {
  //   	vec3 distv = world_position - u_position[i];
  //   	float dist = length(distv);
		// float angle = asin(u_radius[i] / dist);
		// t = texture()
	 //    add(logv, rotate(t, normalize(distv));
  //   }

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
