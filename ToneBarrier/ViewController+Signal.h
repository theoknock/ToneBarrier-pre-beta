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
#import <simd/simd.h>
#import <Accelerate/Accelerate.h>

#include "easing.h"
#include <math.h>
#include <time.h>
#include <stdio.h>


NS_ASSUME_NONNULL_BEGIN

#define PI 3.1415926535897932384626
#define D_PI simd_make_double2(2.f * M_PI)

static simd_double1 (^scale)(simd_double1, simd_double1, simd_double1, simd_double1, simd_double1) = ^ simd_double1 (simd_double1 val_old, simd_double1 min_new, simd_double1 max_new, simd_double1 min_old, simd_double1 max_old) {
   simd_double1 val_new = min_new + ((((val_old - min_old) * (max_new - min_new))) / (max_old - min_old));
   
   return val_new;
};

static simd_double1 (^normalize_value)(simd_double1, simd_double1, simd_double1) = ^simd_double1(simd_double1 min, simd_double1 max, simd_double1 value) {
   simd_double1 result = (value - min) / (max - min);
   return result;
};

static simd_double1 (^(^normalized_random_generator)(void))(void) = ^{
   srand48((unsigned int)clock());
   static simd_double1 random;
   return ^ (simd_double1 * random_t) {
      return ^ simd_double1 {
         return (*random_t = (drand48()));
      };
   }(&random);
};

