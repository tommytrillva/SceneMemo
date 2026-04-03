#!/usr/bin/env python3
"""Generate 260 factory presets for SceneMemo across all Tonal Palette categories."""

import json
import os
import random
import math

random.seed(42)  # reproducible

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'resources', 'presets')

CATEGORIES = {
    0: "Golden Hour",
    1: "Night Drive",
    2: "Rooftop Rain",
    3: "City Fog",
    4: "Desert Heat",
    5: "Neon Alley",
    6: "Ocean Floor",
    7: "Midnight Studio",
    8: "First Light",
    9: "Street Level",
    10: "Memory Lane",
    11: "The Cosmos",
}

# Parameter XML template — JUCE APVTS ValueTree format
def make_param_xml(params: dict) -> str:
    lines = ['<?xml version="1.0" encoding="UTF-8"?>', '<SceneMemoState>']
    for pid, val in params.items():
        lines.append(f'  <PARAM id="{pid}" value="{val:.6f}"/>')
    lines.append('</SceneMemoState>')
    return '\n'.join(lines)

def make_preset(name, author, category, tags, description, params):
    return {
        "metadata": {
            "name": name,
            "author": author,
            "category": category,
            "tags": tags,
            "description": description,
            "sourceAudioPath": "",
            "locationTag": "",
            "dateCreated": "2026-04-03T00:00:00Z",
            "dateModified": "2026-04-03T00:00:00Z",
            "isFavorite": False,
        },
        "parameters": make_param_xml(params),
        "version": "1.0"
    }

def base_params():
    """Return default parameter dict."""
    p = {}
    p['bypass'] = 0.0
    p['master_vol'] = 0.8
    p['drywet'] = 1.0
    # Osc 1 defaults
    for i in range(1, 5):
        prefix = f'osc{i}'
        lvl = 0.8 if i == 1 else 0.0
        if i == 1:
            p['osc1_level'] = lvl
            p['osc1_tune'] = 0.0
            p['osc1_fine'] = 0.0
            p['osc1_waveform'] = 0.0
        else:
            p[f'{prefix}_level'] = lvl
            p[f'{prefix}_tune'] = 0.0
            p[f'{prefix}_fine'] = 0.0
            p[f'{prefix}_waveform'] = 0.0
        p[f'{prefix}_type'] = 0.0
        p[f'{prefix}_pan'] = 0.0
        p[f'{prefix}_octave'] = 0.0
        p[f'{prefix}_wt_pos'] = 0.0
        p[f'{prefix}_unison'] = 1.0
        p[f'{prefix}_unison_spread'] = 0.5
    # Filters
    for i in range(1, 3):
        p[f'filt{i}_type'] = 1.0  # LP24
        p[f'filt{i}_cutoff'] = 20000.0
        p[f'filt{i}_reso'] = 0.0
        p[f'filt{i}_drive'] = 0.0
        p[f'filt{i}_keytrack'] = 0.0
        p[f'filt{i}_env_amt'] = 0.0
    p['filt_routing'] = 0.0
    # Envelopes
    for i in range(1, 5):
        prefix = f'env{i}'
        sus = 0.7 if i == 1 else 0.0
        if i == 1:
            p['env1_attack'] = 0.01
            p['env1_hold'] = 0.0
            p['env1_decay'] = 0.3
            p['env1_sustain'] = sus
            p['env1_release'] = 0.3
        else:
            p[f'{prefix}_attack'] = 0.01
            p[f'{prefix}_hold'] = 0.0
            p[f'{prefix}_decay'] = 0.3
            p[f'{prefix}_sustain'] = sus
            p[f'{prefix}_release'] = 0.3
        p[f'{prefix}_attack_curve'] = 0.0
        p[f'{prefix}_decay_curve'] = 0.0
        p[f'{prefix}_release_curve'] = 0.0
    # LFOs
    for i in range(1, 5):
        prefix = f'lfo{i}'
        p[f'{prefix}_shape'] = 0.0
        p[f'{prefix}_rate'] = 1.0
        p[f'{prefix}_sync'] = 0.0
        p[f'{prefix}_sync_rate'] = 9.0
        p[f'{prefix}_phase'] = 0.0
        p[f'{prefix}_fadein'] = 0.0
        p[f'{prefix}_retrigger'] = 0.0
        p[f'{prefix}_humanize'] = 0.0
    # Mod matrix
    for i in range(1, 17):
        p[f'mod{i}_source'] = 0.0
        p[f'mod{i}_dest'] = 0.0
        p[f'mod{i}_depth'] = 0.0
    # Macros
    for i in range(1, 5):
        p[f'macro{i}'] = 0.0
    return p

