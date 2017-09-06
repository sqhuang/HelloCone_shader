//
//  GLView.h
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//
#import <UIKit/UIKit.h>
#import "VCCRenderingEngine.hpp"
#import <OpenGLES/EAGL.h>//eagl 头文件并不是 OpenGL 标准中的一部分，但需要通过它创建 OpenGL ES 上下文环境
#import <QuartzCore/QuartzCore.h>

//

@interface GLView : UIView
{
    EAGLContext* m_context; //该对象负责对当前的 OpenGL 上下文进行管理；EAGL 表示一类小型的、特定于 Apple 的 API，并通过 OpenGL 与 iPhone 操作系统进行链接。
    
    VCCRenderingEngine* m_renderingEngine;
    
    float m_timestamp;
}

//当通过 OpenGL 函数调用改变当前的 API 的状态时，用户需要再某一特定的上下文中执行这一过程。针对于运行与系统中的某一线程，任何时候都仅存在一个当前上下文。而 iOS 应用程序一般很少出现多上下文的情况。考虑到移动设备上的有限资源，通常不推荐采用多上下文环境。
- (void) drawView: (CADisplayLink*) displayLink;
- (void) didRotate: (NSNotification*) notification;
@end
