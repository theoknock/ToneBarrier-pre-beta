//
//  ToneGenerator.m
//  JABPlanetaryHourToneBarrier
//
//  Created by Xcode Developer on 7/8/19.
//  Copyright © 2019 The Life of a Demoniac. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>

#import "ToneGenerator.h"
#import "ToneBarrierPlayer.h"
#import "ClicklessTones.h"
#import "FrequenciesPairs.h"
#import "Frequencies.h"

#include "easing.h"

//static const float high_frequency = 6000.0;
//static const float low_frequency  = 1000.0;

@interface ToneGenerator ()

@property (nonatomic, readonly) AVAudioMixerNode * mixerNode;
@property (nonatomic, readonly) AVAudioPCMBuffer * pcmBufferOne;
@property (nonatomic, readonly) AVAudioPCMBuffer * pcmBufferTwo;

@end

@implementation ToneGenerator

- (NSUInteger)greatestCommonDivisor:(NSUInteger)firstValue secondValue:(NSUInteger)secondValue
{
    if (firstValue == 0 && secondValue == 0)
        return 1;
    
    NSUInteger r;
    while(secondValue)
    {
        r = firstValue % secondValue;
        firstValue = secondValue;
        secondValue = r;
    }
    return firstValue;
}

static ToneGenerator *sharedGenerator = NULL;
+ (nonnull ToneGenerator *)sharedGenerator
{
    static dispatch_once_t onceSecurePredicate;
    dispatch_once(&onceSecurePredicate,^
                  {
        if (!sharedGenerator)
        {
            sharedGenerator = [[self alloc] init];
        }
    });
    
    return sharedGenerator;
}

- (instancetype)init
{
    self = [super init];
    
    if (self)
    {
        //        semaphore = dispatch_semaphore_create(1);
        _audioEngine = [[AVAudioEngine alloc] init];
        
        _mixerNode = _audioEngine.mainMixerNode;
        
        _playerOneNode = [[AVAudioPlayerNode alloc] init];
        [_audioEngine attachNode:_playerOneNode];
        
        [_audioEngine connect:_playerOneNode to:_mixerNode format:[_playerOneNode outputFormatForBus:0]];
        
        _playerTwoNode = [[AVAudioPlayerNode alloc] init];
        [_audioEngine attachNode:_playerTwoNode];
        
        [_audioEngine connect:_playerTwoNode to:_mixerNode format:[_playerTwoNode outputFormatForBus:0]];
        
        __autoreleasing NSError *error = nil;
        //        [_audioEngine startAndReturnError:&error];
        
        [[AVAudioSession sharedInstance] setActive:YES error:&error];
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
    }
    
    return self;
}

- (float)generateRandomNumberBetweenMin:(int)min Max:(int)max
{
    return ( (arc4random() % (max-min+1)) + min );
}



//- (AVAudioPCMBuffer *)createAudioBufferWithLoopableSineWaveFrequency:(NSUInteger)frequency
//{
//    AVAudioFormat *mixerFormat = [_mixerNode outputFormatForBus:0];
//    NSUInteger randomNum = [self generateRandomNumberBetweenMin:1 Max:4];
//    double frameLength = mixerFormat.sampleRate / randomNum;
//    AVAudioPCMBuffer *pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:mixerFormat frameCapacity:frameLength];
//    pcmBuffer.frameLength = frameLength;
//
//    float *leftChannel = pcmBuffer.floatChannelData[0];
//    float *rightChannel = mixerFormat.channelCount == 2 ? pcmBuffer.floatChannelData[1] : nil;
//
//    NSUInteger r = arc4random_uniform(2);
//    double amplitude_step  = (1.0 / frameLength > 0.000100) ? (((double)arc4random() / 0x100000000) * (0.000100 - 0.000021) + 0.000021) : 1.0 / frameLength;
//    double amplitude_value = 0.0;
//    for (int i_sample = 0; i_sample < pcmBuffer.frameCapacity; i_sample++)
//    {
//        amplitude_value += amplitude_step;
//        double amplitude = pow(((r == 1) ? ((amplitude_value < 1.0) ? (amplitude_value) : 1.0) : ((1.0 - amplitude_value > 0.0) ? 1.0 - (amplitude_value) : 0.0)), ((r == 1) ? randomNum : 1.0/randomNum));
//        amplitude = ((amplitude < 0.000001) ? 0.000001 : amplitude);
//        double value = sinf((frequency*i_sample*2*M_PI) / mixerFormat.sampleRate);
//        if (leftChannel)  leftChannel[i_sample]  = value * amplitude;
//        if (rightChannel) rightChannel[i_sample] = value * (1.0 - amplitude);
//    }
//
//    return pcmBuffer;
//}