def r(lo, hi):
    return random.uniform(lo, hi)

def ri(lo, hi):
    return float(random.randint(lo, hi))

# ============================================================================
# Preset generators per category
# ============================================================================

def golden_hour_presets():
    """Warm, open, nostalgic, sunset — amber/gold"""
    presets = []
    names = [
        "Amber Glow", "Sunset Boulevard", "Warm Nostalgia", "Honey Light",
        "Dusk Horizon", "Golden Drift", "Soft Focus Sunset", "Analog Warmth",
        "Summer Haze", "Porch Swing", "Fading Daylight", "Sepia Tone",
        "Late Afternoon", "Vintage Gold", "Sundowner", "Copper Sky",
        "Firefly Evening", "Mellow Rays", "Harvest Moon Rise", "Cathedral Light",
        "Amber Cascade", "Gilded Memory",
    ]
    for name in names:
        p = base_params()
        # Warm saw/triangle pads
        p['osc1_waveform'] = ri(1, 3)  # saw/square/triangle
        p['osc1_level'] = r(0.5, 0.8)
        p['osc1_unison'] = ri(2, 4)
        p['osc1_unison_spread'] = r(0.3, 0.6)
        p['env1_attack'] = r(0.3, 1.5)
        p['env1_decay'] = r(0.5, 2.0)
        p['env1_sustain'] = r(0.5, 0.8)
        p['env1_release'] = r(0.5, 2.0)
        # Warm filter
        p['filt1_cutoff'] = r(2000, 8000)
        p['filt1_reso'] = r(0.05, 0.25)
        # Second osc: sub for warmth
        p['osc2_type'] = 3.0  # Sub
        p['osc2_level'] = r(0.2, 0.5)
        p['osc2_waveform'] = 0.0  # sine sub
        # Tape character
        tags = ["warm", "pad", "sunset", "nostalgic"]
        presets.append(make_preset(name, "SceneMemo", 0, tags,
            f"Warm golden atmosphere — {name.lower()}", p))
    return presets

