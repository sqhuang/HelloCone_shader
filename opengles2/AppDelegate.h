//
//  AppDelegate.h
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GLView.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    UIWindow* m_window;
    GLView* m_view;

}

//@property (strong, nonatomic) UIWindow *m_window;
////@property 声明了一个属性
//@property (strong, retain) GLView *m_view;

//这个属性删了也没事？属性机制不是很明白
//简而言之，关键字 @property 声明了一个属性， @synthesize 则定义了一个访问方法
@end

