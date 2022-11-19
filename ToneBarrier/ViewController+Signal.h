//
//  ViewController+Signal.h
//  JABPlanetaryHourToneBarrier
//
//  Created by Xcode Developer on 11/2/22.
//  Copyright © 2022 The Life of a Demoniac. All rights reserved.
//

#import "ViewController.h"
#import "ViewController+Audio.h"
#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

#include "easing.h"
#include <math.h>

NS_ASSUME_NONNULL_BEGIN

#define PI 3.1415926535897932384626
#define EULER 2.71828183

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

typedef typeof(Float32(^)(void)) random_generator;
typedef typeof(Float32(^(* restrict))(void)) random_n_t;
static Float32 (^(^(^(^generate_random)(Float32(^)(void)))(Float32(^)(Float32)))(Float32(^)(Float32)))(void) = ^ (Float32(^randomize)(void)) {
    return ^ (Float32(^distribute)(Float32)) {
        return ^ (Float32(^scale)(Float32)) {
            return ^ Float32 {
                return scale(distribute(randomize()));
            };
        };
    };
};

typedef NS_ENUM(NSUInteger, TonalHarmony) {
    TonalHarmonyConsonance,
    TonalHarmonyDissonance,
    TonalHarmonyRandom
};

typedef NS_ENUM(NSUInteger, TonalInterval) {
    TonalIntervalUnison,
    TonalIntervalOctave,
    TonalIntervalMajorSixth,
    TonalIntervalPerfectFifth,
    TonalIntervalPerfectFourth,
    TonalIntervalMajorThird,
    TonalIntervalMinorThird,
    TonalIntervalOffkey,
    TonalIntervalDefault
};

typedef NS_ENUM(NSUInteger, TonalEnvelope) {
    TonalEnvelopeAverageSustain,
    TonalEnvelopeLongSustain,
    TonalEnvelopeShortSustain
};

typedef NS_ENUM(unsigned int, MusicalNote) {
    MusicalNoteA,
    MusicalNoteBFlat,
    MusicalNoteB,
    MusicalNoteC,
    MusicalNoteCSharp,
    MusicalNoteD,
    MusicalNoteDSharp,
    MusicalNoteE,
    MusicalNoteF,
    MusicalNoteFSharp,
    MusicalNoteG,
    MusicalNoteAFlat
};

typedef NS_ENUM(unsigned int, MusicalNoteFrequency) {
    MusicalNoteFrequencyA      = 440,
    MusicalNoteFrequencyBFlat  = 466,
    MusicalNoteFrequencyB      = 494,
    MusicalNoteFrequencyC      = 523,
    MusicalNoteFrequencyCSharp = 554,
    MusicalNoteFrequencyD      = 587,
    MusicalNoteFrequencyDSharp = 622,
    MusicalNoteFrequencyE      = 659,
    MusicalNoteFrequencyF      = 698,
    MusicalNoteFrequencyFSharp = 740,
    MusicalNoteFrequencyG      = 784,
    MusicalNoteFrequencyAFlat  = 831
};

//typedef struct attribute((objc_boxable)) MusicalNotes MusicalNotes;
//struct attribute((objc_boxable)) MusicalNotes
//{
//    MusicalNoteFrequency note_frequency[12];
//}  musical_note = { .note_frequency = { MusicalNoteFrequencyA, MusicalNoteFrequencyBFlat, MusicalNoteFrequencyB, MusicalNoteFrequencyC, MusicalNoteFrequencyCSharp, MusicalNoteFrequencyD, MusicalNoteFrequencyDSharp, MusicalNoteFrequencyE, MusicalNoteFrequencyF, MusicalNoteFrequencyFSharp, MusicalNoteFrequencyG, MusicalNoteFrequencyAFlat } };

static unsigned int counter = 0;
static Float32 (^tonal_interval)(TonalInterval) = ^ Float32 (TonalInterval interval) {
    Float32 consonant_harmonic_interval_ratios [8] = {1.0, 2.0, 5.0/3.0, 4.0/3.0, 5.0/4.0, 6.0/5.0, (1.1 + drand48()), 5.0/4.0};
    return consonant_harmonic_interval_ratios[interval % TonalIntervalDefault];
};

const Float32 DBL_PI = 2.f * PI;

static typeof(AVAudioPCMBuffer *) audio_buffer_ref = NULL;

static Float32 (^generate_normalized_random)(void);
static random_generator random_musical_note_generator;

