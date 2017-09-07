//
//  VCCRenderingEngine1.cpp
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//

//VCCRenderingEngine1类和工厂方法

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include "VCCRenderingEngine.hpp"

#include "Quaternion.hpp"
#include <vector>

static const float AnimationDuration = 0.25f;
using namespace std;
struct Vertex{
    vec3 Position;
    vec4 Color;
};

// Animation 结构将开启 3D 转换功能并包含了初始方位、当前差值方位以及结束方位3个方向上的四元数。
// 同时，该结构还定义了两个时间间隔值 Elapsed 和 Duration，单位为秒，他们用于计算 0~1 之间的差值结构。

struct Animation{
    Quaternion Start;
    Quaternion End;
    Quaternion Current;
    float Elapsed;
    float Duration;
};




//浮点常量以定义对应的角速度；
static const float RevolutionsPerSecond = 1;

class VCCRenderingEngine1 : public VCCRenderingEngine{
public:
    VCCRenderingEngine1();
    void Initialize(int width, int height);
    void Render() const;
    void UpdateAnimation(float timeStep);
    void OnRotate(VCCDeviceOrientation newOrientation);
private:

    
    //三角形数据位于两个 STL 容器 m_cone 和 m_disk 中。由于数据尺寸事先已知，向量容器类可视为一类较为理想的数据结构并可确保数据的连续存储。这里，针对 OpenGL，数据的连续存储是十分必要的。
    
    vector<Vertex> m_cone;
    vector<Vertex> m_disk;
    Animation m_animation;
    //float m_desiredAngle;
    //float m_currentAngle;
    GLuint m_framebuffer;
    //GLuint m_renderbuffer;
    
    //与 HelloArrow 程序不同，此处定义了两个渲染缓冲区。HelloArrow 程序执行 2d 渲染操作，因而仅使用了一个颜色缓冲区；而 helloCone 程序则需要需要新增一个缓冲区用于存储场景的深度值。简而言之，深度缓冲区可视为一类在各像素处存储 Z 值得特定的图像平面。
    //
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
};

//其中， UpdateAnimation() 和 OnRotate()通过桩函数（存根函数）实现，且需要进一步完善以支持旋转操作


VCCRenderingEngine* CreateRenderer1()
{
    return new VCCRenderingEngine1();
}
VCCRenderingEngine1::VCCRenderingEngine1()
{
    //生成渲染缓冲区操作符并将其绑定至管线上。
    glGenRenderbuffersOES(1, &m_colorRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderbuffer);
}

//Initialize()方法将构建视口变换居正以及投影矩阵。其中，投影矩阵定义了一个当前可见场景的 3D 空间
//首先，代码定义了椎体的半径值、高度以及几何细节层次。其中，几何细节层次可表示为构成当前椎体的若干垂直“片段”。
//待生成全部顶点后，代码将对 OpenGL 帧缓冲区对象以及转换状态进行初始化操作。
//鉴于当前程序包含了大量的 3D 操作，因而需要开启深度测试。

void VCCRenderingEngine1::Initialize(int width, int height)
{
    // 将对象分解为三角形的过程通常称作三角形剖分，更常见的称谓为细分操作，进而反映了更为广泛的表面多边形填充问题。
    
    // 针对圆和椎体，定义相关常量。
    const float coneRadius = 0.5f;
    const float coneHeight = 1.866f;
    const int coneSlices = 40;
    
    // Generate vertices for the disk
    {
        // 分配底盘点
        m_disk.resize(coneSlices + 2);
        
        // 初始化为 GL_TRIANGLE_FAN 拓扑关系
        // 初始化底盘中心点
        vector<Vertex>::iterator vertex_it = m_disk.begin();
        vertex_it->Color = vec4(0.75, 0.75, 0.75, 1);
        vertex_it->Position.x = 0;
        vertex_it->Position.y = 1 - coneHeight;
        vertex_it->Position.z = 0;
        vertex_it++;
        
        // 初始化底盘圆周点
        const float dtheta = TwoPi / coneSlices;
        for (float theta = 0; vertex_it != m_disk.end(); theta += dtheta) {
            vertex_it->Color = vec4(0.75, 0.75, 0.75, 1);
            vertex_it->Position.x = coneRadius * cos(theta);
            vertex_it->Position.y = 1 - coneHeight;
            vertex_it->Position.z = coneRadius * sin(theta);
            vertex_it++;
        }
        
    }
    
    // Generate bertices for the body of the cone
    {
        // 分配椎体点
        m_cone.resize((coneSlices + 1) * 2);
        
        // 初始化为 GL_TRISNGLES_STRIP 三角带拓扑关系.
        vector<Vertex>::iterator vertex_it = m_cone.begin();
        const float dtheta = TwoPi / coneSlices;
        for (float theta = 0; vertex_it != m_cone.end(); theta += dtheta) {
            
            // Grayscale gradient
            // 为了简化光照模拟计算，这里采用了灰度梯度值，其中，颜色值使用了固定值，且不会随对象位置的变化而变化，
            // 该技术有时也称作烘焙光照。
            float brightness = abs(sin(theta));
            vec4 color(brightness, brightness, brightness, 1);
            
            // 顶点
            vertex_it->Position = vec3(0, 1, 0);
            vertex_it->Color = color;
            vertex_it++;
            
            // 圆周点
            vertex_it->Position.x = coneRadius * cos(theta);
            vertex_it->Position.y = 1 - coneHeight;
            vertex_it->Position.z = coneRadius * sin(theta);
            vertex_it->Color = color;
            vertex_it++;
        }

    }
    
    
    // 创建深度缓存
    // 生成深度缓冲区 ID，实施绑定操作并分配储存空间。
    glGenRenderbuffersOES(1, &m_depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
    
    
    
    
    //Create the framebuffer object and attach the depth and color buffer
    // 生成帧缓冲区对象 ID，实施绑定操作，并通过 glFramebufferRenderbufferOES 将其与颜色值和深度值进行关联。
    
    
    
    glGenFramebuffersOES(1, &m_framebuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_framebuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_colorRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthRenderbuffer);
    
    // Bind the color buffer for rendering
    // 绑定颜色渲染缓冲区并使未来的渲染操作与其发生关联。
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderbuffer);
    // 构建视口的左、下、宽、以及高度属性值。
    glViewport(0, 0, width, height);
    // 针对 3D 场景，开启深度测试功能。
    glEnable(GL_DEPTH_TEST);
    // 构建投影和模型-视图转换。
    glMatrixMode(GL_PROJECTION);
    glFrustumf(-1.6f, 1.6, -2.4, 2.4, 5, 10);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0, 0, -7);
            //glMatrixMode(GL_PROJECTION);
            //initialize the projection matrix
            //const float maxX = 2;
            //const float maxY = 3;
            //glOrthof(-maxX, maxX, -maxY, maxY, -1, 1);
            //glMatrixMode(GL_MODELVIEW);
            // Initialize the rotation animation state
            //OnRotate(VCCDeviceOrientationPortrait);
            //m_currentAngle = m_desiredAngle;
    
}

