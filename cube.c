/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * Copyright (c) 2005 Jesper Svennevid
 */

// Modified by Geecko, 2010

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pspgu.h>
#include <pspgum.h>

#include "callbacks.h"
#include "vram.h"
#include "intraFont.h"


PSP_MODULE_INFO("3D Cube Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

static unsigned int __attribute__((aligned(16))) list[262144];
extern unsigned char logo_start[];
SceCtrlData pad, oldPad;
intraFont* font;

int dist=0, step = 0, dir = 1, anaglyph = 0;
float cam_pos = 0.25f, cam_rot = 0.042f;

struct Vertex
{
  float u, v;
  unsigned int color;
  float x,y,z;
};

// cube vertices
struct Vertex __attribute__((aligned(16))) vertices[12*3] =
{
  {0, 0, 0xffcf0000,-1,-1, 1}, // 0
  {1, 0, 0xffcf0000,-1, 1, 1}, // 4
  {1, 1, 0xffcf0000, 1, 1, 1}, // 5

  {0, 0, 0xffcf0000,-1,-1, 1}, // 0
  {1, 1, 0xffcf0000, 1, 1, 1}, // 5
  {0, 1, 0xffcf0000, 1,-1, 1}, // 1

  {0, 0, 0xffcf0000,-1,-1,-1}, // 3
  {1, 0, 0xffcf0000, 1,-1,-1}, // 2
  {1, 1, 0xffcf0000, 1, 1,-1}, // 6

  {0, 0, 0xffcf0000,-1,-1,-1}, // 3
  {1, 1, 0xffcf0000, 1, 1,-1}, // 6
  {0, 1, 0xffcf0000,-1, 1,-1}, // 7

  {0, 0, 0xff00cf00, 1,-1,-1}, // 0
  {1, 0, 0xff00cf00, 1,-1, 1}, // 3
  {1, 1, 0xff00cf00, 1, 1, 1}, // 7

  {0, 0, 0xff00cf00, 1,-1,-1}, // 0
  {1, 1, 0xff00cf00, 1, 1, 1}, // 7
  {0, 1, 0xff00cf00, 1, 1,-1}, // 4

  {0, 0, 0xff00cf00,-1,-1,-1}, // 0
  {1, 0, 0xff00cf00,-1, 1,-1}, // 3
  {1, 1, 0xff00cf00,-1, 1, 1}, // 7

  {0, 0, 0xff00cf00,-1,-1,-1}, // 0
  {1, 1, 0xff00cf00,-1, 1, 1}, // 7
  {0, 1, 0xff00cf00,-1,-1, 1}, // 4

  {0, 0, 0xff0000cf,-1, 1,-1}, // 0
  {1, 0, 0xff0000cf, 1, 1,-1}, // 1
  {1, 1, 0xff0000cf, 1, 1, 1}, // 2

  {0, 0, 0xff0000cf,-1, 1,-1}, // 0
  {1, 1, 0xff0000cf, 1, 1, 1}, // 2
  {0, 1, 0xff0000cf,-1, 1, 1}, // 3

  {0, 0, 0xff0000cf,-1,-1,-1}, // 4
  {1, 0, 0xff0000cf,-1,-1, 1}, // 7
  {1, 1, 0xff0000cf, 1,-1, 1}, // 6

  {0, 0, 0xff0000cf,-1,-1,-1}, // 4
  {1, 1, 0xff0000cf, 1,-1, 1}, // 6
  {0, 1, 0xff0000cf, 1,-1,-1}, // 5
};

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

void drawScene()
{
  sceGuClear(GU_DEPTH_BUFFER_BIT);

  // setup texture
  sceGuTexMode(GU_PSM_4444,0,0,0);
  sceGuTexImage(0,64,64,64,logo_start);
  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGB);
  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
  sceGuTexScale(1.0f,1.0f);
  sceGuTexOffset(0.0f,0.0f);
  sceGuColor(0xffffffff);
  sceGuDisable(GU_BLEND);

  // First cube (centered)
  sceGumMatrixMode(GU_MODEL);
  sceGumLoadIdentity();
  {
    ScePspFVector3 pos = { 0, 0, -3.f - dist/50.f };
    ScePspFVector3 rot = { step * 0.79f * (GU_PI/180.0f), step * 0.38f * (GU_PI/180.0f), step * 0.32f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 1
  {
    ScePspFVector3 pos = { 0, 5.f, 0};
    ScePspFVector3 rot = { -step * 0.49f * (GU_PI/180.0f), step * 0.08f * (GU_PI/180.0f), step * 0.53f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 2
  {
    ScePspFVector3 pos = { 0, 0, 7.5f};
    ScePspFVector3 rot = { step * 0.23f * (GU_PI/180.0f), step * 0.94f * (GU_PI/180.0f), step * 0.45f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 3
  {
    ScePspFVector3 pos = { 10.f, 0, 0};
    ScePspFVector3 rot = { -step * 0.75f * (GU_PI/180.0f), step * 0.94f * (GU_PI/180.0f), -step * 0.39f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 4
  {
    ScePspFVector3 pos = { 0, 0, 12.5f};
    ScePspFVector3 rot = { -step * 0.29f * (GU_PI/180.0f), -step * 0.38f * (GU_PI/180.0f), step * 0.81f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 5
  {
    ScePspFVector3 pos = { 0, 15.f, 0};
    ScePspFVector3 rot = { -step * 0.93f * (GU_PI/180.0f), -step * 0.28f * (GU_PI/180.0f), -step * 0.4f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 6
  {
    ScePspFVector3 pos = { 0, 0, 17.5f};
    ScePspFVector3 rot = { -step * 0.79f * (GU_PI/180.0f), -step * 0.98f * (GU_PI/180.0f), -step * 1.32f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 7
  {
    ScePspFVector3 pos = { 0, 0, 20.f};
    ScePspFVector3 rot = { step * 0.45f * (GU_PI/180.0f), -step * 0.28f * (GU_PI/180.0f), -step * 0.39f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);

  // Satellite 8
  {
    ScePspFVector3 pos = { 22.5f, 0, 0};
    ScePspFVector3 rot = { step * 0.79f * (GU_PI/180.0f), -step * 0.98f * (GU_PI/180.0f), -step * 0.42f * (GU_PI/180.0f) };
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);
  }
  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,vertices);
}

int main(int argc, char* argv[])
{
  setupCallbacks();

  // setup IntraFont
  intraFontInit();
  font = intraFontLoad("flash0:/font/ltn8.pgf",INTRAFONT_CACHE_MED);

  // setup GU

  void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
  void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

  sceGuInit();

  sceGuStart(GU_DIRECT,list);
  sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
  sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
  sceGuDepthBuffer(zbp,BUF_WIDTH);
  sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
  sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
  sceGuDepthRange(65535,0);
  sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuDepthFunc(GU_GEQUAL);
  sceGuEnable(GU_DEPTH_TEST);
  sceGuFrontFace(GU_CW);
  sceGuShadeModel(GU_SMOOTH);
  sceGuEnable(GU_CULL_FACE);
  sceGuEnable(GU_TEXTURE_2D);
  sceGuEnable(GU_CLIP_PLANES);
  sceGuFinish();
  sceGuSync(0,0);

  sceDisplayWaitVblankStart();
  sceGuDisplay(GU_TRUE);

  // clear setup
  sceGuClearColor(0xffffffff);
  sceGuClearDepth(0);

  // setup projection matrix
  sceGumMatrixMode(GU_PROJECTION);
  sceGumLoadIdentity();
  sceGumPerspective(60.0f,16.0f/9.0f,0.5f,1000.0f);

  // run sample

  while(running())
  {
    oldPad = pad;
    sceCtrlPeekBufferPositive(&pad,1);
    if ((pad.Buttons & PSP_CTRL_CROSS) && !(oldPad.Buttons & PSP_CTRL_CROSS))
      anaglyph = !anaglyph; // switches 3D mode.

    if (anaglyph)
    {
      if      (pad.Buttons & PSP_CTRL_LEFT)  cam_pos -= 0.01f;
      else if (pad.Buttons & PSP_CTRL_RIGHT) cam_pos += 0.01f;
      if      (pad.Buttons & PSP_CTRL_DOWN)  cam_rot -= 0.01f;
      else if (pad.Buttons & PSP_CTRL_UP)    cam_rot += 0.01f;
    }

    sceGuStart(GU_DIRECT,list);

    // clear screen
    sceGuClear(GU_COLOR_BUFFER_BIT);

    if (!anaglyph)
    {
      // render scene for the each eye
      sceGumMatrixMode(GU_VIEW);
      sceGumLoadIdentity();
      drawScene();
    }
    else
    {
      // render scene for the left eye
      sceGumMatrixMode(GU_VIEW);
      sceGumLoadIdentity();
      {
        ScePspFVector3 pos = { cam_pos, 0, 0 };
        ScePspFVector3 rot = { 0, cam_rot, 0 };
        sceGumTranslate(&pos);
        sceGumRotateXYZ(&rot);
      }
      sceGuPixelMask(0x00ffff00); // Only cyan
      drawScene();

      // render scene for the right eye
      sceGumMatrixMode(GU_VIEW);
      sceGumLoadIdentity();
      {
        ScePspFVector3 pos = { -cam_pos, 0, 0 };
        ScePspFVector3 rot = { 0, -cam_rot, 0 };
        sceGumTranslate(&pos);
        sceGumRotateXYZ(&rot);
      }
      sceGuPixelMask(0x000000ff); // Only red
      drawScene();
    }

    // UI
    sceGuPixelMask(0x00000000);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);

    intraFontSetStyle(font,1.0f,0xFF000000,0xFFFFFFFF,INTRAFONT_ALIGN_CENTER);
    intraFontPrintf(font,480/2,272-10,(anaglyph)?"3D Mode Enabled - Press Cross to switch":
                                                 "3D Mode Disabled - Press Cross to switch");
    if (anaglyph)
    {
      intraFontSetStyle(font,0.9f,0xFF000000,0xFFFFFFFF,INTRAFONT_ALIGN_LEFT);
      intraFontPrintf(font,1,10,"%g/%g",cam_pos,cam_rot);
    }

    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();

    step++; // For rotation purposes
    dist += dir; // Distance of the main cube
    if (dist > 60*15 || dist < 0) dir = -dir;
  }

  sceGuTerm();

  sceKernelExitGame();
  return 0;
}
