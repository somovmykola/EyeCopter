#ifndef DRAWING_H
#define DRAWING_H

enum eFloorTypes {
	none,
	solid,
	checker
};

int InitGL();
int InitDrawing();

int CleanupGL();
int CleanupDrawing();

void UpdateMVP(int n);

void DrawScene();

#endif