typedef typeof(simd_double1(^)(void)) random_generator;
typedef typeof(simd_double1(^(* restrict))(void)) random_n_t;
static simd_double1 (^(^(^(^generate_random)(simd_double1(^)(void)))(simd_double1(^)(simd_double1)))(simd_double1(^)(simd_double1)))(void) = ^ (simd_double1(^randomize)(void)) {
   return ^ (simd_double1(^distribute)(simd_double1)) {
      return ^ (simd_double1(^scale)(simd_double1)) {
         return ^ simd_double1 {
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
static simd_double1 (^tonal_interval)(TonalInterval) = ^ simd_double1 (TonalInterval interval) {
   simd_double1 consonant_harmonic_interval_ratios [8] = {1.0, 2.0, 5.0/3.0, 4.0/3.0, 5.0/4.0, 6.0/5.0, (1.1 + drand48()), 5.0/4.0};
   return consonant_harmonic_interval_ratios[interval % TonalIntervalDefault];
};

static typeof(AVAudioPCMBuffer *) audio_buffer_ref = NULL;

static simd_double1 (^generate_normalized_random)(void);
volatile random_generator random_musical_note_generator;
volatile random_generator random_tone_duration_generator;

static simd_double1 (^gaussian_distribution)(simd_double1, simd_double1, simd_double1) = ^ simd_double1 (simd_double1 x, simd_double1 mean, simd_double1 variance) {
   simd_double1 j = (exp(-((pow((x - mean), 2.f) / pow(2.f * variance, 2.f))))); //(1.f / (std_dev * sqrt(2.f * M_PI))) * (exp(-((pow((x - mean), 2.f) / pow(2.f * variance, 2.f))))); // divide a mean of .5 or less by 4 to get a variance value that extends to the x = 0
   //    simd_float1 scaled_gaussian_distribution = 0.f + (((j - 0.f) * (1.f - 0.f)) / ((1.f - 0.f) - 0.f));
   
   //   simd_float1 s = sinf(M_PI * x);
   
   return j;
};

static simd_double1 (^logistic_function)(simd_double1, simd_double1) = ^ simd_double1 (simd_double1 x, simd_double1 c) {
   simd_double1 sigma = 1.0 / (1.0 + exp(-1 * c) * (x- (x / 2.f)));
   return sigma;
};

// Collection pointer
typedef typeof(void **(^)(void)) source;
static typeof(source) (^_Nonnull collection)(size_t) = ^ (size_t length) {
   typedef void * element_collection[length];
   typeof(element_collection) elements[length];
   __block void ** elements_t = elements[0];
   return ^ (void ** elements_ptr) {
      return (^ void ** {
         return elements_ptr;
      });
   }(elements_t);
};

// Element pointer generator (element access)
typedef typeof(void **(^)(size_t)) stream;
static typeof(stream) (^ _Nonnull elements)(typeof(source), size_t) = ^ (typeof(source) source, size_t stride) {
   return ^ (void ** source_ptr) {
      return (^ void ** (size_t index) {
         return ((void **)source_ptr + (index * stride));
      });
   }(source());
};

// Element index iterator (element traversal)
//

// Applies an operation to an element in a given collection using the supplied pointer
static void (^(^operation)(void *))(void **) = ^ (void * value) {
   return ^ (void ** element_ptr) {
      *(element_ptr) = value;
   };
};

// Maps a value to a given collection and returns a block that returns a pointer to the source
//static typeof(source) (^map)(typeof(stream), ) = ^ (typeof(stream) stream) {
//
//};

static void (^generate_signal_sample)(void);
static void (^(^signal_sample_generator)(float * _Nonnull const * _Nonnull, AVAudioFrameCount, simd_double1 *))(void) = ^ (float * _Nonnull const * _Nonnull float_channel_data, AVAudioFrameCount buffer_length, simd_double1 * normalized_time) {
   AVAudioFrameCount frame = 0;
   AVAudioFrameCount * frame_t = &frame;
   __block simd_double1 duration_split;
   __block simd_double2 tone_durations;
   __block simd_double2 tones;
   __block simd_double2 frequency_theta_v;
   __block simd_double2 frequency_theta_increment_v;
   __block simd_double2 envelope_theta_v;
   __block simd_double2 envelope_theta_increment_v;
   
   return ^{
      
      frequency_theta_v = simd_make_double2(0.0, 0.0);
      frequency_theta_increment_v = simd_make_double2((2.f * M_PI) * random_musical_note_generator() / buffer_length,
                                                      (2.f * M_PI) * random_musical_note_generator() / buffer_length);
      
      simd_double2 time = simd_make_double2((frequency_theta_increment_v[0] * buffer_length),
                                            (frequency_theta_increment_v[1] * buffer_length));
      
      envelope_theta_v = simd_make_double2(0.0, 0.0);
      duration_split = scale(random_tone_duration_generator() * 2.f, 0.25, 1.75, 0.0, 2.0) * buffer_length;
      tone_durations = simd_make_double2(duration_split, 2.0 - duration_split);
      envelope_theta_increment_v = simd_make_double2(M_PI / buffer_length, M_PI / buffer_length);
      
      for (*frame_t = 0; *frame_t < buffer_length; *frame_t += 1) {
         ({
            ({
               //               time = (simd_double1)*((simd_double1 *)normalized_time + (*frame_t));
               
               //               tone_durations = simd_make_double2(_simd_sinpi_d2(simd_make_double2(pow(((frequency_theta_increment_v[0] * (*frame_t)) / time[0]), 0.25f),
               //                                                                                   pow(((frequency_theta_increment_v[1] * (*frame_t)) / time[1]), 0.75f))));
               tones = simd_make_double2(_simd_sin_d2(simd_make_double2( ({ (frequency_theta_v = simd_make_double2(frequency_theta_v + frequency_theta_increment_v)); }) )) *
                                         _simd_sin_d2(simd_make_double2( ({ (envelope_theta_v = simd_make_double2(envelope_theta_v + envelope_theta_increment_v)); }) )));
               //               tone_durations = simd_make_double2(gaussian_distribution((frequency_theta_increment_v[0] * (*frame_t)) / time[0], 0.f, 1.f), gaussian_distribution((  frequency_theta_increment_v[1] * (*frame_t)) / time[0], 0.f, 1.f));
               //               tone_durations *= simd_make_double2(logistic_function(tone_durations[0], 1.f), logistic_function(tone_durations[1], 100.f));
               //               tone_durations = simd_make_double2(logistic_function(time, 1.f), logistic_function(time, 1.f));
               //               frequency_theta_increment_v = (frequency_theta_increment_v + simd_make_double2(gaussian_distribution(time, 0.f, 1.f), gaussian_distribution(time, 0.f, 1.f)));
               
               ((!(*(frame_t) <=  duration_split) && ( ({ envelope_theta_v[0] = 0.f; }) )));
               ((!(*(frame_t) >=  duration_split) && ( ({ envelope_theta_v[1] = 0.f; }) ))); // ( ({ (envelope_theta_v[1] = (simd_double1)(envelope_theta_v[1] + envelope_theta_increment_v[1])); }) )); //(simd_double1((frequency_theta_v[0] = frequency_theta_v[0] + frequency_theta_increment_v[0];
               
               !(frequency_theta_v > D_PI) && (frequency_theta_v -= D_PI);
               (!(envelope_theta_v > D_PI) && (envelope_theta_v  -= D_PI));
            });
            ({
               *((float *)float_channel_data[0] + *frame_t) = tones[0];
               *((float *)float_channel_data[1] + *frame_t) = tones[1];
            });
         });
      }
   };
};

static typeof(void (^)(void)) buffer_signal = ^{
   generate_signal_sample();
   [player_node_ref scheduleBuffer:audio_buffer_ref atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
      if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) if ([player_node_ref isPlaying]) buffer_signal();
   }];
};

static typeof(audio_buffer_ref) (^audio_buffer)(AVAudioFormat *) = ^ (AVAudioFormat * buffer_format) {
   AVAudioFrameCount frame_count = buffer_format.sampleRate * buffer_format.channelCount;
   static dispatch_once_t onceToken;
   dispatch_once(&onceToken, ^{
      audio_buffer_ref = [[AVAudioPCMBuffer alloc] initWithPCMFormat:buffer_format frameCapacity:frame_count];
      audio_buffer_ref.frameLength = frame_count;
      generate_normalized_random = normalized_random_generator();
      random_musical_note_generator  = generate_random(generate_normalized_random)(^ simd_double1 (simd_double1 n) { return n; })(^ simd_double1 (simd_double1 n) { return pow(2.f, round(scale(n, MusicalNoteA, MusicalNoteAFlat, 0.0, 1.0))/12.f) * 440.f; });
      random_tone_duration_generator = generate_random(generate_normalized_random)(^ simd_double1 (simd_double1 n) { return normalize_value(0.0, 2.0, n); })(^ simd_double1 (simd_double1 n) { return scale(n, 0.25, 1.75, 0.0, 2.0); });
      
      simd_double1 normalized_time[frame_count];
      simd_double1 * normalized_time_t = &normalized_time[0];
      for (AVAudioFrameCount frame = 0; frame < frame_count; frame++) {
         *((simd_double1 *)normalized_time_t + frame) = simd_smoothstep((simd_double1)0.f, (simd_double1)frame_count, (simd_double1)frame);
      }
      generate_signal_sample = signal_sample_generator(audio_buffer_ref.floatChannelData, audio_buffer_ref.frameLength, normalized_time_t);
      
      
   });
   //        simd_float1 (^generate_normalized_random)(void) = normalized_random_generator();
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
