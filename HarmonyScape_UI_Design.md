# HarmonyScape UI Design
**User Interface Mockup and Component Specifications**

## UI Design Philosophy

The HarmonyScape interface is designed around four key principles:

1. **Visual Clarity** - Intuitive representation of complex harmonic and spatial concepts
2. **Workflow Efficiency** - Minimal clicks for common operations
3. **Musical Representation** - Appropriate visualization of musical concepts
4. **Scalability** - Adaptable from basic to advanced usage

The UI employs a dark theme with accent colors representing harmonic elements and utilizes both traditional music notation and innovative spatial visualizations.

## Main UI Layout

The main interface is divided into five primary sections:

```
┌────────────────────────────────────────────────────────────────────┐
│                       HEADER & PRESET BROWSER                      │
├────────────────────┬───────────────────────┬────────────────────────┤
│                    │                       │                        │
│                    │                       │                        │
│  CHORD INPUT       │  VOICING              │  SPATIAL              │
│  & SEQUENCER       │  VISUALIZATION        │  CONFIGURATION        │
│                    │                       │                        │
│                    │                       │                        │
├────────────────────┴───────────────────────┴────────────────────────┤
│                                                                     │
│                       PARAMETER CONTROLS                            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. Header & Preset Browser

**Design Elements:**
- Plugin logo and version number
- Preset browser dropdown with categorization
- A/B comparison toggle
- Settings menu access
- Undo/Redo buttons

**User Interactions:**
- Click to open preset browser
- Drag presets for quick loading
- Right-click for additional preset options
- A/B button to compare two different states

**Mockup Description:**
The header spans the full width of the UI with the HarmonyScape logo on the left. The preset browser occupies the center with a dropdown showing the current preset name. A/B comparison, settings, and undo/redo buttons are positioned on the right side of the header.

### 2. Chord Input & Sequencer

**Design Elements:**
- Step sequencer with 4-16 steps (expandable)
- Chord buttons for quick selection
- Text input for chord symbols
- Piano keyboard for note input
- Playback controls (play/stop/loop)

**User Interactions:**
- Click step to select/edit chord
- Drag steps to copy/move chords
- Type chord names directly (e.g., "Cmaj7")
- Click piano keys to build custom chords
- MIDI learn functionality for hardware integration

**Mockup Description:**
The left panel displays a vertical step sequencer with 8 visible steps (scrollable). Each step has a chord display showing the current chord name and a miniature visualization. Below the sequencer is a piano keyboard input spanning two octaves with chord detection. Playback controls and sequencer settings are positioned at the bottom of this panel.

### 3. Voicing Visualization

**Design Elements:**
- Interactive staff notation display
- Voice distribution visualization
- Harmonic analysis labels
- Alternative view options (piano roll, chord diagram)

**User Interactions:**
- Click and drag notes to adjust voicings
- Toggle between notation views
- Hover for detailed note/function information
- Scale/zoom notation display

**Mockup Description:**
The central panel displays the current chord voicing on a grand staff with customizable clefs. Notes are color-coded by function (root, third, fifth, extensions). Below the staff is a horizontal representation of voice distribution across the frequency spectrum. Voice leading between consecutive chords is shown with subtle connecting lines. Alternative visualization buttons allow switching between staff, piano roll, and chord diagram views.

### 4. Spatial Configuration

**Design Elements:**
- Interactive 3D/2D spatial field representation
- Voice position indicators
- Movement pattern visualization
- Width/depth/height controls
- Voicing spread visualization

**User Interactions:**
- Drag voices to position in space
- Select predefined spatial patterns
- Configure automated movement patterns
- Toggle between 2D (stereo) and 3D spatial view

**Mockup Description:**
The right panel shows a primary spatial field visualization as a 3D cube (simplifying to 2D when in stereo mode). Colored spheres represent individual voices, with size corresponding to velocity and color matching the harmonic function. Below are controls for spatial width, movement patterns, and automation. A "Spatial Presets" section offers quick access to common spatial configurations.

### 5. Parameter Controls

**Design Elements:**
- Style selector (dropdown or tabs)
- Voice count and density controls
- Voice leading intensity slider
- Key/scale selection
- Spatial automation controls
- Advanced parameters (collapsible)

**User Interactions:**
- Slider controls for continuous parameters
- Dropdown selectors for categorical options
- Toggle buttons for binary settings
- Collapsible sections for advanced parameters

**Mockup Description:**
The bottom panel spans the full width of the UI, organized into logical parameter groups. The left side contains harmonic parameters (style, density, voice count), the center focuses on voice leading controls (strength, rules, transition smoothing), and the right side contains spatial automation settings. Each section has a header with a collapse/expand control for showing/hiding detailed parameters.

## Resizing Behavior

The UI is designed to be resizable with the following behaviors:

- Base resolution: 950 × 600 pixels
- Minimum size: 800 × 500 pixels
- Maximum size: Unlimited, with elements scaling proportionally
- High-DPI support with 200% scaling for retina/high-resolution displays

At different sizes:
- **Small**: Some advanced controls collapse into expandable areas
- **Medium**: All standard controls visible with comfortable spacing
- **Large**: Enhanced visualizations with more detailed information

## Color Scheme

The color palette is designed to provide clear visual separation while maintaining a cohesive aesthetic:

- **Background**: Dark gray (#212121) with subtle gradient
- **Primary UI elements**: Medium gray (#333333) with light borders
- **Text**: Light gray (#E0E0E0) for primary text, white for highlights
- **Accent**: Teal (#00B8D4) for primary interactions and highlights
- **Harmonic function colors**:
  - Root: Blue (#2979FF)
  - Third: Red (#FF5252) 
  - Fifth: Green (#00E676)
  - Seventh: Purple (#D500F9)
  - Extensions: Orange (#FFAB40)

## Typography

- Primary Font: "Roboto" for UI elements and controls
- Alternate Font: "Fira Code" for chord symbols and values
- Music Notation: SMuFL-compliant notation font

## Interactive Behaviors

### Real-time Feedback

- Parameter changes immediately reflected in visualizations
- Audio preview updates in real-time as parameters change
- Visual highlights indicate active/playing elements

### Contextual Help

- Hover tooltips with parameter explanations
- Info buttons for detailed descriptions of complex features
- Interactive tutorials accessible from help menu

### Error States

- Clear visual indicators for invalid settings
- Helpful suggestions for resolving conflicts
- Non-disruptive error messaging

## Accessibility Considerations

- High contrast mode option
- Keyboard navigation for all controls
- Screen reader support for key UI elements
- Configurable UI scaling independent of resolution

## UI States

### 1. Default State
The standard view with all essential controls visible and accessible.

### 2. Performance Mode
Minimizes non-essential elements to focus on core parameters during live use.

### 3. Advanced Mode
Exposes all parameters and detailed visualizations for deep editing.

### 4. Compact Mode
Reduces UI size while maintaining core functionality.

## Mobile Considerations (Phase 4)

While the primary focus is desktop VST3/AU integration, the UI design includes considerations for future mobile companion app development:

- Touch-friendly control sizing
- Simplified views for smaller screens
- Responsive layout adjustments
- Multi-touch gesture support

## UI Mockup Descriptions for Key Interaction Flows

### Chord Entry Flow

1. User clicks on a step in the sequencer
2. Step highlights and shows current chord (if any)
3. User can:
   - Type chord name directly (e.g., "Gmaj7")
   - Click chord buttons for common chords
   - Play notes on piano input to build custom chord
4. Voicing visualization updates in real-time
5. Spatial view updates to show new voice positions

### Voicing Adjustment Flow

1. User selects a chord in the sequence
2. Voicing visualization shows current note distribution
3. User adjusts voicing parameters (style, density, etc.)
4. Visualization updates to show new voicing
5. User can manually adjust individual notes by dragging
6. Voice leading visualization shows transitions to next chord

### Spatial Configuration Flow

1. User selects a spatial preset or starts with default
2. Interactive spatial view shows voice positions
3. User can:
   - Drag individual voices to custom positions
   - Adjust global width/depth/height controls
   - Select movement patterns for dynamic positioning
4. Changes reflected immediately in audio output
5. User can save custom spatial configuration as preset

### Preset Management Flow

1. User creates desired chord progression and configuration
2. Clicks "Save" in preset browser
3. Dialog appears for naming and categorizing preset
4. User can add tags and description
5. Preset appears in browser under specified category
6. User can later recall, update, or share preset

## Implementation Notes

### MVP UI Implementation (Phase 1)
The initial UI will focus on core functionality:
- Basic chord sequencer (8 steps)
- Simple staff notation view
- Basic stereo positioning controls
- Essential parameter controls
- Streamlined preset system

### Future UI Enhancements (Phase 2+)
- Enhanced visualizations with note connectivity
- 3D spatial view for advanced formats
- Expanded keyboard input options
- Advanced pattern editing
- Custom color schemes
- Expanded preset browser with tagging and search

This UI design balances intuitive operation with powerful features, providing both newcomers and advanced users with an engaging, efficient interface for harmonic exploration. 