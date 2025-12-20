#version 330 core
out vec4 FragColor;

// 离得越近，深度值越小，图越黑，离得越远，深度值越大，图越白
// 深度：描述的是当前绘制的fragment距离相机的距离

// gl_FragCoord：三维向量，包含了当前片元的窗口坐标x、y（范围与窗口大小有关，0-Width, 0-Height）以及深度值z(0-1)
uniform float far;
uniform float near;

float LinearizeDepth(float depth) // 将非线性深度值转换为线性深度值
{
    float z = depth * 2.0 - 1.0; // back to NDC ，z 为 NDC 坐标
    return (2.0 * near * far) / (far + near - z * (far - near));	// 返回的是相机坐标下的z值，即距离相机的距离
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z) / log(far + 1.0); // divide by far to get depth in range [0,1] for visualization purposes
    FragColor = vec4(vec3(depth), 1.0);
}