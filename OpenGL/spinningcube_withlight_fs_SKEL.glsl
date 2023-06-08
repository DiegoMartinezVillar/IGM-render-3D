#version 130

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

out vec4 frag_col;

in vec3 frag_3Dpos;
in vec3 vs_normal;
// in vec2 vs_tex_coord;

#define NUM_LIGHTS 2

uniform Material material;
uniform Light light[NUM_LIGHTS];
uniform vec3 view_pos;

void main() {
  vec3 result = vec3(0.0);
  for (int i = 0; i < NUM_LIGHTS; i++) {
    // Ambient
    vec3 ambient = light[i].ambient * material.ambient;

    vec3 light_dir = normalize(light[i].position - frag_3Dpos);

    // Diffuse
    float diff = max(dot(vs_normal, light_dir), 0.0);
    vec3 diffuse = light[i].diffuse * diff * material.diffuse;

    // Specular
    vec3 view_dir = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir = reflect(-light_dir, vs_normal);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light[i].specular * spec * material.specular; 

    result += (ambient + diffuse + specular);
  }
  
  frag_col = vec4(result, 1.0);
}