//

//- (void)start
//{
//    static void (^recursive_block)(double, AudioBufferCompletionBlock);
//    recursive_block = ^(double first_frequency, AudioBufferCompletionBlock audioBufferCompletionBlock)
//    {
//        // create audio buffer with first_frequency
//        // generate second frequency
//        // generate third frequency
//        // create audio buffer with interstitial frequency by averaging first and second frequency
//        // create audio buffer with second frequency
//        // create audio buffer with interstitial frequency by averaging second and third frequency
//        // return audio buffers with CreateAudioBufferCompletionBlock
//        // pass third frequency to recursive_block as first_frequency when PlayToneCompletionBlock is called
//        audioBufferCompletionBlock([AVAudioPCMBuffer new], ^{
//
//        });
//    };
//
//    recursive_block((((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency), ^(AVAudioPCMBuffer *buffer, ToneCompletionBlock toneCompletionBlock)
//    {
//        [self->_playerOneNode scheduleBuffer:buffer atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
//            if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack)
//                toneCompletionBlock();
//            NSLog(@"Calling playToneCompletionBlock 2...");
//        }];
//    });
//}

/// / To-Do: Use dispatch_io to read buffers instead

static const float high_frequency = 6000.0;
static const float low_frequency  = 1000.0;

NSArray<NSDictionary<NSString *, id> *> *(^tonesDictionary)(void) = ^NSArray<NSDictionary<NSString *, id> *> *(void)
{
    NSMutableArray *tones = [NSMutableArray arrayWithCapacity:90];
    for (int i = 0; i < 180; i++)
    {
        AVAudioTime *time = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
        NSDictionary *td = @{@"time" : time,
                             @"frequency_right" : @((((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency)),
                             @"frequency_left" : @((((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency))};
        [tones addObject:td];
        
        float randomNum = (((double)arc4random() / 0x100000000) * (1.0 - 0.0) + 0.0);
        CMTime current_cmtime = CMTimeAdd(CMClockGetTime(CMClockGetHostTimeClock()), CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC));
        time = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(current_cmtime)];
        td = @{@"time" : time,
               @"frequency_right" : @((((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency)),
               @"frequency_left" : @((((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency))};
        [tones addObject:td];
    }
    
    return tones;
};

- (void)start
{
    [[AVAudioSession sharedInstance] setActive:YES error:nil];

    if (self.audioEngine.isRunning == NO)
    {
        NSError *error = nil;
        [_audioEngine startAndReturnError:&error];
        NSLog(@"error: %@", error);
        [[NSNotificationCenter defaultCenter] postNotificationName:@"ToneBarrierPlayingNotification" object:nil userInfo:nil];

        if (![self->_playerOneNode isPlaying] || ![self->_playerTwoNode isPlaying])
        {
            [self->_playerOneNode play];
            [self->_playerTwoNode play];
        }

        if (self->_playerOneNode)
        {
            //        [self createAudioBufferWithCompletionBlock:^(AVAudioPCMBuffer *buffer1, AVAudioPCMBuffer *buffer2, PlayToneCompletionBlock playToneCompletionBlock) {
            //            [self->_playerOneNode scheduleBuffer:buffer1 atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
            //                //                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack)
            //                //                    NSLog(@"Calling playToneCompletionBlock 1...");
            //            }];
            //            [self->_playerTwoNode scheduleBuffer:buffer2 atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
            //                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack)
            //                    playToneCompletionBlock();
            //                //                NSLog(@"Calling playToneCompletionBlock 2...");
            //            }];
            //        }];
            //            ToneBarrierPlayer *player = [[ToneBarrierPlayer alloc] init];
            ClicklessTones *tones = [[ClicklessTones alloc] init];

            [ToneBarrierPlayer.context setPlayer:(id<ToneBarrierPlayerDelegate> _Nonnull)tones];
            [ToneBarrierPlayer.context createAudioBufferWithFormat:[self->_mixerNode outputFormatForBus:0] completionBlock:^(AVAudioPCMBuffer * _Nonnull buffer1, AVAudioPCMBuffer * _Nonnull buffer2, PlayToneCompletionBlock playToneCompletionBlock) {
                [self->_playerOneNode scheduleBuffer:buffer1 atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
                    //                if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack)
                    //                    NSLog(@"Calling playToneCompletionBlock 1...");
                }];
                [self->_playerTwoNode scheduleBuffer:buffer2 atTime:nil options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
                    if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack)
                        playToneCompletionBlock();
                    //                NSLog(@"Calling playToneCompletionBlock 2...");
                }];
            }];
        }
    }
}

NSArray<Frequencies *> * (^pairFrequencies)(Frequencies *, AVAudioTime *) = ^NSArray<Frequencies *> * (Frequencies *returnFrequencies, AVAudioTime *time)
{
    StereoChannel returnFrequenciesStereoChannel = (StereoChannel)returnFrequencies.channel.unsignedIntValue;
    [returnFrequencies setChannel:(returnFrequenciesStereoChannel != StereoChannelLeft) ? @(StereoChannelLeft) : @(StereoChannelRight)];
    
    NSMutableArray<Frequencies *> *pairs = [NSMutableArray new];
    [pairs addObject:[[Frequencies alloc] initWithFrequency1:returnFrequencies.frequency1.doubleValue frequency2:returnFrequencies.frequency2.doubleValue stereoChannel:(StereoChannel)returnFrequencies.channel.unsignedIntValue time:time]];
    [pairs addObject:[[Frequencies alloc] initWithFrequency1:(((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency) frequency2:(((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency) stereoChannel:returnFrequenciesStereoChannel time:time]];
    
    NSLog(@"Return frequencies (1st): %f\t%f", pairs.firstObject.frequency1.doubleValue, pairs.firstObject.frequency2.doubleValue);
    NSLog(@"Return frequencies (2nd): %f\t%f", pairs.lastObject.frequency1.doubleValue, pairs.lastObject.frequency2.doubleValue);
    NSLog(@"Time: %f", [AVAudioTime secondsForHostTime:[pairs.firstObject.time hostTime]]);
    
    return (NSArray<Frequencies *> *)pairs;
};

AVAudioTime *(^bufferSchedule)(CMTime, NSUInteger) = ^AVAudioTime *(CMTime current_time, NSUInteger count)
{
    CMTime scheduled_time = CMTimeAdd(current_time, CMTimeMakeWithSeconds(count, NSEC_PER_SEC));
    AVAudioTime *time = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(scheduled_time)];
    
    return time;
};

NSArray<NSArray<Frequencies *> *> *(^scoreFrequenciesPairs)(void) = ^NSArray<NSArray<Frequencies *> *> *(void)
{
    __block NSMutableArray *score = [NSMutableArray new];
    CMTime current_time = CMClockGetTime(CMClockGetHostTimeClock());
    __block NSUInteger count = 0;
    static void (^block)(Frequencies *);
    block = ^void(Frequencies *returnFrequencies)
    {
        NSArray<Frequencies *> *frequenciesPair = pairFrequencies(returnFrequencies, bufferSchedule(current_time, count++));
        [score addObject:frequenciesPair];
        if (score.count < 90) block([frequenciesPair lastObject]);
        //        if ([ToneGenerator.sharedGenerator.audioEngine isRunning]) block([frequenciesPair lastObject]); // Can't do this without a completion block that returns every frequenciesPair array
    };
    block(nil);
    
    return score;
};

AVAudioPCMBuffer * (^audioBufferFromFrequencies)(Frequencies *, AVAudioFormat *) = ^AVAudioPCMBuffer *(Frequencies *frequencies, AVAudioFormat *audioFormat)
{
    AVAudioFrameCount frameCount = audioFormat.sampleRate;
    AVAudioPCMBuffer *pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:audioFormat frameCapacity:frameCount];
    pcmBuffer.frameLength = frameCount;
    float *left_channel  = pcmBuffer.floatChannelData[0];
    float *right_channel = (audioFormat.channelCount == 2) ? pcmBuffer.floatChannelData[1] : nil;
    
    int amplitude_frequency = arc4random_uniform(8) + 4;
    for (int index = 0; index < frameCount; index++)
    {
        double normalized_index = LinearInterpolation(index, frameCount);
        if (left_channel)  left_channel[index]  = (NormalizedSineEaseInOut(normalized_index, frequencies.frequency1.doubleValue) * NormalizedSineEaseInOut(normalized_index, amplitude_frequency));
        if (right_channel) right_channel[index] = (NormalizedSineEaseInOut(normalized_index, frequencies.frequency2.doubleValue) * NormalizedSineEaseInOut(normalized_index, amplitude_frequency)); // fade((leading_fade == FadeOut) ? FadeIn : leading_fade, normalized_index, (SineEaseInOutFrequency(normalized_index, frequencyRight) * NormalizedSineEaseInOutAmplitude((1.0 - normalized_index), 1)));
    }
    
    return pcmBuffer;
};

void (^scheduleBuffers)(AVAudioPlayerNode *, AVAudioPlayerNode *, AVAudioFormat *) = ^void(AVAudioPlayerNode *playerNode1, AVAudioPlayerNode *playerNode2, AVAudioFormat *audioFormat)
{
//    AVAudioTime *time = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
    NSArray<NSArray<Frequencies *> *> *score = (NSArray<NSArray<Frequencies *> *> *)[NSArray arrayWithArray:scoreFrequenciesPairs()];
    for (NSUInteger index = 0; index < score.count; index++)  // (NSArray<Frequencies *> *frequencyPair in score)
    {
        NSArray<Frequencies *> *frequencyPair = (NSArray<Frequencies *> *)[score objectAtIndex:index];
        [playerNode1 scheduleBuffer:audioBufferFromFrequencies([frequencyPair firstObject], audioFormat) atTime:[[frequencyPair firstObject] time] options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
            NSLog(@"%lu Played frequencies (1st): %f\t%f\t%f", index, frequencyPair.firstObject.frequency1.doubleValue, frequencyPair.firstObject.frequency2.doubleValue, [AVAudioTime secondsForHostTime:[frequencyPair.firstObject.time hostTime]]);
        }];
        [playerNode2 scheduleBuffer:audioBufferFromFrequencies([frequencyPair lastObject], audioFormat) atTime:[[frequencyPair lastObject] time] options:AVAudioPlayerNodeBufferInterruptsAtLoop completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack completionHandler:^(AVAudioPlayerNodeCompletionCallbackType callbackType) {
           NSLog(@"%lu Played frequencies (2nd): %f\t%f\t%f", index, frequencyPair.lastObject.frequency1.doubleValue, frequencyPair.lastObject.frequency2.doubleValue, [AVAudioTime secondsForHostTime:[frequencyPair.lastObject.time hostTime]]);
        }];
    }
};

//- (void)start
//{
//    [[AVAudioSession sharedInstance] setActive:YES error:nil];
//
//    if (self.audioEngine.isRunning == NO)
//    {
//        NSError *error = nil;
//        [_audioEngine startAndReturnError:&error];
//        NSLog(@"error: %@", error);
//        [[NSNotificationCenter defaultCenter] postNotificationName:@"ToneBarrierPlayingNotification" object:nil userInfo:nil];
//
//        if (![self->_playerOneNode isPlaying] || ![self->_playerTwoNode isPlaying])
//        {
//            [self->_playerOneNode play];
//            [self->_playerTwoNode play];
//        }
//
//        if (self->_playerOneNode && self->_playerTwoNode)
//        {
//            scheduleBuffers(self->_playerOneNode, self->_playerTwoNode, [self->_mixerNode outputFormatForBus:0]);
//        }
//    }
//}

/*
 BARRIER ONE
 */

//- (void)start
//{
//    if (self.audioEngine.isRunning == NO)
//    {
//        NSError *error = nil;
//        [_audioEngine startAndReturnError:&error];
//        NSLog(@"error: %@", error);
//    }
//
//
//    if (self->_timer != nil) [self stop];
//    self.timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
//    dispatch_source_set_timer(self->_timer, DISPATCH_TIME_NOW, 2.0 * NSEC_PER_SEC, 0.0 * NSEC_PER_SEC);
//    dispatch_source_set_event_handler(self->_timer, ^{
//        if ([self->_playerOneNode isPlaying] || [self->_playerTwoNode isPlaying])
//        {
//            [self->_playerOneNode stop];
//            [self->_playerTwoNode stop];
//        }
//
//        if (self->_playerOneNode)
//        {
//            AVAudioTime *start_time_one = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
//
//            [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:(((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency)] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//
//            }];
//
//            [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:(((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency)] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//
//            }];
//
//            [self->_playerOneNode play];
//        }
//    });
//    dispatch_resume(self.timer);
//}

/*
 BARRIER TWO
 */

// - (void)start
// {
//     if (self.audioEngine.isRunning == NO)
//     {
//         NSError *error = nil;
//         [_audioEngine startAndReturnError:&error];
//         NSLog(@"error: %@", error);
//     }
//
//     if (self->_timer != nil) [self stop];
//
//     self.timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
//     dispatch_source_set_timer(self->_timer, DISPATCH_TIME_NOW, 2.0 * NSEC_PER_SEC, 0.0 * NSEC_PER_SEC);
//     dispatch_source_set_event_handler(self->_timer, ^{
//         if (![self->_playerOneNode isPlaying] || ![self->_playerTwoNode isPlaying])
//         {
//             [self->_playerOneNode play];
//             [self->_playerTwoNode play];
//         }
//
//         if (self->_playerOneNode && self->_playerTwoNode)
//         {
//             AVAudioTime *start_time_one = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
//             double frequencyOne = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//             [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
////                 [self.toneWaveRendererDelegate drawFrequency:frequencyOne amplitude:1.0 channel:StereoChannelR];
//             }];
//
//             float randomNum = (((double)arc4random() / 0x100000000) * (1.0 - 0.0) + 0.0); //((float)rand() / RAND_MAX) * 1;
//             CMTime current_cmtime = CMTimeAdd(CMClockGetTime(CMClockGetHostTimeClock()), CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC));
//             AVAudioTime *start_time_two = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(current_cmtime)];
////
//             double frequencyTwo = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//             [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyTwo] atTime:start_time_two options:AVAudioPlayerNodeBufferLoops completionHandler:^{
////                 [self.toneWaveRendererDelegate drawFrequency:frequencyTwo amplitude:1.0/2.0 channel:StereoChannelL];
//             }];
////         }
////
////
////          if (self->_playerOneNode && self->_playerTwoNode)
////          {
////              float randomNum = ((float)rand() / RAND_MAX) * 1;
////              CMTime current_cmtime = CMTimeAdd(CMClockGetTime(CMClockGetHostTimeClock()), CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC));
////              AVAudioTime *start_time_three = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(current_cmtime)];
////              double frequencyOne = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
////              [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_three options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//////                  [self.toneWaveRendererDelegate drawFrequency:frequencyOne amplitude:1.0 channel:StereoChannelR];
////              }];
////
////              randomNum = ((float)rand() / RAND_MAX) * 1;
////              current_cmtime = CMTimeAdd(CMClockGetTime(CMClockGetHostTimeClock()), CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC));
////              AVAudioTime *start_time_four = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(current_cmtime)];
////
////              double frequencyTwo = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
////              [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyTwo] atTime:start_time_four options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//////                  [self.toneWaveRendererDelegate drawFrequency:frequencyTwo amplitude:1.0/3.0 channel:StereoChannelL];
////              }];
//          }
//     });
//     dispatch_resume(self.timer);
// }

/*
 BARRIER THREE
 */

//- (void)start
//{
//    if (self.audioEngine.isRunning == NO)
//    {
//        NSError *error = nil;
//        [_audioEngine startAndReturnError:&error];
//        NSLog(@"error: %@", error);
//    }
//
//    if (self->_timer != nil) [self stop];
//
//    self.timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
//    dispatch_source_set_timer(self->_timer, DISPATCH_TIME_NOW, 2.0 * NSEC_PER_SEC, 0.0 * NSEC_PER_SEC);
//    dispatch_source_set_event_handler(self->_timer, ^{
//        if (![self->_playerOneNode isPlaying] || ![self->_playerTwoNode isPlaying])
//        {
//            [self->_playerOneNode play];
//            [self->_playerTwoNode play];
//        }
//
//        NSUInteger r = arc4random_uniform(2);
//        switch (r) {
//            case 1: {
//                CMTime ctime = CMClockGetTime(CMClockGetHostTimeClock());
//                uint64_t htime = CMClockConvertHostTimeToSystemUnits(ctime);
//                if (self->_playerOneNode && self->_playerTwoNode)
//                {
//                    double freq = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:htime]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:htime]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    freq = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    float randomNum = ((float)rand() / RAND_MAX) * 1;
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    freq = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    randomNum += ((float)rand() / RAND_MAX) * 1;
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    freq = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    randomNum += ((float)rand() / RAND_MAX) * 1;
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:freq]
//                                                  atTime:[[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMTimeAdd(ctime, CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC)))]
//                                                 options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    }];
//                 }
//                break;
//            }
//
//            default: {
//                AVAudioTime *start_time_one = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
//                if (self->_playerOneNode && self->_playerTwoNode)
//                {
//                    double frequencyOne = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    double frequencyTwo = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    //                         [self.toneWaveRendererDelegate drawFrequency:frequencyOne amplitude:1.0 channel:StereoChannelR];
//                    }];
//
//
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyTwo] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    //                         [self.toneWaveRendererDelegate drawFrequency:frequencyTwo amplitude:1.0 channel:StereoChannelL];
//                    }];
//                }
//
//                float randomNum = ((float)rand() / RAND_MAX) * 1;
//                CMTime current_cmtime = CMTimeAdd(CMClockGetTime(CMClockGetHostTimeClock()), CMTimeMakeWithSeconds(randomNum, NSEC_PER_SEC));
//                AVAudioTime *start_time_two = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(current_cmtime)];
//                if (self->_playerOneNode && self->_playerTwoNode)
//                {
//                    double frequencyOne = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_two options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    //                          [self.toneWaveRendererDelegate drawFrequency:frequencyOne amplitude:1.0 channel:StereoChannelR];
//                    }];
//
//                    double frequencyTwo = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//                    [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyTwo] atTime:start_time_two options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//                    //                          [self.toneWaveRendererDelegate drawFrequency:frequencyTwo amplitude:1.0 channel:StereoChannelL];
//                    }];
//                }
//                }
//                break;
//        }
//
//
//    });
//    dispatch_resume(self.timer);
//}

//
//typedef NS_OPTIONS(NSUInteger, Texture) {
//    Monophonic = 1 << 0,
//    Heterophonic = 1 << 1
//};
//
//// Phonaesthesia: the study of aesthetic properties of sounds
//typedef NS_OPTIONS(NSUInteger, Phonaesthetic) {
//    Cacophonic = 1 << 0,
//    Euphonic = 1 << 1
//};
//
//typedef NS_ENUM(NSUInteger, Rhythm) {
//    Monody,
//    Heterodony
//};
//
//
//static void (^generateTone)(AVAudioPlayerNode *) = ^(AVAudioPlayerNode *playerNode) {
//    // Loop the number of stereoChannels
//    AVAudioTime *start_time_one = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
//        double frequencyOne = (((double)arc4random() / 0x100000000) * (high_frequency - low_frequency) + low_frequency);
//        [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
//            if ([self->_playerOneNode isPlaying] && [self->_playerTwoNode isPlaying])
//                generateTones(stereoChannels);
//        }];
//};
//
//- (void)play:(StereoChannels)stereoChannels
//{
//    if (self.audioEngine.isRunning == NO)
//    {
//        NSError *error = nil;
//        [_audioEngine startAndReturnError:&error];
//        NSLog(@"error: %@", error);
//    }
//
//    if ((stereoChannels & StereoChannelRight) && self->_playerOneNode)
//    {
//        if (![self->_playerOneNode isPlaying])
//            [self->_playerOneNode play];
//
//        generateTones(self->_playerOneNode);
//
//    }
//
//    if ((stereoChannels & StereoChannelLeft) && self->_playerTwoNode)
//    {
//        if (![self->_playerTwoNode isPlaying])
//            [self->_playerTwoNode play];
//
//        generateTones(self->_playerTwoNode);
//    }
//}

- (void)stop
{
    //    dispatch_source_cancel(self->_timer);
    //    self->_timer = nil;
    //    dispatch_async(dispatch_get_main_queue(), ^{
    //    [self->_playerOneNode stop];
    //    [self->_playerTwoNode stop];
    if (self.audioEngine.isRunning == YES) [self->_audioEngine pause];
    //        [self.playerOneNode reset];
    //        [self.playerTwoNode reset];
    
    
    [[NSNotificationCenter defaultCenter] postNotificationName:@"ToneBarrierPlayingNotification" object:nil userInfo:nil];
    //    });
}

//- (void)alarm
// {
//     if (self.audioEngine.isRunning == NO)
//     {
//         NSError *error = nil;
//         [_audioEngine startAndReturnError:&error];
//         NSLog(@"error: %@", error);
//     }
//
//     if (self->_timer != nil) [self stop];
//
//         if (![self->_playerOneNode isPlaying] || ![self->_playerTwoNode isPlaying])
//         {
//             [self->_playerOneNode play];
//             [self->_playerTwoNode play];
//         }
//
//         if (self->_playerOneNode && self->_playerTwoNode)
//         {
//             AVAudioTime *start_time_one = [[AVAudioTime alloc] initWithHostTime:CMClockConvertHostTimeToSystemUnits(CMClockGetTime(CMClockGetHostTimeClock()))];
//             double frequencyOne = (high_frequency + low_frequency) / 2.0;
//             [self->_playerOneNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyOne] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
////                 [self.toneWaveRendererDelegate drawFrequency:frequencyOne amplitude:1.0 channel:StereoChannelR];
//             }];
//
//             double frequencyTwo = high_frequency;
//             [self->_playerTwoNode scheduleBuffer:[self createAudioBufferWithLoopableSineWaveFrequency:frequencyTwo] atTime:start_time_one options:AVAudioPlayerNodeBufferLoops completionHandler:^{
////                 [self.toneWaveRendererDelegate drawFrequency:frequencyTwo amplitude:1.0/2.0 channel:StereoChannelL];
//             }];
//          }
// }

@end