def night_drive_presets():
    """Cool, reflective, urban motion — deep blue"""
    presets = []
    names = [
        "Midnight Highway", "City Lights Blur", "Neon Reflections", "Cool Cruise",
        "Dashboard Glow", "Empty Streets", "Rain on Windshield", "Tunnel Vision",
        "Headlight Trails", "2AM Drive", "Urban Drift", "Asphalt Dreams",
        "Highway Pulse", "Chrome Reflection", "Speed of Dark", "Vapor Trail",
        "Blue Hour Drive", "Late Night Radio", "Interstate Haze", "Backroad Echo",
        "Midnight Chrome", "Electric Commute",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 1.0  # saw
        p['osc1_level'] = r(0.4, 0.7)
        p['osc1_unison'] = ri(3, 6)
        p['osc1_unison_spread'] = r(0.4, 0.8)
        p['env1_attack'] = r(0.1, 0.8)
        p['env1_sustain'] = r(0.6, 0.9)
        p['env1_release'] = r(0.8, 3.0)
        # Filtered — cooler tone
        p['filt1_cutoff'] = r(1500, 5000)
        p['filt1_reso'] = r(0.1, 0.4)
        p['filt1_env_amt'] = r(0.1, 0.4)
        # LFO on filter for movement
        p['lfo1_rate'] = r(0.1, 0.5)
        p['mod1_source'] = 0.0  # LFO1
        p['mod1_dest'] = 16.0  # Filt1Cutoff
        p['mod1_depth'] = r(0.1, 0.35)
        # Osc2: noise for texture
        p['osc3_type'] = 2.0  # Noise
        p['osc3_level'] = r(0.05, 0.15)
        p['osc3_waveform'] = 3.0  # Air
        tags = ["cool", "driving", "urban", "reflective"]
        presets.append(make_preset(name, "SceneMemo", 1, tags,
            f"Cool urban motion — {name.lower()}", p))
    return presets

def rooftop_rain_presets():
    """Melancholy, introspective, gentle — slate gray"""
    presets = []
    names = [
        "Quiet Rainfall", "Gray Sky Meditation", "Puddle Reflections", "Gentle Drizzle",
        "Storm Window", "Misty Rooftop", "Rain Delay", "Overcast", "Melancholy Drops",
        "Tin Roof Lullaby", "November Fog", "Weeping Sky", "Downpour Distant",
        "Damp Air", "Cloud Cover", "Soft Thunder", "Rain on Glass", "Patter",
        "Wet Pavement Glow", "Gray Morning", "Monsoon Whisper", "Drizzle Drift",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 3.0  # triangle
        p['osc1_level'] = r(0.3, 0.6)
        p['osc1_unison'] = ri(2, 3)
        p['osc1_unison_spread'] = r(0.2, 0.5)
        p['env1_attack'] = r(0.5, 2.0)
        p['env1_decay'] = r(1.0, 3.0)
        p['env1_sustain'] = r(0.3, 0.6)
        p['env1_release'] = r(1.0, 4.0)
        p['filt1_cutoff'] = r(1000, 4000)
        p['filt1_reso'] = r(0.0, 0.15)
        # Noise layer for rain texture
        p['osc3_type'] = 2.0
        p['osc3_level'] = r(0.1, 0.25)
        p['osc3_waveform'] = 1.0  # Pink noise
        # Slow LFO modulating level
        p['lfo1_rate'] = r(0.05, 0.2)
        p['mod1_source'] = 0.0
        p['mod1_dest'] = 0.0  # Osc1Level
        p['mod1_depth'] = r(0.05, 0.15)
        tags = ["rain", "melancholy", "introspective", "gentle"]
        presets.append(make_preset(name, "SceneMemo", 2, tags,
            f"Gentle melancholy — {name.lower()}", p))
    return presets

def city_fog_presets():
    """Mysterious, dense, distant — soft white"""
    presets = []
    names = [
        "Dense Mist", "Foghorn Echo", "Lost Signal", "White Veil",
        "Obscured", "Ghost Frequency", "Pale Distance", "Vapor Blanket",
        "Haze Machine", "Clouded Vision", "Zero Visibility", "Phantom Tone",
        "Gray Matter", "Smog Layer", "Diffuse Light", "Invisible City",
        "Mist Walker", "Fade to White", "Cloud Nine", "Shrouded",
        "Fog Drift", "Ethereal Mist",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 0.0  # sine
        p['osc1_level'] = r(0.3, 0.5)
        p['osc1_unison'] = ri(4, 6)
        p['osc1_unison_spread'] = r(0.5, 0.9)
        p['osc1_fine'] = r(-10, 10)
        p['env1_attack'] = r(1.0, 3.0)
        p['env1_sustain'] = r(0.4, 0.7)
        p['env1_release'] = r(2.0, 5.0)
        p['filt1_cutoff'] = r(500, 3000)
        p['filt1_reso'] = r(0.0, 0.1)
        # Heavy noise layer
        p['osc2_type'] = 2.0  # Noise
        p['osc2_level'] = r(0.15, 0.35)
        p['osc2_waveform'] = 1.0  # Pink
        p['osc3_type'] = 2.0
        p['osc3_level'] = r(0.05, 0.15)
        p['osc3_waveform'] = 3.0  # Air
        tags = ["fog", "mysterious", "dense", "ambient"]
        presets.append(make_preset(name, "SceneMemo", 3, tags,
            f"Dense mysterious atmosphere — {name.lower()}", p))
    return presets

def desert_heat_presets():
    """Dry, wide, shimmering, vast — burnt orange"""
    presets = []
    names = [
        "Mirage Shimmer", "Sand Dune Drift", "Heat Haze", "Dry Canyon",
        "Sahara Wind", "Scorched Earth", "Oasis Distant", "Red Rock",
        "Desert Sun", "Dust Devil", "Arid Expanse", "Tumbleweed",
        "Nomad Path", "Mesa Echo", "Cactus Shadow", "Sunbaked",
        "Burning Plains", "Wide Open", "Salt Flat", "Rattlesnake Hum",
        "Horizon Glow", "Sandstorm Veil",
    ]
    for name in names:
        p = base_params()
        p['osc1_type'] = 1.0  # VA
        p['osc1_waveform'] = 1.0  # saw
        p['osc1_level'] = r(0.4, 0.7)
        p['osc1_unison'] = ri(2, 4)
        p['osc1_unison_spread'] = r(0.6, 0.9)
        p['env1_attack'] = r(0.2, 1.0)
        p['env1_sustain'] = r(0.5, 0.8)
        p['env1_release'] = r(1.0, 3.0)
        # Wide open filter
        p['filt1_cutoff'] = r(4000, 12000)
        p['filt1_reso'] = r(0.1, 0.3)
        # Shimmer from high-frequency noise
        p['osc3_type'] = 2.0
        p['osc3_level'] = r(0.05, 0.2)
        p['osc3_waveform'] = 3.0  # Air
        # LFO for shimmer
        p['lfo1_rate'] = r(2.0, 8.0)
        p['mod1_source'] = 0.0
        p['mod1_dest'] = 8.0  # Osc3Level (approximate)
        p['mod1_depth'] = r(0.1, 0.3)
        tags = ["desert", "wide", "shimmering", "vast"]
        presets.append(make_preset(name, "SceneMemo", 4, tags,
            f"Vast desert expanse — {name.lower()}", p))
    return presets

def neon_alley_presets():
    """Electric, gritty, vibrant, late-night — hot pink/magenta"""
    presets = []
    names = [
        "Neon Buzz", "Electric Alley", "Pink Static", "Gritty Pulse",
        "Late Night Glow", "Voltage Drop", "Cyberpunk Haze", "Dirty Synth",
        "Hot Wire", "Bass Alley", "Flicker", "Magenta Drive",
        "Circuit Bent", "Lo-Fi Neon", "Distorted City", "Gutter Glow",
        "Broken Sign", "Synth Trash", "Night Market", "Buzzing Light",
        "Raw Current", "Analog Dirt",
    ]
    for name in names:
        p = base_params()
        p['osc1_type'] = 1.0  # VA
        p['osc1_waveform'] = 2.0  # square
        p['osc1_level'] = r(0.5, 0.8)
        p['osc1_unison'] = ri(3, 6)
        p['osc1_unison_spread'] = r(0.3, 0.7)
        p['env1_attack'] = r(0.01, 0.3)
        p['env1_sustain'] = r(0.6, 0.9)
        p['env1_release'] = r(0.3, 1.0)
        # Resonant filter
        p['filt1_cutoff'] = r(800, 4000)
        p['filt1_reso'] = r(0.3, 0.7)
        p['filt1_drive'] = r(0.2, 0.6)
        p['filt1_env_amt'] = r(0.2, 0.5)
        # Second osc: detuned saw
        p['osc2_type'] = 1.0  # VA
        p['osc2_waveform'] = 1.0  # saw
        p['osc2_level'] = r(0.3, 0.6)
        p['osc2_fine'] = r(-15, 15)
        # Sub for weight
        p['osc4_type'] = 3.0
        p['osc4_level'] = r(0.2, 0.4)
        tags = ["neon", "gritty", "electric", "vibrant"]
        presets.append(make_preset(name, "SceneMemo", 5, tags,
            f"Electric gritty atmosphere — {name.lower()}", p))
    return presets

def ocean_floor_presets():
    """Deep, submerged, slow, pressure — teal"""
    presets = []
    names = [
        "Abyssal Drone", "Pressure Zone", "Deep Current", "Sonar Ping",
        "Bioluminescence", "Trencher", "Whale Song", "Subaquatic",
        "Coral Hum", "Dark Water", "Kelp Forest", "Plankton Drift",
        "Submarine", "Bathysphere", "Tidal Breath", "Blue Void",
        "Undercurrent", "Sea Floor", "Bubble Rise", "Depth Charge",
        "Marine Layer", "Aquatic Pulse",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 0.0  # sine
        p['osc1_level'] = r(0.4, 0.6)
        p['osc1_octave'] = -1.0
        p['osc1_unison'] = ri(2, 4)
        p['osc1_unison_spread'] = r(0.2, 0.5)
        p['env1_attack'] = r(1.0, 3.0)
        p['env1_decay'] = r(1.0, 3.0)
        p['env1_sustain'] = r(0.5, 0.8)
        p['env1_release'] = r(2.0, 5.0)
        # Very dark filter
        p['filt1_cutoff'] = r(300, 1500)
        p['filt1_reso'] = r(0.1, 0.3)
        # Sub for deep pressure
        p['osc2_type'] = 3.0  # Sub
        p['osc2_level'] = r(0.3, 0.6)
        # Slow movement
        p['lfo1_rate'] = r(0.02, 0.1)
        p['mod1_source'] = 0.0
        p['mod1_dest'] = 16.0  # Filt1Cutoff
        p['mod1_depth'] = r(0.05, 0.2)
        tags = ["deep", "submerged", "slow", "oceanic"]
        presets.append(make_preset(name, "SceneMemo", 6, tags,
            f"Deep subaquatic tone — {name.lower()}", p))
    return presets

def midnight_studio_presets():
    """Intimate, close, focused, late-session — dark purple"""
    presets = []
    names = [
        "3AM Session", "Studio Haze", "Late Night Beat", "Intimate Keys",
        "Purple Haze Pad", "Close Mic", "Session Smoke", "Dim Light",
        "Focus Mode", "Headphone Space", "After Hours", "Quiet Intensity",
        "Monitor Glow", "Worn Faders", "Console Warmth", "Tape Hiss",
        "Analog Midnight", "Deep Focus", "Solo Session", "Night Owl",
        "Last Take", "Red Light On",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = ri(1, 3)
        p['osc1_level'] = r(0.5, 0.7)
        p['osc1_unison'] = ri(2, 4)
        p['osc1_unison_spread'] = r(0.2, 0.4)
        p['env1_attack'] = r(0.05, 0.5)
        p['env1_sustain'] = r(0.6, 0.85)
        p['env1_release'] = r(0.5, 1.5)
        # Medium filter for warmth
        p['filt1_cutoff'] = r(2000, 6000)
        p['filt1_reso'] = r(0.05, 0.2)
        p['filt1_drive'] = r(0.05, 0.2)
        # Second osc for depth
        p['osc2_level'] = r(0.2, 0.4)
        p['osc2_waveform'] = 0.0  # sine
        p['osc2_octave'] = -1.0
        tags = ["intimate", "studio", "focused", "late-night"]
        presets.append(make_preset(name, "SceneMemo", 7, tags,
            f"Late-night studio atmosphere — {name.lower()}", p))
    return presets

def first_light_presets():
    """Hopeful, emerging, delicate, new — pale yellow"""
    presets = []
    names = [
        "Dawn Break", "Morning Dew", "New Day", "Gentle Awakening",
        "Pale Sunrise", "Bird Song Pad", "Fresh Air", "Opening Eyes",
        "Horizon Line", "Soft Beginning", "Early Glow", "Spring Bloom",
        "First Breath", "Clear Sky", "Dewy Meadow", "Morning Light",
        "Renewal", "Tender Start", "Pastel Sky", "Crystalline",
        "Daybreak Bells", "New Chapter",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 3.0  # triangle — delicate
        p['osc1_level'] = r(0.3, 0.5)
        p['osc1_unison'] = ri(2, 3)
        p['osc1_unison_spread'] = r(0.3, 0.6)
        p['env1_attack'] = r(0.5, 2.0)
        p['env1_decay'] = r(0.5, 1.5)
        p['env1_sustain'] = r(0.4, 0.7)
        p['env1_release'] = r(1.5, 4.0)
        # Bright, open filter
        p['filt1_cutoff'] = r(6000, 15000)
        p['filt1_reso'] = r(0.0, 0.1)
        # Shimmer osc
        p['osc3_type'] = 2.0  # Noise
        p['osc3_level'] = r(0.03, 0.1)
        p['osc3_waveform'] = 3.0  # Air
        # High octave sine for sparkle
        p['osc4_level'] = r(0.1, 0.25)
        p['osc4_waveform'] = 0.0
        p['osc4_octave'] = 1.0
        tags = ["hopeful", "delicate", "morning", "bright"]
        presets.append(make_preset(name, "SceneMemo", 8, tags,
            f"Delicate hopeful tone — {name.lower()}", p))
    return presets

def street_level_presets():
    """Raw, textured, human, real — concrete gray"""
    presets = []
    names = [
        "Concrete Jungle", "Sidewalk Beat", "Urban Texture", "Raw Signal",
        "Grit and Grain", "Street Corner", "Broken Speaker", "City Pulse",
        "Subway Rumble", "Crowd Murmur", "Traffic Flow", "Construction Pad",
        "Asphalt Layer", "Real Talk", "Worn Boots", "Fire Escape",
        "Brick Wall", "Stoop Session", "Block Party", "Hydrant Hiss",
        "Rough Cut", "Manhole Steam",
    ]
    for name in names:
        p = base_params()
        p['osc1_type'] = 1.0  # VA
        p['osc1_waveform'] = 1.0  # saw
        p['osc1_level'] = r(0.5, 0.8)
        p['osc1_unison'] = ri(1, 3)
        p['env1_attack'] = r(0.01, 0.2)
        p['env1_sustain'] = r(0.5, 0.8)
        p['env1_release'] = r(0.2, 0.8)
        # Gritty filter
        p['filt1_cutoff'] = r(1000, 5000)
        p['filt1_reso'] = r(0.15, 0.5)
        p['filt1_drive'] = r(0.3, 0.7)
        # Noise texture
        p['osc2_type'] = 2.0
        p['osc2_level'] = r(0.1, 0.3)
        p['osc2_waveform'] = 0.0  # White noise
        # Sub
        p['osc3_type'] = 3.0
        p['osc3_level'] = r(0.2, 0.4)
        tags = ["raw", "textured", "urban", "gritty"]
        presets.append(make_preset(name, "SceneMemo", 9, tags,
            f"Raw urban texture — {name.lower()}", p))
    return presets

def memory_lane_presets():
    """Nostalgic, personal, warped, emotional — sepia"""
    presets = []
    names = [
        "Faded Photograph", "Old Cassette", "Childhood Room", "Distant Memory",
        "VHS Warmth", "Time Warp", "Forgotten Song", "Dusty Keys",
        "Grandma's Radio", "Heirloom", "Diary Pages", "Music Box",
        "Vinyl Crackle Pad", "Aged Film", "Remember When", "Polaroid",
        "Worn Record", "Home Movie", "Locket", "Attic Find",
        "Sepia Dream", "Lost and Found",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = ri(1, 3)
        p['osc1_level'] = r(0.4, 0.7)
        p['osc1_unison'] = ri(2, 4)
        p['osc1_unison_spread'] = r(0.3, 0.5)
        p['osc1_fine'] = r(-8, 8)  # slight detune for "warped"
        p['env1_attack'] = r(0.3, 1.5)
        p['env1_sustain'] = r(0.4, 0.7)
        p['env1_release'] = r(1.0, 3.0)
        # Warm, rolled off
        p['filt1_cutoff'] = r(1500, 5000)
        p['filt1_reso'] = r(0.0, 0.15)
        # Noise for vinyl texture
        p['osc3_type'] = 2.0
        p['osc3_level'] = r(0.05, 0.15)
        p['osc3_waveform'] = 1.0  # Pink
        # LFO for wow-like movement
        p['lfo1_rate'] = r(0.1, 0.5)
        p['mod1_source'] = 0.0
        p['mod1_dest'] = 3.0  # Osc1Fine
        p['mod1_depth'] = r(0.05, 0.15)
        tags = ["nostalgic", "warped", "emotional", "vintage"]
        presets.append(make_preset(name, "SceneMemo", 10, tags,
            f"Nostalgic emotional texture — {name.lower()}", p))
    return presets

def the_cosmos_presets():
    """Vast, ethereal, weightless, sci-fi — black/silver"""
    presets = []
    names = [
        "Event Horizon", "Stellar Drift", "Zero Gravity", "Nebula Core",
        "Dark Matter", "Cosmic Dust", "Interstellar", "Void Walker",
        "Supernova Swell", "Orbital Decay", "Star Birth", "Galaxy Spin",
        "Quantum Field", "Wormhole", "Singularity", "Astral Plane",
        "Cosmic Hum", "Deep Space", "Solar Wind", "Light Year",
        "Constellation", "Infinite Expanse",
    ]
    for name in names:
        p = base_params()
        p['osc1_waveform'] = 0.0  # sine
        p['osc1_level'] = r(0.3, 0.6)
        p['osc1_unison'] = ri(4, 8)
        p['osc1_unison_spread'] = r(0.6, 1.0)
        p['osc1_fine'] = r(-5, 5)
        p['env1_attack'] = r(2.0, 5.0)
        p['env1_decay'] = r(1.0, 3.0)
        p['env1_sustain'] = r(0.5, 0.8)
        p['env1_release'] = r(3.0, 5.0)
        p['env1_attack_curve'] = r(-0.5, 0.5)
        # Open but slightly dark
        p['filt1_cutoff'] = r(2000, 10000)
        p['filt1_reso'] = r(0.0, 0.2)
        # Second osc: higher octave sine
        p['osc2_level'] = r(0.15, 0.35)
        p['osc2_waveform'] = 0.0
        p['osc2_octave'] = 1.0
        p['osc2_unison'] = ri(2, 4)
        p['osc2_unison_spread'] = r(0.5, 0.8)
        # Noise for space texture
        p['osc3_type'] = 2.0
        p['osc3_level'] = r(0.05, 0.15)
        p['osc3_waveform'] = 3.0  # Air
        # Slow LFO for movement
        p['lfo1_rate'] = r(0.01, 0.1)
        p['lfo1_shape'] = 0.0  # sine
        p['mod1_source'] = 0.0
        p['mod1_dest'] = 16.0  # Filt1Cutoff
        p['mod1_depth'] = r(0.1, 0.3)
        tags = ["cosmic", "vast", "ethereal", "sci-fi"]
        presets.append(make_preset(name, "SceneMemo", 11, tags,
            f"Vast cosmic atmosphere — {name.lower()}", p))
    return presets

# ============================================================================
# Main generation
# ============================================================================

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    all_presets = []
    all_presets.extend(golden_hour_presets())
    all_presets.extend(night_drive_presets())
    all_presets.extend(rooftop_rain_presets())
    all_presets.extend(city_fog_presets())
    all_presets.extend(desert_heat_presets())
    all_presets.extend(neon_alley_presets())
    all_presets.extend(ocean_floor_presets())
    all_presets.extend(midnight_studio_presets())
    all_presets.extend(first_light_presets())
    all_presets.extend(street_level_presets())
    all_presets.extend(memory_lane_presets())
    all_presets.extend(the_cosmos_presets())

    print(f"Generated {len(all_presets)} presets across {len(CATEGORIES)} categories")

    # Write each preset as a .smemo file
    for preset in all_presets:
        name = preset['metadata']['name']
        safe_name = "".join(c if c.isalnum() or c in (' ', '-', '_') else '' for c in name)
        safe_name = safe_name.strip().replace(' ', '_')
        filepath = os.path.join(OUTPUT_DIR, f"{safe_name}.smemo")

        with open(filepath, 'w') as f:
            json.dump(preset, f, indent=2)

    print(f"Written {len(all_presets)} .smemo files to {OUTPUT_DIR}")

    # Print category counts
    from collections import Counter
    cats = Counter(p['metadata']['category'] for p in all_presets)
    for cat_id, count in sorted(cats.items()):
        print(f"  {CATEGORIES[cat_id]}: {count}")

if __name__ == '__main__':
    main()