static void (^signal_sample)(Float32 * channel_l, Float32 * channel_r, AVAudioFrameCount buffer_length) = ^ (Float32 * channel_l, Float32 * channel_r, AVAudioFrameCount buffer_length) {
    static Float32 frequency_theta[2][2], frequencies[2][2], frequency_theta_increment[2][2];
    AVAudioFrameCount frame = 0;
    AVAudioFrameCount * frame_t = &frame;
    Float32 normalized_index = 0.f;
    Float32 * normalized_index_t = &normalized_index;
    Float32 o[2], v[2], u[2];
    //    ({
    frequencies[0][0] = random_musical_note_generator();
    frequency_theta_increment[0][0] = DBL_PI * frequencies[0][0] / audio_buffer_ref.format.sampleRate;
    frequencies[0][1] = random_musical_note_generator();
    frequency_theta_increment[0][1] = DBL_PI * frequencies[0][1] / audio_buffer_ref.format.sampleRate;
    simd_double2 frequency_theta_v = simd_make_double2(frequency_theta[0][0], frequency_theta[0][1]);
    simd_double2 frequency_theta_increment_v = simd_make_double2(frequency_theta_increment[0][0], frequency_theta_increment[0][1]);
    
    o[0] = 0.4f;
    v[0] = 0.9; // variance [width] (values lower than 0.15 increase amplitude)
    u[0] = 0.5f; // mean [shift] (variance / 2)
    
    frequencies[1][0] = random_musical_note_generator();
    frequency_theta_increment[1][0] = DBL_PI * frequencies[1][0] / audio_buffer_ref.format.sampleRate;
    frequencies[1][1] = random_musical_note_generator();
    frequency_theta_increment[1][1] = DBL_PI * frequencies[1][1] / audio_buffer_ref.format.sampleRate;
    o[1] = 0.4f;
    v[1] = 0.9; // variance [width] (values lower than 0.15 increase amplitude)
    u[1] = 0.5f; // mean [shift]
    //    });
    //    ({
    
    for (; *frame_t < buffer_length; *frame_t += 1)
    {
        ({ *normalized_index_t = (*frame_t / buffer_length); });
        
        Float32 tone_pair_1_envelope = (1.f/(o[0] * sqrtf(2.f * PI))) * (EULER * -(pow(*normalized_index_t - u[0], 2.f) / (pow(2.f * v[0], 2.f))));
        simd_double2 tone_v = simd_make_double2(_simd_sin_d2(frequency_theta_v += frequency_theta_increment_v));
        Float32 tone_pair_1 = (tone_v[0] + (0.1 - (*normalized_index_t * 0.5f)) * (tone_v[1] - tone_v[0]));
        !(frequency_theta_v[0] > DBL_PI) ?: (frequency_theta_v[0] -= DBL_PI);
        !(frequency_theta_v[1] > DBL_PI) ?: (frequency_theta_v[1] -= DBL_PI);
        
        Float32 tone_pair_2_envelope = (1.f/(o[1] * sqrtf(2.f * PI))) * (EULER * -(pow(*normalized_index_t - u[1], 2.f) / (pow(2.f * v[1], 2.f))));
        Float32 tone_a2 = sinf(frequency_theta[1][0] += frequency_theta_increment[1][0]);
        Float32 tone_b2 = sinf(frequency_theta[1][1] += frequency_theta_increment[1][1]);
        Float32 tone_pair_2 = (tone_a2 + (0.5f * (tone_b2 - tone_a2)));
        !(frequency_theta[1][0] > DBL_PI) ?: (frequency_theta[1][0] -= DBL_PI);
        !(frequency_theta[1][1] > DBL_PI) ?: (frequency_theta[1][1] -= DBL_PI);
        
        Float32 tone_pair_dyad_1 = (tone_pair_1 + (0.5 * (tone_pair_2 - tone_pair_1))); // what would a gaussian distribution curve look like if the first tone pair played at a balance of 1.0 and the second played at 0.0, but then swapped halfway
        
        channel_l[*frame_t] = tone_pair_1;
        //            channel_r[*frame_t] = tone_pair_dyad_1;
    }
    //    });
};

static typeof(void (^)(void)) buffer_signal = ^{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    signal_sample(audio_buffer_ref.floatChannelData[0], audio_buffer_ref.floatChannelData[1], audio_buffer_ref.frameLength);
    [player_node_ref scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
        if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node_ref isPlaying]) buffer_signal();
        NSLog(@"AVAudioPlayerNodeCompletionDataPlayedBack\n");
    }];
};

