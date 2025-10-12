<h1><strong>Circulate</strong></h1>
<p>Circulate is an audio effect which uses banks of allpass filters to create phase smearing effects. The allpass filters, while leaving the frequency spectrum untouched, cause selective phase offsets around the chosen center frequency. These offsets present themselves as hollow and metallic short reverberation-like effects, useful for adding a dynamic metallic character to sounds. The effect is most pronounced on transient material, such as bass stabs, drum hits and plucks.</p>
<p>&nbsp;</p>
<img width="304" height="160" alt="XXXXXXXX_snapshot_2 0x" src="https://github.com/user-attachments/assets/c7016aa4-c912-46bc-a521-47cd146c17d5" />
<h3>Download</h3>
https://github.com/GullDSP/Circulate/releases
<h3>Demo</h3>
www.youtube.com/watch?v=tcsrC33vn1s&t=1s
<h3>Features</h3>
<ul>
<li>Sample accurate parameters and automation. Suitable for fast and complex automation and control via DAW modulators (for example Ableton LFOs)</li>
<li>Up to 64 stages of allpass filtering with variable Q (Resonance).</li>
<li>Center frequency can be controlled in Hz, or by selecting a MIDI note as the center.</li>
<li>Optional positive or negative feedback through the filter bank to create spectral effects. Similar to a steep phaser</li>
</ul>
<h3>Parameters</h3>
<ul>
<li><strong>Center</strong> - Center frequency of the allpass filter(s) in Hz</li>
<li><strong>Pitch</strong> - Sets the center frequency through MIDI note</li>
<li><strong>Det</strong> - Allows smooth offset (+/- 1 Octave) of the center frequency from the selected MIDI note</li>
<li><strong>Focus</strong> - The Q factor, or 'Resonance' of the allpass filters. Lower Q values spread the phase smearing over a wider range, higher values focus the smearing tighter around the center.</li>
<li><strong>Depth</strong> - Sets the number of allpass filters in the filter bank, up to a maximum of 64. This parameter is not sample accurate (due to discrete nature)</li>
<li><strong>Feed</strong> - Feedback is introduced into the filter bank, this *will* lead to frequency spectrum changes, through cancelling or boosting affected frequencies.</li>
<img width="304" height="160" alt="XXXXXXXX_snapshot_2 0x" src="https://github.com/user-attachments/assets/c2d43bf0-cc44-4c3a-83eb-300a56d641fe" />
<h3>Version</h3>
 - v1.0.5 fixes parameter issues with FLStudio 
<h3>Acknowledgements</h3>
<ul>
<li>This project is built using the Steinberg VST 3 SDK(https://www.steinberg.net/developers/).</li>
<li>VST is a trademark of Steinberg Media Technologies GmbH.</li>
</ul>
