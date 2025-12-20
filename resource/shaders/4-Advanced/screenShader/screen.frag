#version 330 core
out vec4 FragColor;

// 从顶点着色器传入的变量
in vec2 UV;       

uniform sampler2D screenTexture;

void main(){
	FragColor = texture(screenTexture, UV);
}
