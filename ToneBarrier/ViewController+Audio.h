//
//  ViewController+Audio.h
//  JABPlanetaryHourToneBarrier
//
//  Created by Xcode Developer on 11/2/22.
//  Copyright © 2022 The Life of a Demoniac. All rights reserved.
//

#import "ViewController.h"
#import "ViewController+Signal.h"

NS_ASSUME_NONNULL_BEGIN

//static typeof(AVAudioFormat *) audio_format_ref = NULL;
//static typeof(audio_format_ref) (^audio_format)(void) = ^{
//    static dispatch_once_t onceToken;
//    dispatch_once(&onceToken, ^{
//        audio_format_ref = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatFloat64 sampleRate:192000.f channels:2 interleaved:FALSE];
//    });
//
//    return audio_format_ref;
//};

static typeof(AVAudioPlayerNode *) player_node_ref = NULL;
static AVAudioPlayerNode * (^player_node)(void) = ^{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        player_node_ref = [[AVAudioPlayerNode alloc] init];
        [player_node_ref setRenderingAlgorithm:AVAudio3DMixingRenderingAlgorithmAuto];
        [player_node_ref setSourceMode:AVAudio3DMixingSourceModeAmbienceBed];
        [player_node_ref setPosition:AVAudioMake3DPoint(0.0, 0.0, 0.0)];
    });
    
    return player_node_ref;
};

static typeof(AVAudioEngine *) audio_engine_ref = NULL;
static typeof(audio_engine_ref) (^audio_engine)(typeof(player_node_ref)) = ^ (typeof(player_node_ref) player_node) {
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate, ^{
        audio_engine_ref = [[AVAudioEngine alloc] init];
    });
    
    [audio_engine_ref attachNode:player_node_ref];
    [audio_engine_ref connect:player_node_ref to:audio_engine_ref.mainMixerNode format:[audio_engine_ref.mainMixerNode outputFormatForBus:(AVAudioNodeBus)0]];
    
    return audio_engine_ref;
};

typedef typeof(void(^(^)(NSNotification *, void(^)(NSNotification *)))(void)) notification_observer;
typedef typeof(void(^)(void)) notification;
void(^(^(^notification_observation)(NSNotificationCenter *, NSOperationQueue *))(NSNotification *, void(^)(NSNotification *)))(void)  = ^ (NSNotificationCenter * notification_center, NSOperationQueue * operation_queue) {
    return ^ (NSNotification * observed_notification, typeof(void(^)(NSNotification *))notification_handler) {
        return ^{
            [notification_center addObserverForName:observed_notification.name object:observed_notification.object queue:operation_queue usingBlock:notification_handler];
        };
    };
};
static void (^observe_notifications)(NSArray<notification> *) = ^ (NSArray<notification> * notifications) {
    for (notification observed_notification in notifications) observed_notification();
};

static typeof(AVAudioSession *) audio_session_ref = NULL;
static typeof(audio_session_ref) (^audio_session)(void) = ^ AVAudioSession * {
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate, ^{
        audio_session_ref = [AVAudioSession sharedInstance];
    });
    @try {
        __autoreleasing NSError *error = nil;
        [audio_session_ref setCategory:AVAudioSessionCategoryPlayAndRecord mode:AVAudioSessionModeDefault options:AVAudioSessionCategoryOptionDefaultToSpeaker error:&error];
        [audio_session_ref setSupportsMultichannelContent:TRUE  error:&error];
        [audio_session_ref setPreferredInputNumberOfChannels:2  error:&error];
        [audio_session_ref setPreferredOutputNumberOfChannels:2 error:&error];
        [audio_session_ref setPrefersNoInterruptionsFromSystemAlerts:TRUE error:&error]; // TO-DO: Make this a user preference
        
        !(error) ?: ^ (NSError ** error_t) {
            printf("Error configuring audio session:\n\t%s\n", [[*error_t debugDescription] UTF8String]);
            NSException* exception = [NSException
                                      exceptionWithName:(*error_t).domain
                                      reason:(*error_t).localizedDescription
                                      userInfo:@{@"Error Code" : @((*error_t).code)}];
            @throw exception;
        }(&error);
    } @catch (NSException *exception) {
        printf("Exception configuring audio session:\n\t%s\n\t%s\n\t%lu",
               [exception.name UTF8String],
               [exception.reason UTF8String],
               ((NSNumber *)[exception.userInfo valueForKey:@"Error Code"]).unsignedIntegerValue);
    } @finally {
        
    }
    
    return audio_session_ref;
};

@interface ViewController (Audio)
@end

NS_ASSUME_NONNULL_END
