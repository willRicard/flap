#ifndef FLAP_WINDOW_H_
#define FLAP_WINDOW_H_

void flapWindowInit();

void flapWindowQuit();

int flapWindowShouldClose();

double flapWindowGetTime();

int flapWindowThrust();

void flapWindowUpdate();

void flapWindowRender();

#endif // FLAP_WINDOW_H_
