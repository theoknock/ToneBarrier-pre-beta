//
//  ViewController.h
//  JABPlanetaryHourToneBarrier
//
//  Created by Xcode Developer on 7/8/19.
//  Copyright © 2019 The Life of a Demoniac. All rights reserved.
//

#import <WatchConnectivity/WatchConnectivity.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreGraphics/CoreGraphics.h>
#import <HealthKit/HealthKit.h>

@import UIKit;
@import Foundation;
@import AVFoundation;
@import AVFAudio;
@import AVRouting;
@import AVKit;
@import MediaPlayer;

#import "AppDelegate.h"

#define low_bound   300.0f
#define high_bound 4000.0f

#define min_duration 0.25f
#define max_duration 1.75f

#define min_amplitude 0.5f
#define max_amplitude 1.0f

@interface ViewController : UIViewController <WCSessionDelegate, DeviceStatusInterfaceDelegate, UIGestureRecognizerDelegate, AVRoutePickerViewDelegate>

@property (strong, nonatomic) UIDevice *device;
@property (strong, nonatomic) WCSession *watchConnectivitySession;
@property (weak, nonatomic) IBOutlet AVRoutePickerView *routePickerView;

@end

