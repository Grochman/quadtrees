#version 330 core
in vec3 vColor;
in vec2 vUv;
out vec4 FragColor;
void main() {
    if (dot(vUv, vUv) > 1.0) {
        discard;
    }
    FragColor = vec4(vColor, 1.0);
}
