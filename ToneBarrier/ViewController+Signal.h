//
//  ViewController+Signal.h
//  JABPlanetaryHourToneBarrier
//
//  Created by Xcode Developer on 11/2/22.
//  Copyright © 2022 The Life of a Demoniac. All rights reserved.
//

#import "ViewController.h"
#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

#include "easing.h"

NS_ASSUME_NONNULL_BEGIN

static Float32 (^scale)(Float32, Float32, Float32, Float32, Float32) = ^ Float32 (Float32 val_old, Float32 min_new, Float32 max_new, Float32 min_old, Float32 max_old) {
    Float32 val_new = min_new + ((((val_old - min_old) * (max_new - min_new))) / (max_old - min_old));
    
    return val_new;
};

static Float32 (^normalize_value)(Float32, Float32, Float32) = ^Float32(Float32 min, Float32 max, Float32 value) {
    Float32 result = (value - min) / (max - min);
    return result;
};

static Float32 (^(^normalized_random_generator)(void))(void) = ^{
    srand48((unsigned int)time(0));
    static Float32 random;
    return ^ (Float32 * random_t) {
        return ^ Float32 {
            return (*random_t = (drand48()));
        };
    }(&random);
};

const Float32 DBL_M_PI = 2.f * M_PI;

static typeof(AVAudioPCMBuffer *) audio_buffer_ref = NULL;
static typeof(void (^)(AVAudioPlayerNode *)) buffer_signal;
static typeof(void (^)(AVAudioFrameCount frame_start, AVAudioFrameCount frame_length, AVAudioFrameCount frame_count, Float32 * channel)) signal_sample;
static void (^audio_buffer)(AVAudioFormat *) = ^ (AVAudioFormat * buffer_format) {
    AVAudioFrameCount frame_count = buffer_format.sampleRate * buffer_format.channelCount;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        audio_buffer_ref = [[AVAudioPCMBuffer alloc] initWithPCMFormat:buffer_format frameCapacity:frame_count];
        audio_buffer_ref.frameLength = frame_count;

        Float32 (^generate_normalized_random)(void) = normalized_random_generator();
        
        signal_sample = ^ (AVAudioFrameCount frame_start, AVAudioFrameCount frame_split, AVAudioFrameCount frame_end, Float32 * channel) {
            __block Float32 frequency_theta = 0.f;
            Float32 frequency = scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
            Float32 frequency_theta_increment = DBL_M_PI * frequency / buffer_format.sampleRate;

            for (AVAudioFrameCount frame = frame_start; frame < frame_split; frame++)
            {
                channel[frame] = sinf(frequency_theta += frequency_theta_increment);// * (.5f * (1.f * cosf((2.f * M_PI * normalize_value(frame_start, frame_end, frame)) / 2.f))); // 4.f covers the entire two-second tone pair; 2.f assumes the split frame evenly divides the duration between the two tones (one second each)
                !(frequency_theta > DBL_M_PI) ?: (frequency_theta -= DBL_M_PI);
            }
            // Replace random frequency with harmonic consonnance/disonnance scheme here...
            frequency = scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
            frequency_theta_increment = DBL_M_PI * frequency / buffer_format.sampleRate;
            for (AVAudioFrameCount frame = ++frame_split; frame < frame_end; frame++)
            {
                channel[frame] = sinf(frequency_theta += frequency_theta_increment);// * (.5f * (1.f * cosf((2.f * M_PI * normalize_value(frame_start, frame_end, frame)) / 2.f))); // 4.f covers the entire two-second tone pair; 2.f assumes the split frame evenly divides the duration between the two tones (one second each)
                !(frequency_theta > DBL_M_PI) ?: (frequency_theta -= DBL_M_PI);
            }
        };
        
        buffer_signal = ^ (AVAudioPlayerNode * player_node) {
            AVAudioFrameCount left_duration = scale(scale(generate_normalized_random(), 0.5, 1.75, 0.0, 1.0), 0.0, frame_count, 0.5, 1.75);
            AVAudioFrameCount right_duration = scale(scale(generate_normalized_random(), 0.5, 1.75, 0.0, 1.0), 0.0, frame_count, 0.5, 1.75);
            printf("left_duration == %d\nright_duration == %d\n\n", left_duration, right_duration);
           
            signal_sample(0, left_duration, frame_count, audio_buffer_ref.floatChannelData[0]);
            signal_sample(0, right_duration, frame_count, audio_buffer_ref.floatChannelData[1]);
            
            [player_node scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node isPlaying]) buffer_signal(player_node);
            }];
        };
    });
};

@interface ViewController (Signal)

@end

NS_ASSUME_NONNULL_END
