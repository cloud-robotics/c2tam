/*
*
* This file is part of the c2tam project
*
* c2tam is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation, either version 3 of the License, or 
* (at your option) any later version.
*
* c2tam is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with c2tam. If not, see http://www.gnu.org/licenses/.
*
*/

#include "MapViewer.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include "LevelHelpers.h"
#include <iomanip>

#include "OpenGL.h"

#include <cvd/gl_helpers.h>

using namespace CVD;
using namespace std;


MapViewer::MapViewer(Map &map):
  mMap(map)
{
  mse3ViewerFromWorld =
    SE3<>::exp(makeVector(0,0,2,0,0,0)) * SE3<>::exp(makeVector(0,0,0,0.8 * M_PI,0,0));
}

void MapViewer::DrawMapDots()
{
  SetupFrustum();
  SetupModelView();

  int nForMass = 0;
  glColor3f(0,1,1);
  glPointSize(3);
  glBegin(GL_POINTS);
  mv3MassCenter = Zeros;
  for(size_t i=0; i<mMap.vpPoints.size(); i++)
    {
      Vector<3> v3Pos = mMap.vpPoints[i]->v3WorldPos;
      glColor(gavLevelColors[mMap.vpPoints[i]->nSourceLevel]);
      if(v3Pos * v3Pos < 10000)
	{
	  nForMass++;
	  mv3MassCenter += v3Pos;
	}
      glVertex(v3Pos);
    }
  glEnd();
  mv3MassCenter = mv3MassCenter / (0.1 + nForMass);
}

void MapViewer::DrawMapDotsUpdater()
{
  SetupFrustum();
  SetupModelView();

  int nForMass = 0;
  glColor3f(0,1,1);
  glPointSize(3);
  glBegin(GL_POINTS);
  mv3MassCenter = Zeros;
  for(size_t i=0; i<mMap.vpPoints.size(); i++)
    {
      Vector<3> v3Pos = mMap.vpPoints[i]->v3WorldPos;
      if(mMap.vpPoints[i]->update)
        glColor3f(0,1,0);
      else
        glColor3f(1,0,0);
      if(v3Pos * v3Pos < 10000)
	{
	  nForMass++;
	  mv3MassCenter += v3Pos;
	}
      glVertex(v3Pos);
    }
  glEnd();
  mv3MassCenter = mv3MassCenter / (0.1 + nForMass);
}


void MapViewer::DrawGrid()
{
  SetupFrustum();
  SetupModelView();
  glLineWidth(1);

  glBegin(GL_LINES);

  // Draw a larger grid around the outside..
  double dGridInterval = 0.1;

  double dMin = -100.0 * dGridInterval;
  double dMax =  100.0 * dGridInterval;

  for(int x=-10;x<=10;x+=1)
    {
      if(x==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.3,0.3,0.3);
      glVertex3d((double)x * 10 * dGridInterval, dMin, 0.0);
      glVertex3d((double)x * 10 * dGridInterval, dMax, 0.0);
    }
  for(int y=-10;y<=10;y+=1)
    {
      if(y==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.3,0.3,0.3);
      glVertex3d(dMin, (double)y * 10 *  dGridInterval, 0.0);
      glVertex3d(dMax, (double)y * 10 * dGridInterval, 0.0);
    }

  glEnd();

  glBegin(GL_LINES);
  dMin = -10.0 * dGridInterval;
  dMax =  10.0 * dGridInterval;

  for(int x=-10;x<=10;x++)
    {
      if(x==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.5,0.5,0.5);

      glVertex3d((double)x * dGridInterval, dMin, 0.0);
      glVertex3d((double)x * dGridInterval, dMax, 0.0);
    }
  for(int y=-10;y<=10;y++)
    {
      if(y==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.5,0.5,0.5);
      glVertex3d(dMin, (double)y * dGridInterval, 0.0);
      glVertex3d(dMax, (double)y * dGridInterval, 0.0);
    }

  glColor3f(1,0,0);
  glVertex3d(0,0,0);
  glVertex3d(1,0,0);
  glColor3f(0,1,0);
  glVertex3d(0,0,0);
  glVertex3d(0,1,0);
  glColor3f(1,1,1);
  glVertex3d(0,0,0);
  glVertex3d(0,0,1);
  glEnd();
}

