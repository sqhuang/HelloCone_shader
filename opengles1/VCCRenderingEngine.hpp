//
//  VCCRenderingEngine.hpp
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//
/*
我们采用面向组件的方式定义了一个 C++ 接口，编码风格如下
 全部接口方法定义为纯虚方法
 鉴于接口方法的 public 特性，接口设计为结构体 （在 C++ 语言中，结构体成员默认状态下为 public；而类成员则默认为 private）
 接口名字以大写VCC开始
 接口仅由方法构成且不包含任何字段
 对应的类实现则通过工厂方法完成。此处相应的工厂方法定义为 CreateRender1
 全部接口应包含一个虚析构方法以实现正确的释放操作
 */

#ifndef VCCRenderingEngine_hpp
#define VCCRenderingEngine_hpp
#include <stdio.h>

enum VCCDeviceOrientation{
    VCCDeviceOrientationUnknown,
    VCCDeviceOrientationPortrait,            // Device oriented vertically, home button on the bottom
    VCCDeviceOrientationPortraitUpsideDown,  // Device oriented vertically, home button on the top
    VCCDeviceOrientationLandscapeLeft,       // Device oriented horizontally, home button on the right
    VCCDeviceOrientationLandscapeRight,      // Device oriented horizontally, home button on the left
    VCCDeviceOrientationFaceUp,              // Device oriented flat, face up
    VCCDeviceOrientationFaceDown             // Device oriented flat, face down
};
// Creates an instance of the renderer and sets up various OpenGL state



struct tagVCCRenderingEngine {
    virtual void Initialize(int width, int height) = 0;
    virtual void Render() const = 0;
    virtual void UpdateAnimation(float timeStep) = 0;
    virtual void OnRotate(VCCDeviceOrientation newOrientation) = 0;
    virtual ~tagVCCRenderingEngine(){}
};

typedef struct tagVCCRenderingEngine VCCRenderingEngine;

VCCRenderingEngine* CreateRenderer1();

#endif /* VCCRenderingEngine_hpp */
