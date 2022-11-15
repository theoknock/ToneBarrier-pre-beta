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

static const float high_frequency = 1750.0;
static const float low_frequency  = 500.0;
//static const float min_duration   = 0.25;
//static const float max_duration   = 2.00;

double frequency[2];
NSInteger alternate_channel_flag;
double duration_bifurcate;

static typeof(GKMersenneTwisterRandomSource *) randomizer_ref = NULL;
static typeof(randomizer_ref) (^randomizer)(void) = ^{
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate, ^{
        randomizer_ref = [[GKMersenneTwisterRandomSource alloc] initWithSeed:time(NULL)];
    });
    return randomizer_ref;
};

static typeof(GKGaussianDistribution *) frequency_distributor_ref = NULL;
static typeof(frequency_distributor_ref) (^frequency_distributor)(void) = ^{
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate, ^{
        frequency_distributor_ref = [[GKGaussianDistribution alloc] initWithRandomSource:randomizer() mean:(high_frequency / .75) deviation:low_frequency];
    });
    return frequency_distributor_ref;
};

static typeof(GKGaussianDistribution *) duration_distributor_ref = NULL;
static typeof(duration_distributor_ref) (^duration_distributor)(void) = ^{
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate, ^{
        duration_distributor_ref = [[GKGaussianDistribution alloc] initWithRandomSource:randomizer_ref mean:(high_frequency / .75) deviation:low_frequency];
    });
    return duration_distributor_ref;
};

typedef NS_ENUM(unsigned int, Fade) {
    FadeOut,
    FadeIn
};

static unsigned int fade_bit = FadeIn;
double (^fade)(Fade, double, double) = ^ double(Fade fadeType, double x, double freq_amp)
{
    double fade_effect = freq_amp * ((1.0 - x) + fadeType);
    return fade_effect;
};

static typeof(AVAudioPCMBuffer *) audio_buffer_ref = NULL;
static typeof(void (^)(AVAudioPlayerNode *)) buffer_signal;
static void (^audio_buffer)(AVAudioFormat *) = ^ (AVAudioFormat * buffer_format) {
    AVAudioFrameCount frame_count = buffer_format.sampleRate * buffer_format.channelCount;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        audio_buffer_ref = [[AVAudioPCMBuffer alloc] initWithPCMFormat:buffer_format frameCapacity:frame_count];
        frequency_distributor();
        duration_distributor();
        
//        void(^sample_tone)(void) =
//        ^{
//            typedef typeof(Float32(^)(Float32))  sample_tone_function;
//            typedef typeof(Float32(^*)(Float32)) sample_tone_function_t;
//
//            sample_tone_function tone_a, tone_b;
//            __block void * tone_a_t  = (sample_tone_function_t)&tone_a;
//            __block void * tone_b_t = (sample_tone_function_t)&tone_b;
//
//            tone_a = ^ Float32 (Float32 frame) {
//                printf(" func_a returned ");
//                return (Float32)(((Float32)frame_count ^ frame) && (*aggregate_op_t)(~-p));
//            };
//
//            tone_b = ^ Float32 (Float32 index) {
//                printf(" func_b returned ");
//                return index;
//            };
//
//            // Instead of a "tone_c" block that swaps the current_sample_tone_function pointer,
//            // swap inside tone_a and tone_b instead (since they are already evaluating a conditional;
//            // Each will call itself until the frame equals the frame count or the split_duration frame
//
//            printf("Invoked current(11) and");
//            printf("%lu.\n", (*((sample_tone_function_t)(active_tone)))(11));
//            printf("Swapped current with swap and then ");
//            (sample_tone_function_t)({ dormant_tone = (uintptr_t)dormant_tone ^ (uintptr_t)active_tone ^ (uintptr_t)(active_tone = (uintptr_t)dormant_tone); dormant_tone; });
//            printf("invoked current again (with a value of 24):");
//            printf("%lu\n", (*((sample_tone_function_t)(active_tone)))(24));
//        };
        
        buffer_signal = ^ (AVAudioPlayerNode * player_node) {
            ^ (float frequency_right, float frequency_left) {
                audio_buffer_ref.frameLength = frame_count;
                float *left_channel  = audio_buffer_ref.floatChannelData[0];
                float *right_channel = (buffer_format.channelCount == 2) ? audio_buffer_ref.floatChannelData[1] : nil;
//
//                Float32 first_tone = NormalizedSineEaseInOut(normalized_index, frequency_left), second_tone = NormalizedSineEaseInOut(normalized_index, frequency_right);
//                Float32 left_channel_mix = 0.f, right_channel_mix = 0.f;
                
                int amplitude_frequency = arc4random_uniform(4) + 2;
                for (int index = 0; index < frame_count; index++) {
                    double normalized_index = LinearInterpolation(index, frame_count);
                    
                    // To-do for each tone pair:
                    //        - play one frequency (either the right and left) on both channels at a time
                    //          - spatially-orient the tone using a channel-mix/balance calculation
                    //        - transition to the other frequency using a mix (or channel balance) calculation
                    //          - transition between frequencies using index
                    //          - weight the index value to vary duration between tones
                    // To-do for each tone-pair dyad:
                    //        - vary the duration-weighted values between channels for a panning effect (i.e., replace Fade)
                    
                    // The mix/balance algorithms should be applied to:
                    //          - Tone-pair dyad:
                    //              - ease each tone-pair dyad in and out (volume ramping) by applying the same calculation to both tones (in other words, to both channels for the duration of the dyad)
                    //          - orient each tone pair spatially (balancing or mixing) by applying a separate calculation to each tone (in other words, to the right and left channel, each)
                    // The order of evaluation is:
                    //          - The calculations that spatially orients each tone should be applied to each channel first
                    Float32 amplitude_signal = NormalizedSineEaseInOut(normalized_index, amplitude_frequency);
                    if (left_channel)  left_channel[index]  = (NormalizedSineEaseInOut(normalized_index, frequency_left)  * amplitude_signal);
                    if (right_channel) right_channel[index] = (NormalizedSineEaseInOut(normalized_index, frequency_right) * amplitude_signal); // fade((leading_fade == FadeOut) ? FadeIn : leading_fade, normalized_index, (SineEaseInOutFrequency(normalized_index, frequencyRight) * NormalizedSineEaseInOutAmplitude((1.0 - normalized_index), 1)));
                }
            }([frequency_distributor_ref nextInt], [frequency_distributor_ref nextInt]);
            
            [player_node scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node isPlaying]) ^{ buffer_signal(player_node); fade_bit ^= 1; }();
                NSLog(@"player_node %@: AVAudioPlayerNodeCompletionDataPlayedBack\n", [player_node description]);
            }];
        };
    });
};

@interface ViewController (Signal)

@end

NS_ASSUME_NONNULL_END
