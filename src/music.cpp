/* 
 * FFNx - Complete OpenGL replacement of the Direct3D renderer used in 
 * the original ports of Final Fantasy VII and Final Fantasy VIII for the PC.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * music.c - replacements routines for music player
 */

#include <windows.h>

#include "music.h"
#include "patch.h"
#include "ff7music/music.h"
#include "winamp/music.h"

void music_init()
{
	// Add Global Focus flag to DirectSound Secondary Buffers
	patch_code_byte(common_externals.directsound_buffer_flags_1, 0x80); // DSBCAPS_GLOBALFOCUS & 0x0000FF00

	if (use_external_music > FFNX_MUSIC_NONE && use_external_music <= FFNX_MUSIC_FF7MUSIC)
	{
		if (ff8) {
			replace_function(common_externals.play_midi, ff8_play_midi);
			replace_function(common_externals.pause_midi, pause_midi);
			replace_function(common_externals.restart_midi, restart_midi);
			replace_function(common_externals.stop_midi, stop_midi);
			replace_function(common_externals.midi_status, midi_status);
		}
		else {
			replace_function(common_externals.midi_init, midi_init);
			replace_function(common_externals.play_midi, play_midi);
			replace_function(common_externals.cross_fade_midi, cross_fade_midi);
			replace_function(common_externals.pause_midi, pause_midi);
			replace_function(common_externals.restart_midi, restart_midi);
			replace_function(common_externals.stop_midi, stop_midi);
			replace_function(common_externals.midi_status, midi_status);
			replace_function(common_externals.set_master_midi_volume, set_master_midi_volume);
			replace_function(common_externals.set_midi_volume, set_midi_volume);
			replace_function(common_externals.set_midi_volume_trans, set_midi_volume_trans);
			replace_function(common_externals.set_midi_tempo, set_midi_tempo);
		}

		switch (use_external_music)
		{
		case FFNX_MUSIC_WINAMP:
			winamp_music_init();
			break;
		case FFNX_MUSIC_FF7MUSIC:
			break;
		}
	}
	else {
		error("Unknown use_external_music value\n");
	}
}

uint midi_init(uint unknown)
{
	// without this there will be no volume control for music in the config menu
	*ff7_externals.midi_volume_control = true;

	// enable fade function
	*ff7_externals.midi_initialized = true;

	return true;
}

uint ff8_play_midi(uint midi, uint volume, uint u1, uint u2)
{
	// midi_name format: {num}{type}-{name}.sgt
	char* midi_name = common_externals.get_midi_name(midi),
		* max_midi_name;
	char truncated_midi_name[32];

	info("FF8 midi play %i %i %i %i %s\n", midi, volume, u1, u2, midi_name);

	midi_name = strchr(midi_name, '-');
	
	if (nullptr != midi_name) {
		midi_name += 1; // Remove "-"
		max_midi_name = strchr(midi_name, '.');

		if (nullptr != max_midi_name) {
			size_t len = max_midi_name - midi_name;

			if (len < 32) {
				memcpy(truncated_midi_name, midi_name, len);
				truncated_midi_name[len] = '\0';

				switch (use_external_music)
				{
				case FFNX_MUSIC_VGMSTREAM:
					vgm_play_music(truncated_midi_name, midi);
					//vgm_set_music_volume(volume);
					break;
				case FFNX_MUSIC_FF7MUSIC:
					ff7music_play_music(truncated_midi_name, midi);
					//ff7music_set_music_volume(volume);
					break;
				case FFNX_MUSIC_WINAMP:
					winamp_play_music(truncated_midi_name, midi);
					//winamp_set_music_volume(volume);
					break;
				}
			}
		}
	}

	// TODO: set dword_1CD21A4 to 1 and current_midi_id (1CD21A8)

	return 1; // Success
}

void play_midi(uint midi)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_play_music(common_externals.get_midi_name(midi), midi);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_play_music(common_externals.get_midi_name(midi), midi);
		break;
	}
}

void cross_fade_midi(uint midi, uint time)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_cross_fade_music(common_externals.get_midi_name(midi), midi, time);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_cross_fade_music(common_externals.get_midi_name(midi), midi, time);
		break;
	}
}

void pause_midi()
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_pause_music();
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_pause_music();
		break;
	}
}

void restart_midi()
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_resume_music();
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_resume_music();
		break;
	}
}

void stop_midi()
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_stop_music();
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_stop_music();
		break;
	}
}

uint midi_status()
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		return ff7music_music_status();
		break;
	case FFNX_MUSIC_WINAMP:
		return winamp_music_status();
		break;
	}
}

void set_master_midi_volume(uint volume)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_set_master_music_volume(volume);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_set_master_music_volume(volume);
		break;
	}
}

void set_midi_volume(uint volume)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_set_music_volume(volume);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_set_music_volume(volume);
		break;
	}
}

void set_midi_volume_trans(uint volume, uint step)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_set_music_volume_trans(volume, step);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_set_music_volume_trans(volume, step);
		break;
	}
}

void set_midi_tempo(unsigned char tempo)
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_set_music_tempo(tempo);
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_set_music_tempo(tempo);
		break;
	}
}

void music_cleanup()
{
	switch (use_external_music)
	{
	case FFNX_MUSIC_FF7MUSIC:
		ff7music_music_cleanup();
		break;
	case FFNX_MUSIC_WINAMP:
		winamp_music_cleanup();
		break;
	}
}

bool is_wm_theme(char* midi)
{
	if (nullptr == midi)
	{
		return false;
	}

	if (ff8)
	{
		return strcmp(midi, "041s-field.sgt") == 0;
	}
	
	return strcmp(midi, "TA") == 0 || strcmp(midi, "TB") == 0 || strcmp(midi, "KITA") == 0;
}

bool needs_resume(uint old_mode, uint new_mode, char* old_midi, char* new_midi)
{
	/*
	 * BATTLE -> FIELD or WM
	 * FIELD  -> WM
	 */
	return ((new_mode == MODE_WORLDMAP || new_mode == MODE_AFTER_BATTLE) && !is_wm_theme(old_midi) && is_wm_theme(new_midi))
		|| (old_mode == MODE_BATTLE || old_mode == MODE_SWIRL)
		&& (new_mode == MODE_FIELD || new_mode == MODE_AFTER_BATTLE);
}
