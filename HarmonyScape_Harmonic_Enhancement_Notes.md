# HarmonyScape Harmonic Enhancement Notes
*Date: Current*

## Overview
This document describes the enhancements made to the chord generation and synthesis engine to create more colorful, musical, and enjoyable sounds.

## Key Changes Made

### 1. Enhanced Chord Generation Algorithm

The previous voicing algorithm was too mechanical - it simply added octave doublings based on density. The new algorithm provides:

#### For Single Notes:
- **Low Density (0-33%)**: Simple power chord feel with just a fifth and subtle bass
- **Medium Density (33-66%)**: Sus2/sus4 ambiguity creating open, colorful harmonies with 9ths
- **High Density (66-100%)**: Rich jazz voicings that vary by pitch class:
  - C/F: Maj9#11 for a bright, lydian sound
  - D/G/A: m11 for a sophisticated minor color
  - E/B: 7#9 for bluesy tension
  - Others: Sus4add9 for modal ambiguity

#### For Chord Input:
- **Low Density**: Adds strategic sparkle (2 octaves up) and bass foundation
- **Medium Density**: Adds color tones (7ths, 9ths) and selective upper voicing
- **High Density**: Creates rich upper structure triads with extensions (9, 11, 13)
- Analyzes existing chord to avoid duplicating important tones
- Respects MIDI range limits (24-108)

### 2. Enhanced Waveform Generation

Replaced simple waveforms with band-limited, harmonically rich versions:

#### Sine Wave:
- Added subtle 2nd and 3rd harmonics for warmth
- Creates a more organic, less clinical tone

#### Saw Wave:
- Band-limited with 8 harmonics to reduce aliasing
- Proper 1/n harmonic series for authentic saw sound

#### Square Wave:
- Band-limited with odd harmonics only (up to 7th)
- Classic square wave character without harsh aliasing

#### Triangle Wave:
- Added subtle detuning (chorus effect) for thickness
- Two slightly detuned triangles mixed for movement

### 3. Enhanced Voice Rendering

Added several features to make each voice more dynamic and musical:

#### Pitch Modulation:
- Subtle vibrato (4.4Hz) with amount based on chord position
- Higher chord tones get slightly more modulation for expressiveness
- Initial phase offset based on note start time for organic feel

#### Dynamic Filtering:
- Filter cutoff varies with:
  - Note pitch (higher notes = brighter)
  - Envelope level (filter opens with attack)
  - Chord position (higher positions get more resonance)
- Resonant filter adds character and movement

#### Stereo Movement:
- Subtle automatic panning based on LFO
- Each voice has unique movement pattern
- Creates a living, breathing stereo field

#### Soft Saturation:
- Tanh saturation adds warmth and glues voices together
- Prevents digital harshness

## Musical Benefits

1. **Single Note Improvisation**: Playing single notes now generates musically interesting harmonies that vary by pitch, creating a more engaging experience

2. **Chord Enhancement**: Instead of mechanical doubling, chords are enhanced with appropriate tensions and extensions based on their type

3. **Timbral Variety**: The enhanced waveforms and filtering create a richer, more analog-like sound

4. **Spatial Interest**: The subtle movement and positioning creates an immersive stereo field

5. **Dynamic Response**: The envelope-following filter and pitch modulation make each note feel alive

## Testing Recommendations

1. **Single Notes**: Play scales slowly to hear how different pitches generate different harmonic colors
2. **Density Sweep**: Try the same note/chord at different density settings to hear the harmonic evolution
3. **Waveform Comparison**: Switch between waveforms to hear the enhanced character of each
4. **Chord Types**: Try major, minor, and seventh chords to hear appropriate extensions
5. **Fast Passages**: Play rapid notes to test the voice allocation and envelope behavior

## Future Enhancement Ideas

1. **Scale Awareness**: Detect the current key/scale for even more appropriate harmonization
2. **Chord Progression Memory**: Analyze recent chords for better voice leading
3. **Microtuning**: Add subtle pitch variations for more organic ensemble sound
4. **Formant Filtering**: Add vowel-like resonances for more vocal character
5. **Dynamic Waveform Morphing**: Smooth transitions between waveforms

The synth should now produce much more colorful, musical, and enjoyable sounds that inspire improvisation and creativity! 