void MapViewer::DrawMap(SE3<> se3CamFromWorld, GLWindow2 &w,bool dense)
{
  mMessageForUser.str(""); // Wipe the user message clean

  // Update viewer position according to mouse input:
  {
    pair<Vector<6>, Vector<6> > pv6 = w.GetMousePoseUpdate();
    SE3<> se3CamFromMC;
    se3CamFromMC.get_translation() = mse3ViewerFromWorld * mv3MassCenter;
    mse3ViewerFromWorld = SE3<>::exp(pv6.first) *
      se3CamFromMC * SE3<>::exp(pv6.second) * se3CamFromMC.inverse() * mse3ViewerFromWorld;
  }

  w.SetupViewport();
  glClearColor(0,0,0,0);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(1,1,1,1);

  glEnable(GL_DEPTH_TEST);
  //DrawGrid();

  if(dense){
    if (mMap.enableDense) //DCTAM
      DrawDense(); //DCTAM
  }

  DrawMapDots();
  DrawCamera(se3CamFromWorld);
  for(size_t i=0; i<mMap.vpKeyFrames.size(); i++)
    DrawCameraColor(mMap.vpKeyFrames[i]->se3CfromW, true,mMap.vpKeyFrames[i]->color);
 //    DrawCamera(mMap.vpKeyFrames[i]->se3CfromW, true);

//  if(dense){
//    if (mMap.enableDense) //DCTAM
//      DrawDense(); //DCTAM
//  }

  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  mMessageForUser << " Map: " << mMap.vpPoints.size() << "P, " << mMap.vpKeyFrames.size() << "KF";
  mMessageForUser << setprecision(4);
  mMessageForUser << "   Camera Pos: " << se3CamFromWorld.inverse().get_translation();
}

// DCTAM
void MapViewer::DrawDense()
{ 
  bool find = false;
  int index = 0;

  for(size_t i=1; i<mMap.vpKeyFrames.size(); i++){
    find = false;
    for(int j=0; j< mMap.denseID.size(); j++){
      if(i == mMap.denseID[j]){
        find = true;
        index = j;
        //fprintf(stderr,"INDEX %d\n",index);
      }
    }


//if (i <= 3){
    if (find){
      //fprintf(stderr,"DrawDense\n");
//    SetupModelView();
    SetupFrustum();
    SetupModelView(mMap.vpKeyFrames[i]->se3CfromW.inverse());
//glPushMatrix();
    glScalef(mMap.denseScale,mMap.denseScale,mMap.denseScale);
    //glScalef(1.2,1.2,1.2);

    glEnableClientState(GL_VERTEX_ARRAY);
    //Enable the color array functionality (so we can specify a color for each vertex)
    glEnableClientState(GL_COLOR_ARRAY);

    //pass the vertex pointer:
    glVertexPointer( 3,   //3 components per vertex (x,y,z)
                       GL_FLOAT,
                       sizeof(SVertex),
                       mMap.densePoints[index]);
    //pass the color pointer
    glColorPointer( 3,   //3 components per vertex (r,g,b)
                      GL_FLOAT,
                      sizeof(SVertex),
                      &mMap.densePoints[index][0].r);  //Pointer to the first color
    glDrawArrays(GL_POINTS,0,640*480);
    glDisableClientState(GL_VERTEX_ARRAY);
    //Enable the color array functionality (so we can specify a color for each vertex)
    glDisableClientState(GL_COLOR_ARRAY);
        glPopMatrix();
    }
  }
//}
}


void MapViewer::DrawMapUpdater(SE3<> se3CamFromWorld, GLWindow2 &w, bool dense)
{
  mMessageForUser.str(""); // Wipe the user message clean

  // Update viewer position according to mouse input:
  {
    pair<Vector<6>, Vector<6> > pv6 = w.GetMousePoseUpdate();
    SE3<> se3CamFromMC;
    se3CamFromMC.get_translation() = mse3ViewerFromWorld * mv3MassCenter;
    mse3ViewerFromWorld = SE3<>::exp(pv6.first) *
      se3CamFromMC * SE3<>::exp(pv6.second) * se3CamFromMC.inverse() * mse3ViewerFromWorld;
  }

  w.SetupViewport();
  glClearColor(0,0,0,0);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(1,1,1,1);

  glEnable(GL_DEPTH_TEST);
  //DrawGrid();
  DrawMapDotsUpdater();
  DrawCamera(se3CamFromWorld);
  for(size_t i=0; i<mMap.vpKeyFrames.size(); i++)
    DrawCamera(mMap.vpKeyFrames[i]->se3CfromW, true);

//  if(dense){
//    if (mMap.enableDense) // DCTAM
//      DrawDense(); // DCTAM
//  }

  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  mMessageForUser << " Map: " << mMap.vpPoints.size() << "P, " << mMap.vpKeyFrames.size() << "KF";
  mMessageForUser << setprecision(4);
  mMessageForUser << "   Camera Pos: " << se3CamFromWorld.inverse().get_translation();
}

