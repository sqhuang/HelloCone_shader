//
//  GLView.m
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//

#import "GLView.h"
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES2/gl.h>  //for GL_RENDERBUFFER only
#import "mach/mach_time.h"
//#include <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
//#import "VCCRenderingEngine.hpp"

@implementation GLView

//+ 前缀表明，这将是一个覆写类方法而非实例化方法。另外，覆写类型是 Objective-C 语言独有的特性，该特性一般不会出现于其他语言中。
+ (Class) layerClass{
    return [CAEAGLLayer class];
}


//initWithFrame 方法调用工厂方法实例化 C++ 渲染器并构建两个事件句柄，
//事件句柄一在每次屏幕更新是用于显示链接
//事件句柄二则在方向发生改变时作出响应。


- (id) initWithFrame:(CGRect)frame{
    if (self = [super initWithFrame:frame]) {
        //Initialize code...
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*) super.layer;//从基类（UIView）中获取 layer 属性，并将其从 CALayer 向下转型至 CAEAGLLayer。鉴于 layerClass 中的相关覆写方法，该转换过程是安全的。
        eaglLayer.opaque =YES;//设置当前层的 opaque 属性，即无需使用 Quartz 处理透明度问题。这也是 Apple 针对 OpenGL 推荐使用的一类优化方案。另外，OpenGL 可轻松地处理 appha 混合这一类问题。
        EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES1;
        m_context = [[EAGLContext alloc] initWithAPI:api];//创建 EAGLContext 对象并告之当前所使用的 OpenGL 版本

        if (![EAGLContext setCurrentContext:m_context] || !m_context) {
            NSLog(@"OpenGL ES Fail!");
            return nil;
        }
    //封装在引擎内的包括下面的步骤
        m_renderingEngine = CreateRenderer1();
    
    //OpenGL ES 初始化过程
    //首先，代码定义了 renderbuffer 和 framebuffer 两个 OpenGL 标识符。renderbuffer 表示一类 2D 表面并使用某种数据类型加以填充；而 framebuffer 则由多个 renderbuffer 组成。全部 OpenGL ES 应用程序均通过 FBO 将相关数据绘制到屏幕上。待 framebuffer 和 renderbuffer 定义完毕后，随即将这一类对象与当前管线进行绑定，后续的 OpenGL 操作课实现修改和释放操作。当对 renderbuffer 加以绑定后，通过 EAGLContext 对象发送 renderbufferStorage 消息，即可实现存储空间的分配操作。接下来，glFramebufferRenderbufferOES 命令将 renderbuffer 对象与 framebuffer 对象进行绑定。代码随后调用 glViewport 命令，其功能相当于构建一个对应的坐标系统。
//    GLuint framebuffer, renderbuffer;
//    glGenFramebuffersOES(1, &framebuffer);
//    glGenRenderbuffersOES(1, &renderbuffer);
//    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
//    glBindRenderbufferOES(GL_RENDERBUFFER_OES, renderbuffer);
    [m_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable: eaglLayer];
        //引擎里封装下面的东西
        m_renderingEngine->Initialize(CGRectGetWidth(frame), CGRectGetHeight(frame));
//        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, renderbuffer);
//        glViewport(0, 0, CGRectGetWidth(frame), CGRectGetHeight(frame));
        [self drawView: nil];
        m_timestamp = CACurrentMediaTime();
        //Apple 建议使用 CADisplayLink 处罚 OpenGL 渲染。一种替代方案是采用 NSTimer 类。
        CADisplayLink* displayLink;
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
    }
    return self;
}

//这将用到 OpenGL 中的“清除”机制，并采用某种固定颜色值填充缓冲区。首先颜色值被定义为灰色值（使用红色、绿色、蓝色和 alpha 值），并于随后调用清除操作 glClear。最终，EAGLContext 对象被告知将渲染缓冲区中的内容发至屏幕。由此可知，多数 OpenGL 应用程序并不讲渲染内容直接绘制在屏幕上，而是在缓冲区中执行先期渲染并于随后将其发至屏幕。这里不建议使用 Xcode 提供的 drawRect 方法，在大多数传统的基于 UIKit 的应用程序中，该方法一般用于绘图的更新操作。但在 3D 程序中，则需要相应地提升渲染标准。
//drawView 方法用于回应显示链接事件，且计算距最后一次调用所经历的时间，并将该值传递至渲染器的 UpdateAnimation 方法中，这将有助于渲染器实现动画的更新以及物理控制。
//drawView 方法发布渲染命令，并将渲染缓冲区中的相关内容发送至屏幕中
- (void) drawView:(CADisplayLink*) displayLink{
    if (displayLink != nil) {
        float elapsedSeconds = displayLink.timestamp - m_timestamp;
        m_timestamp = displayLink.timestamp;
        m_renderingEngine->UpdateAnimation(elapsedSeconds);
    }
    m_renderingEngine->Render();
//    下面的放到render里
//    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);
    [m_context presentRenderbuffer:GL_RENDERBUFFER_OES];
}


//didRotate 事件句柄则将特定于 iPhone 平台的 UIDeviceOrientation 发送至 DeviceOrientation 类型的结构中，并于随后将其传递至渲染引擎中
- (void) didRotate:(NSNotification *)notification{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    m_renderingEngine->OnRotate((VCCDeviceOrientation) orientation);
    [self drawView:nil];
}

@end