//针对平滑旋转操作，Apple 通过 UIViewController 类提供了相应的底层实现方案，但这并非 OpenGL ES 所推荐的方法，其原因如下
//处于性能考虑，Apple 建议应避免 Core Animation 和 OpenGL 之间的交互操作。
//理想情况下，在应用程序的生命周期内，渲染缓冲区应保持相同的尺寸和高宽比，这将有利于提高程序的运行性能并降低代码的复杂度
//在图形应用程序中，开发人员需要全面地对动画和渲染进行操控
//
//
//



void VCCRenderingEngine1::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    // 针对深度缓冲区，增加了一个参数
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //为了防止累积效应，代码中还添加了调用 glPushMatrix() 和 glPopMatrix()
    glPushMatrix();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    mat4 rotation(m_animation.Current.ToMatrix());
    glMultMatrixf(rotation.Pointer());
    
    // draw cone
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_cone[0].Position.x);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &m_cone[0].Color.x);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cone.size());//该函数调用即可令 OpenGL 从定义于 gl*Pointer 中的指针获取数据，同时三角形数据将渲染至目标表面上。
    
    // draw disk
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_disk[0].Position.x);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &m_disk[0].Color.x);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_disk.size());
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    //关闭两个顶点属性。在执行绘制命令时，需要开启相关的顶点属性，但当后续绘制命令采用完全不同的垫垫属性集时，保留原有的属性并非上次。
    glPopMatrix();
}

//程序考察箭头的旋转方向问题，即顺时针还是逆时针旋转。此处，仅检测期望值是否大于当前角度值并不充分：若用户将设备方位从 270 改变至 0，则该角度值应增至 360。
//根据箭头的旋转方向，该方法将返回 -1、0 或 +1。这里，假设 m_currentAngle 和 m_desiredAngle 为 0（含）到 360（不含）之间的角度值




// 为了实现平滑的旋转操作，UpdateAnimation() 方法将在旋转四元数的基础上调用 Slerp() 方法。
void VCCRenderingEngine1::UpdateAnimation(float timeStep)
{
    if (m_animation.Current == m_animation.End)
        return;
    m_animation.Elapsed += timeStep;
    if (m_animation.Elapsed >= AnimationDuration) {
        m_animation.Current = m_animation.End;
    } else {
        float mu = m_animation.Elapsed / AnimationDuration;
        m_animation.Current = m_animation.Start.Slerp(mu, m_animation.End);
    }
}

// OnRotate() 方法将启动一个新的动画序列
void VCCRenderingEngine1::OnRotate(VCCDeviceOrientation newOrientation)
{
    vec3 direction;
    
    switch (newOrientation) {
        case VCCDeviceOrientationUnknown:
        case VCCDeviceOrientationPortrait:
            direction = vec3(0, 1, 0);
            break;
            
        case VCCDeviceOrientationPortraitUpsideDown:
            direction = vec3(0, -1, 0);
            break;
            
        case VCCDeviceOrientationFaceDown:
            direction = vec3(0, 0, -1);
            break;
            
        case VCCDeviceOrientationFaceUp:
            direction = vec3(0, 0, 1);
            break;
            
        case VCCDeviceOrientationLandscapeLeft:
            direction = vec3(+1, 0, 0);
            break;
            
        case VCCDeviceOrientationLandscapeRight:
            direction = vec3(-1, 0, 0);
            break;
    }
    
    m_animation.Elapsed = 0;
    m_animation.Start = m_animation.Current = m_animation.End;
    m_animation.End = Quaternion::CreateFromVectors(vec3(0, 1, 0), direction);
    
}
