<h1><strong>Circulate</strong></h1>
<p>Circulate is an allpass filter-based phase dispersion effect. The allpass filters, while leaving the frequency spectrum untouched, cause selective phase offsets around the chosen center frequency. These offsets present themselves as hollow and metallic short reverberation-like effects, useful for adding a dynamic metallic character to sounds. The effect is most pronounced on transient material, such as bass stabs, drum hits and plucks.</p>
<img width="573" height="296" alt="image" src="https://github.com/user-attachments/assets/f8ac30c6-3629-42cb-884a-67057c998b77" />

<h3>Download</h3>
https://github.com/GullDSP/Circulate/releases
<h3>Demo</h3>
https://www.youtube.com/watch?v=rluT0xgxPuI


www.youtube.com/watch?v=tcsrC33vn1s&t=1s
<h3>Features</h3>
<ul>
<li>Sample accurate parameters and automation. Suitable for fast and complex automation and control via DAW modulators (for example Ableton LFOs)</li>
<li>Up to 64 stages of allpass dipsersion with variable Q (Resonance).</li>
<li>Center frequency can be controlled in Hz, or by selecting a MIDI note as the center.</li>
<li>Optional positive or negative feedback through the filter bank to create spectral effects. Similar to a steep phaser</li>
</ul>
<h3>Parameters</h3>
<ul>
<li><strong>Center</strong> - Center frequency of the allpass filter(s) in Hz. Click the display to type in a precise value.</li>
<li><strong>Pitch</strong> - Sets the center frequency through MIDI note</li>
<li><strong>Det</strong> - Allows smooth offset (+/- 1 Octave) of the center frequency from the selected MIDI note</li>
<li><strong>Focus</strong> - The Q factor, or 'Resonance' of the allpass filters. Lower Q values spread the phase smearing over a wider range, higher values focus the smearing tighter around the center.</li>
<li><strong>Depth</strong> - Sets the number of allpass filters in the filter bank, up to a maximum of 64.</li>
<li><strong>Feed</strong> - Feedback is introduced into the filter bank, this *will* lead to frequency spectrum changes, through cancelling or boosting affected frequencies.</li>

<h3>Version 2</h3>
<li>Resizable UI (right click - UI Zoom).</li>
<li>CPU use improvements.</li>
<li>Fixed Ableton UI bug when switching from Hz to semitone control.</li>
<li>Fixed strange bounce to audio behaviour in some DAWs.</li>
<li>Can manually enter a frequency (in Hz).</li>

<h3>Acknowledgements</h3>
<ul>
<li>This project is built using the Steinberg VST 3 SDK(https://www.steinberg.net/developers/).</li>
<li>VST is a trademark of Steinberg Media Technologies GmbH.</li>
<img width="200" height="187" alt="XXXXXXXX_snapshot_2 0x" src="https://steinbergmedia.github.io/vst3_dev_portal/resources/licensing_3.png" />

</ul>
