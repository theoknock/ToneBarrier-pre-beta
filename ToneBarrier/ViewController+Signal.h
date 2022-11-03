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

typedef NS_ENUM(NSUInteger, Fade) {
    FadeOut,
    FadeIn
};

double (^fade)(Fade, double, double) = ^double(Fade fadeType, double x, double freq_amp)
{
    double fade_effect = freq_amp * ((fadeType == FadeIn) ? x : (1.0 - x));
    return fade_effect;
};

static typeof(AVAudioPCMBuffer *) audio_buffer_ref = NULL;
static typeof(AVAudioPCMBuffer *(^)(void)) buffer_signal;
static void (^audio_buffer)(AVAudioFormat *) = ^ (AVAudioFormat * buffer_format) {
    AVAudioFrameCount frame_count = buffer_format.sampleRate * buffer_format.channelCount;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        audio_buffer_ref = [[AVAudioPCMBuffer alloc] initWithPCMFormat:buffer_format frameCapacity:frame_count];
        frequency_distributor();
        duration_distributor();
        buffer_signal = ^ AVAudioPCMBuffer * {
            ^ (float frequency_right, float frequency_left) {
                audio_buffer_ref.frameLength = frame_count;
                float *left_channel  = audio_buffer_ref.floatChannelData[0];
                float *right_channel = (buffer_format.channelCount == 2) ? audio_buffer_ref.floatChannelData[1] : nil;
                
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
                    
                    
                    if (left_channel)  left_channel[index]  = fade(FadeOut, normalized_index, (NormalizedSineEaseInOut(normalized_index, frequency_left)  * NormalizedSineEaseInOut(normalized_index, amplitude_frequency)));
                    if (right_channel) right_channel[index] = fade(FadeIn,  normalized_index, (NormalizedSineEaseInOut(normalized_index, frequency_right) * NormalizedSineEaseInOut(normalized_index, amplitude_frequency))); // fade((leading_fade == FadeOut) ? FadeIn : leading_fade, normalized_index, (SineEaseInOutFrequency(normalized_index, frequencyRight) * NormalizedSineEaseInOutAmplitude((1.0 - normalized_index), 1)));
                }
            }([frequency_distributor_ref nextInt], [frequency_distributor_ref nextInt]);
            
            return audio_buffer_ref;
        };
    });
};

@interface ViewController (Signal)

@end

NS_ASSUME_NONNULL_END
