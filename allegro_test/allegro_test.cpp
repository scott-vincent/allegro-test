/*
 * This is a test to make sure allegro is working correctly.
 * It can be compiled on both Windows (with Visual Studio Community Edition)
 * and Raspberry Pi 4 (make.sh).
 * 
 * To install Allegro (5.2.7) on Windows:
 * 
 *     Load the allegro_test.sln into Visual Studio Community Edition.
 *     When you build the solution it will automatically download Allegro
 *     as it is specified in packages.config. A new folder called
 *     packages will be created under allegro_test.
 * 
 * To install Allegro on Raspberry Pi:
 * 
 *     apt install liballegro5-dev
 * 
 * Copyright (c) 2021 Scott Vincent
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
// Windows only
#include <Windows.h>
#define millisecEpoch GetTickCount64
#else
#include <sys/time.h>
#define millisecEpoch GetCurrentMillis
#endif
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

// Constants
const bool Debug = false;
const bool UseOpenGL_ES3 = false;
const int _rows = 3;
const int _columns = 4;
const char _bitmap[] = "instrument.png";
const int DataRateFps = 30;
const double DegreesToRadians = ALLEGRO_PI / 180.0f;

// Variables
ALLEGRO_FONT* _font = NULL;
ALLEGRO_DISPLAY* _display = NULL;
ALLEGRO_TIMER* _timer = NULL;
ALLEGRO_EVENT_QUEUE* _eventQueue = NULL;
int _displayWidth;
int _displayHeight;
bool _quit = false;
ALLEGRO_BITMAP* _bmp[64];
int _bmpCount = 0;
double _scaleFactor;
double _angle = 0;
long long _startTime;
long long _prevFrameTime = 0;
int _frameCount;
long long _prevFpsTime;
long long _prevFrameCount;
double _fps;
double _avgFps;

#ifndef _WIN32
long long GetCurrentMillis()
{
    timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}
#endif

/// <summary>
/// Initialise Allegro
/// </summary>
bool init()
{
    if (!al_init()) {
        printf("Failed to initialise Allegro\n");
        return false;
    }

    if (!al_init_font_addon()) {
        printf("Failed to initialise font\n");
        return false;
    }

    if (!al_init_image_addon()) {
        printf("Failed to initialise image\n");
        return false;
    }

    if (!al_install_keyboard()) {
        printf("Failed to initialise keyboard\n");
        return false;
    }

    if (!(_eventQueue = al_create_event_queue())) {
        printf("Failed to create event queue\n");
        return false;
    }

    if (!(_font = al_create_builtin_font())) {
        printf("Failed to create font\n");
        return false;
    }

    al_set_new_window_title("Allegro Test");

    // Use existing desktop resolution/refresh rate and force OpenGL ES 3
    // for Raspberry Pi 4 hardware acceleration compatibility.
    int flags;
    if (Debug) {
        flags = ALLEGRO_WINDOWED;
    }
    else {
        flags = ALLEGRO_FULLSCREEN_WINDOW | ALLEGRO_FRAMELESS;
    }

    if (UseOpenGL_ES3) {
        // If the Raspberry Pi 4 is not configured correctly for hardware OpenGL
        // this line may give a black screen.
        al_set_new_display_flags(flags | ALLEGRO_OPENGL_3_0 | ALLEGRO_OPENGL_ES_PROFILE);
    }
    else {
        al_set_new_display_flags(flags);
    }

#ifdef _WIN32
    // Turn on vsync (fails on Raspberry Pi)
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);
#endif

    // Resolution is ignored for fullscreen window (uses existing desktop resolution)
    // but fails on Rasberry Pi if set to 0!
    if ((_display = al_create_display(1200, 800)) == NULL) {
        printf("Failed to create display\n");
        return false;
    }

    _displayWidth = al_get_display_width(_display);
    _displayHeight = al_get_display_height(_display);

    al_hide_mouse_cursor(_display);
    al_inhibit_screensaver(true);

    al_register_event_source(_eventQueue, al_get_keyboard_event_source());
    al_register_event_source(_eventQueue, al_get_display_event_source(_display));

    if (!(_timer = al_create_timer(1.0 / DataRateFps))) {
        printf("Failed to create timer\n");
        return false;
    }

    al_register_event_source(_eventQueue, al_get_timer_event_source(_timer));

    return true;
}

/// <summary>
/// Cleanup Allegro
/// </summary>
void cleanup()
{
    for (int i = 0; i < _bmpCount; i++) {
        al_destroy_bitmap(_bmp[i]);
    }

    if (_timer) {
        al_destroy_timer(_timer);
    }

    if (_eventQueue) {
        al_destroy_event_queue(_eventQueue);
    }

    if (_font) {
        al_destroy_font(_font);
    }

    if (_display) {
        al_destroy_display(_display);
        al_inhibit_screensaver(false);
    }
}

bool initInstrument(int size)
{
    _scaleFactor = size / 800.0f;

    ALLEGRO_BITMAP* orig = al_load_bitmap(_bitmap);
    if (!orig) {
        printf("Missing file: %s\n", _bitmap);
        return false;
    }

    _bmp[0] = al_create_bitmap(size, size);

    _bmp[1] = al_create_bitmap(size, size);
    al_set_target_bitmap(_bmp[1]);
    al_draw_scaled_bitmap(orig, 0, 0, 800, 800, 0, 0, size, size, 0);

    _bmp[2] = al_create_bitmap(50, 600);
    al_set_target_bitmap(_bmp[2]);
    al_draw_bitmap_region(orig, 800, 0, 50, 600, 0, 0, 0);

    al_set_target_backbuffer(_display);

    _bmpCount = 3;
    al_destroy_bitmap(orig);
    return true;
}

void renderInstrument(int xPos, int yPos, int angleOffset)
{
    al_set_target_bitmap(_bmp[0]);
    al_draw_bitmap(_bmp[1], 0, 0, 0);
    al_draw_scaled_rotated_bitmap(_bmp[2], 25, 400, 400 * _scaleFactor, 400 * _scaleFactor, _scaleFactor, _scaleFactor, (_angle + angleOffset) * DegreesToRadians, 0);

    al_set_target_backbuffer(_display);
    al_draw_bitmap(_bmp[0], xPos, yPos, 0);
}

/// <summary>
/// Initialise the instruments
/// </summary>
bool doInit()
{
    int maxWidth = (_displayWidth / _columns);
    int maxHeight = (_displayHeight / _rows);

    int size = maxWidth - 10;
    if (maxHeight < maxWidth) {
        size = maxHeight - 10;
    }
    if (size < 20) {
        size = 20;
    }

    if (!initInstrument(size)) {
        return false;
    }

    return true;
}

/// <summary>
/// Render the next frame
/// </summary>
void doRender()
{
    // Clear background
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // Draw Instruments
    int offset = 0;
    for (int row = 0; row < _rows; row++) {
        for (int col = 0; col < _columns; col++) {
            renderInstrument(20 + col * (_displayWidth / _columns), 20 + row * (_displayHeight / _rows), offset);
            offset += 360 / (_rows * _columns);
        }
    }

    long long frameTime = millisecEpoch();

    if (_prevFrameTime == 0) {
        _frameCount = 0;
        _startTime = frameTime;
        _prevFrameCount = 0;
        _prevFpsTime = frameTime;
    }
    else {
        _frameCount++;

        if (frameTime - _prevFpsTime > 400) {
            _fps = (_frameCount - _prevFrameCount) * 1000.0 / (frameTime - _prevFpsTime);
            _avgFps = _frameCount * 1000.0 / (frameTime - _startTime);
            _prevFrameCount = _frameCount;
            _prevFpsTime = frameTime;
        }

        char fpsDisplay[256];
        sprintf(fpsDisplay, "FPS: %.1f   Average FPS: %.1f", _fps, _avgFps);
        al_draw_text(_font, al_map_rgb(0xa0, 0xa0, 0xa0), (_displayWidth / 2) - 100, 5, 0, fpsDisplay);
    }

    _prevFrameTime = frameTime;
}

/// <summary>
/// Update everything before the next frame
/// </summary>
void doUpdate()
{
    _angle++;
}

/// <summary>
/// Handle keypress
/// </summary>
void doKeypress(ALLEGRO_EVENT* event)
{
    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_ESCAPE:
            // Quit program
            _quit = true;
            break;
    }
}

///
/// main
///
int main(int argc, char **argv)
{
    if (!init()) {
        cleanup();
        exit(1);
    }

    if (!doInit()) {
        cleanup();
        exit(1);
    }

    doUpdate();

    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(_timer);
    printf("Press Esc to quit\n");
    _startTime = millisecEpoch();

    while (!_quit) {
        al_wait_for_event(_eventQueue, &event);

        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                doUpdate();
                redraw = true;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                doKeypress(&event);
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                _quit = true;
                break;
        }

        if (redraw && al_is_event_queue_empty(_eventQueue) && !_quit) {
            doRender();
            al_flip_display();
            redraw = false;
        }
    }

    cleanup();
    return 0;
}
