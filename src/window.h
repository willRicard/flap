#ifndef FLAP_WINDOW_H_
#define FLAP_WINDOW_H_

void flapWindowInit();

void flapWindowQuit();

int flapWindowShouldClose();

float flapWindowGetTime();

int flapWindowGetThrust();

void flapWindowUpdate();

void flapWindowRender();

void flapWindowFailWithError(const char *error);

#endif // FLAP_WINDOW_H_
