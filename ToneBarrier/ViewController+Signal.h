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
        static random_generator random_musical_note_generator;
        random_musical_note_generator = generate_random(generate_normalized_random)(^ Float32 (Float32 n) { return n; })(^ Float32 (Float32 n) { return pow(2.f, round(scale(n, MusicalNoteA, MusicalNoteAFlat, 0.0, 1.0))/12.f) * 440.f; });
        signal_sample = ^ (AVAudioFrameCount frame_start, AVAudioFrameCount frame_split, AVAudioFrameCount frame_end, Float32 * channel) {
            Float32 frequency_theta[4], frequencies[4], frequency_theta_increment[4], normalized_index;
            volatile AVAudioFrameCount frame = frame_start;
            ({
                frequencies[0] = random_musical_note_generator();//scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
                frequency_theta_increment[0] = DBL_M_PI * frequencies[0] / buffer_format.sampleRate;
                frequencies[1] = random_musical_note_generator();//scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
                frequency_theta_increment[1] = DBL_M_PI * frequencies[1] / buffer_format.sampleRate;
            });
            ({
                for (; frame < frame_split; frame++)
                {
                    normalized_index = frame / frame_count;
                    Float32 tone_a = sinf(frequency_theta[0] += frequency_theta_increment[0]);
                    Float32 tone_b = sinf(frequency_theta[1] += frequency_theta_increment[1]);
                    channel[frame] = tone_a + (normalized_index * (tone_b - tone_a));
                    //                    *
                    //                    (.5f * (1.f * cosf((2.f * M_PI * normalize_value(frame_start, frame_split, frame)) / 4.f))); // 4.f covers the entire two-second tone pair; 2.f assumes the split frame evenly divides the duration between the two tones (one second each)
                    !(frequency_theta[0] > DBL_M_PI) ?: (frequency_theta[0] -= DBL_M_PI);
                    !(frequency_theta[1] > DBL_M_PI) ?: (frequency_theta[1] -= DBL_M_PI);
                }
            });
            
            ({
                frequencies[2] = random_musical_note_generator();//scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
                frequency_theta_increment[2] = DBL_M_PI * frequencies[2] / buffer_format.sampleRate;
                frequencies[3] = random_musical_note_generator();//scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
                frequency_theta_increment[3] = DBL_M_PI * frequencies[3] / buffer_format.sampleRate;
            });
            ({
                for (; ++frame_split < frame_count; frame++)
                {
                    normalized_index = frame / frame_end;
                    Float32 tone_a = sinf(frequency_theta[2] += frequency_theta_increment[2]);
                    Float32 tone_b = sinf(frequency_theta[3] += frequency_theta_increment[3]);
                    channel[frame] = tone_a + (normalized_index * (tone_b - tone_a));
                    //                    *
                    //                    (.5f * (1.f * cosf((2.f * M_PI * normalize_value(frame_start, frame_split, frame)) / 4.f))); // 4.f covers the entire two-second tone pair; 2.f assumes the split frame evenly divides the duration between the two tones (one second each)
                    !(frequency_theta[2] > DBL_M_PI) ?: (frequency_theta[2] -= DBL_M_PI);
                    !(frequency_theta[3] > DBL_M_PI) ?: (frequency_theta[3] -= DBL_M_PI);
                }
            });
            
            // Replace random frequency with harmonic consonnance/disonnance scheme here...
            
            //            ({
            //                frequencies[fade_bit ^= 1] = scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
            //                frequency_theta_increment = DBL_M_PI * frequencies[fade_bit] / buffer_format.sampleRate;
            //                frequencies[fade_bit ^= 1] = scale(generate_normalized_random(), 440.0, 3000.0, 0.0, 1.0);
            //                frequency_theta_increment = DBL_M_PI * frequencies[fade_bit] / buffer_format.sampleRate;
            //
            //                for (AVAudioFrameCount frame = ++frame_split; frame < frame_end; frame++)
            //                {
            //                    channel[frame] = sinf(frequency_theta += frequency_theta_increment);
            ////                    *
            ////                    (.5f * (1.f * cosf((2.f * M_PI * normalize_value(frame_start, frame_end, frame)) / 4.f))); // 4.f covers the entire two-second tone pair; 2.f assumes the split frame evenly divides the duration between the two tones (one second each)
            //                    !(frequency_theta > DBL_M_PI) ?: (frequency_theta -= DBL_M_PI);
            //                }
            //            });
        };
        
        buffer_signal = ^ (AVAudioPlayerNode * player_node) {
            AVAudioFrameCount left_duration = scale(scale(generate_normalized_random(), 0.5, 1.75, 0.0, 1.0), 0.0, frame_count, 0.5, 1.75);
            AVAudioFrameCount right_duration = scale(scale(generate_normalized_random(), 0.5, 1.75, 0.0, 1.0), 0.0, frame_count, 0.5, 1.75);
            
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
