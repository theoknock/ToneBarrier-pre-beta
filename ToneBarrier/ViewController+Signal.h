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
        
        // To-Do:
        //      - Cross-fade between tones
        //      -
        //      - Modulate amplitude of each tone based on pitch (slow or fast)
        //      - Modulate amplitude of tone pair (rise and fall)
        
        signal_sample = ^ (AVAudioFrameCount frame_start, AVAudioFrameCount frame_length, AVAudioFrameCount frame_count, Float32 * channel) {
            __block Float32 frequency_theta = 0.f;
            const Float32 frequency = scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
            Float32 frequency_theta_increment = DBL_M_PI * frequency / buffer_format.sampleRate;
            
            __block Float32 amplitude_theta = 0.f;
            const Float32 amplitude = scale(generate_normalized_random(), 0.f, 2.f, 0.0, 1.0);
            Float32 amplitude_theta_increment = DBL_M_PI * .5f / buffer_format.sampleRate;
            
            for (AVAudioFrameCount frame = frame_start; frame < frame_count; frame++)
            {
                channel[frame] = sinf(frequency_theta += frequency_theta_increment) * sinf(amplitude_theta += amplitude_theta_increment); // increment amplitude from 0 to 1 based on number of frames (more frames means less increment)
                !(frequency_theta > DBL_M_PI) ?: (frequency_theta -= DBL_M_PI);
                !(amplitude_theta > DBL_M_PI) ?: (amplitude_theta -= DBL_M_PI);
            }
        };
        
        buffer_signal = ^ (AVAudioPlayerNode * player_node) {
            Float32 left_duration = scale(scale(generate_normalized_random(), 0.25, 1.75, 0.0, 1.0), 0.0, frame_count, 0.25, 1.75);
            Float32 right_duration = scale(scale(generate_normalized_random(), 0.25, 1.75, 0.0, 1.0), 0.0, frame_count, 0.25, 1.75);
           
            signal_sample(0, frame_count * 0.25f, frame_count, audio_buffer_ref.floatChannelData[0]);
            signal_sample(frame_count * 0.75f, frame_count, frame_count, audio_buffer_ref.floatChannelData[0]);
            signal_sample(0, frame_count * 0.4f, frame_count, audio_buffer_ref.floatChannelData[1]);
            signal_sample(frame_count * 0.6f, frame_count, frame_count, audio_buffer_ref.floatChannelData[1]);
            
            [player_node scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node isPlaying]) buffer_signal(player_node);
            }];
        };
    });
};

@interface ViewController (Signal)

@end

NS_ASSUME_NONNULL_END
