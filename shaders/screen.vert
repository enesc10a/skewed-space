// shaders/screen.vert
#version 410 core
layout(location = 0) in vec2 vPos;   // comes from fsVAO (x,y in NDC)
void main() {
    gl_Position = vec4(vPos, 0.0, 1.0);
}
//
//  screen.vert
//  410HW_00
//
//  Created by Enes Faruk Çona on 10.06.2025.
//