string MapViewer::GetMessageForUser()
{
  return mMessageForUser.str();
}

void MapViewer::SetupFrustum()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double zNear = 0.03;
  glFrustum(-zNear, zNear, 0.75*zNear,-0.75*zNear,zNear,50);
  glScalef(1,1,-1);
  return;
};

void MapViewer::SetupModelView(SE3<> se3WorldFromCurrent)
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrix(mse3ViewerFromWorld * se3WorldFromCurrent);
  return;
};

void MapViewer::DrawCamera(SE3<> se3CfromW, bool bSmall)
{

  SetupModelView(se3CfromW.inverse());
  SetupFrustum();


  if(bSmall)
    glLineWidth(1);
  else
    glLineWidth(3);

  glBegin(GL_LINES);
  glColor3f(1,0,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.1f, 0.0f, 0.0f);
  glColor3f(0,1,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.1f, 0.0f);
  glColor3f(1,1,1);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.1f);
  glEnd();


  if(!bSmall)
  {
	  glLineWidth(1);
	  glColor3f(0.5,0.5,0.5);
	  SetupModelView();
	  Vector<2> v2CamPosXY = se3CfromW.inverse().get_translation().slice<0,2>();
	  glBegin(GL_LINES);
	  glColor3f(1,1,1);
	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] + 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] - 0.04);
  	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] - 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] + 0.04);
	  glEnd();
  }

/*
  glColor3f(0.5,0.5,0.5);
  glutSolidCube( 0.07 );

  glColor3f(0.,0.,0.);
  // Now draw cylinder
  double width = 0.015;
  GLUquadricObj *cylinderptr;
  cylinderptr = gluNewQuadric();
  gluQuadricDrawStyle( cylinderptr, GLU_FILL );
  gluQuadricNormals( cylinderptr, GLU_SMOOTH );
  glTranslatef( 0.0, 0.0, -width / 2.0 );

  // So we can draw our cylinder along -z direction
  glRotatef( 180.0, 1.0, 0.0, 0.0 );
  gluCylinder( cylinderptr, 0.7 * width / 2.0,
               0.8 * width / 2.0, 0.4 * width, 20, 20 );

*/
}

void MapViewer::DrawCameraColor(SE3<> se3CfromW, bool bSmall,int color)
{
  //fprintf(stderr,"cx = %f",se3CfromW.get_translation()[0]);

  SetupModelView(se3CfromW.inverse());
  SetupFrustum();


  if(bSmall)
    glLineWidth(1);
  else
    glLineWidth(3);

  glBegin(GL_LINES);
  glColor3f(1,0,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.1f, 0.0f, 0.0f);
  glColor3f(0,1,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.1f, 0.0f);
  glColor3f(1,1,1);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.1f);
  glEnd();


  if(!bSmall)
  {
	  glLineWidth(1);
	  glColor3f(0.5,0.5,0.5);
	  SetupModelView();
	  Vector<2> v2CamPosXY = se3CfromW.inverse().get_translation().slice<0,2>();
	  glBegin(GL_LINES);
	  glColor3f(1,1,1);
	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] + 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] - 0.04);
  	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] - 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] + 0.04);
	  glEnd();
  }
/*
  if (color == 1)
    glColor3f(1.0,0.0,1.0);

  else if (color == 2)
    glColor3f(0.0,1.0,0.0);

  else if (color == 3)
    glColor3f(0.0,0.0,1.0);

  else if (color == 4)
    glColor3f(0.0,1.0,1.0);
  else if (color == 5)
    glColor3f(1.0,0.0,1.0);

  else if (color == 6)
    glColor3f(1.0,1.0,0.0);

  else if (color == 10)
    glColor3f(1.0,0.0,0.0);

  else
    glColor3f(0.5,0.5,0.5);
  
glutSolidCube( 0.07 );

  glColor3f(0.,0.,0.);
  // Now draw cylinder
  double width = 0.015;
  GLUquadricObj *cylinderptr;
  cylinderptr = gluNewQuadric();
  gluQuadricDrawStyle( cylinderptr, GLU_FILL );
  gluQuadricNormals( cylinderptr, GLU_SMOOTH );
  glTranslatef( 0.0, 0.0, -width / 2.0 );

  // So we can draw our cylinder along -z direction
  glRotatef( 180.0, 1.0, 0.0, 0.0 );
  gluCylinder( cylinderptr, 0.7 * width / 2.0,
               0.8 * width / 2.0, 0.4 * width, 20, 20 );

*/
}



