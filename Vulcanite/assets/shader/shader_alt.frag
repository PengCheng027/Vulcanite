#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

void main(){
    // 4×4 像素棋盘 + 反色:不依赖任何新输入,一眼就能看出这条管线生效了
    float checker = mod(floor(gl_FragCoord.x / 4.0) + floor(gl_FragCoord.y / 4.0), 2.0);
    if (checker < 1.0) discard;
    outColor = vec4(1.0 - fragColor, 1.0);
}