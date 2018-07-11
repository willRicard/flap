#ifndef FLAP_FLAP_H
#define FLAP_FLAP_H

#define FLAP_WINDOW_TITLE "Flap"
#define FLAP_WINDOW_WIDTH 800
#define FLAP_WINDOW_HEIGHT 450

#define FLAP_SCROLL_SPEED -0.004f

#define FLAP_BIRD_X -0.75f
#define FLAP_BIRD_Y 0.75f
#define FLAP_BIRD_WIDTH 0.08f
#define FLAP_BIRD_HEIGHT 0.14f

#define FLAP_NUM_PIPES 4
#define FLAP_PIPE_WIDTH 0.12f
#define FLAP_PIPE_MIN_HEIGHT 0.25f
#define FLAP_PIPE_MAX_HEIGHT 1.0f
#define FLAP_PIPE_OPENING 0.34f
#define FLAP_PIPE_STEP 0.5f

#define GLSL(src) "#version 330 core\n" #src

#endif // FLAP_FLAP_H