static typeof(audio_buffer_ref) (^audio_buffer)(AVAudioFormat *) = ^ (AVAudioFormat * buffer_format) {
    AVAudioFrameCount frame_count = buffer_format.sampleRate * buffer_format.channelCount;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        audio_buffer_ref = [[AVAudioPCMBuffer alloc] initWithPCMFormat:buffer_format frameCapacity:frame_count];
        audio_buffer_ref.frameLength = frame_count;
        generate_normalized_random = normalized_random_generator();
        random_musical_note_generator = generate_random(generate_normalized_random)(^ Float32 (Float32 n) { return n; })(^ Float32 (Float32 n) { return pow(2.f, round(scale(n, MusicalNoteA, MusicalNoteAFlat, 0.0, 1.0))/12.f) * 440.f; });
    });
    //        Float32 (^generate_normalized_random)(void) = normalized_random_generator();
    //        static random_generator random_musical_note_generator;
    //        random_musical_note_generator = generate_random(generate_normalized_random)(^ Float32 (Float32 n) { return n; })(^ Float32 (Float32 n) { return pow(2.f, round(scale(n, MusicalNoteA, MusicalNoteAFlat, 0.0, 1.0))/12.f) * 440.f; });
    //        signal_sample = ^ (AVAudioFrameCount frame_start, AVAudioFrameCount frame_split, AVAudioFrameCount frame_end, Float32 * channel_l, Float32 * channel_r) {
    //            Float32 frequency_theta[2][4], frequencies[2][4], frequency_theta_increment[2][4];
    //            AVAudioFrameCount frame = frame_start;
    //            AVAudioFrameCount * frame_t = &frame;
    //            Float32 normalized_index = 0.f;
    //            Float32 * normalized_index_t = &normalized_index;
    //            Float32 o[2], v[2], u[2];
    //            ({
    //                frequencies[0][0] = random_musical_note_generator();
    //                frequency_theta_increment[0][0] = DBL_PI * frequencies[0][0] / buffer_format.sampleRate;
    //                frequencies[0][1] = random_musical_note_generator();
    //                frequency_theta_increment[0][1] = DBL_PI * frequencies[0][1] / buffer_format.sampleRate;
    //                o[0] = 0.4f;
    //                v[0] = frame_split / frame_end; // variance (width)
    //                u[0] = v[0] / 2.f; // mean [shift] (variance / 2)
    //
    //                frequencies[0][2] = random_musical_note_generator();
    //                frequency_theta_increment[0][2] = DBL_PI * frequencies[0][2] / buffer_format.sampleRate;
    //                frequencies[0][3] = random_musical_note_generator();
    //                frequency_theta_increment[0][3] = DBL_PI * frequencies[0][3] / buffer_format.sampleRate;
    //                o[1] = 0.4f;
    //                v[1] = 1.0 - v[0]; // variance (width)
    //                u[1] = 1.0 - (v[1] - (v[1] / 2.f)); // mean [shift]
    //            });
    //            ({
    //
    //                for (; *frame_t < frame_end; *frame_t += 1)
    //                {
    //                    ({ *normalized_index_t = *frame_t / buffer_format.sampleRate; });
    //
    //                    Float32 tone_pair_1_envelope = 1.f; //(1.f/(o[0] * sqrtf(2.f * PI))) * (EULER * -(pow(*normalized_index_t - u[0], 2.f) / (pow(2.f * v[0], 2.f))));
    //                    Float32 tone_a1 = sinf(frequency_theta[0][0] += frequency_theta_increment[0][0]);
    //                    Float32 tone_b1 = sinf(frequency_theta[0][1] += frequency_theta_increment[0][1]);
    //                    Float32 tone_pair_1 = (tone_a1 + (0.5f * (tone_b1 - tone_a1))) * tone_pair_1_envelope;
    //                    !(frequency_theta[0][0] > DBL_PI) ?: (frequency_theta[0][0] -= DBL_PI);
    //                    !(frequency_theta[0][1] > DBL_PI) ?: (frequency_theta[0][1] -= DBL_PI);
    //
    //                    Float32 tone_pair_2_envelope = 1.f; //(1.f/(o[1] * sqrtf(2.f * PI))) * (EULER * -(pow(*normalized_index_t - u[1], 2.f) / (pow(2.f * v[1], 2.f))));
    //                    Float32 tone_a2 = sinf(frequency_theta[1][2] += frequency_theta_increment[1][2]);
    //                    Float32 tone_b2 = sinf(frequency_theta[1][3] += frequency_theta_increment[1][3]);
    //                    Float32 tone_pair_2 = (tone_a2 + (0.5f * (tone_b2 - tone_a2))) * tone_pair_2_envelope;
    //                    !(frequency_theta[0][2] > DBL_PI) ?: (frequency_theta[0][2] -= DBL_PI);
    //                    !(frequency_theta[0][3] > DBL_PI) ?: (frequency_theta[0][3] -= DBL_PI);
    //
    //                    channel_l[*frame_t] = tone_pair_1 + (0.5f * (tone_pair_2 - tone_pair_1));
    //                }
    //            });
    //
    //            buffer_signal = ^{
    //                NSLog(@"%s", __PRETTY_FUNCTION__);
    //                AVAudioFrameCount split_frame = scale(scale(generate_normalized_random(), 0.5, 1.75, 0.0, 1.0), 0.0, 1.0, 0.5, 1.75); // To-Do: Use a gaussian distribution to scale the normalized duration
    //                signal_sample(0, split_frame, frame_count, audio_buffer_ref.floatChannelData[0], audio_buffer_ref.floatChannelData[1]);
    //                [player_node_ref scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
    //                    if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node_ref isPlaying]) buffer_signal();
    //                    NSLog(@"AVAudioPlayerNodeCompletionDataPlayedBack\n");
    //                }];
    //            };
    //        };
    //    });
    return audio_buffer_ref;
};

@interface ViewController (Signal)

@end

NS_ASSUME_NONNULL_END
