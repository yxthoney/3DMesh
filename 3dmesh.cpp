#include <fstream>
#include <iostream>
#include <gl/glut.h>
#include <gl/glui.h>
#include <math.h>
#include <vector>
#include <list> 
#include <map>
#include "Leap.h"

using namespace std;
using namespace Leap;

#define TRANSFORM_NONE    0 
#define TRANSFORM_ROTATE  1
#define TRANSFORM_SCALE   2 
#define TRANSFORM_MODE    3
#define OBJ_SMOOTH	  0
#define OBJ_FLAT	  1
#define OBJ_POINT     2 
#define OBJ_WIREFRAME 3
#define PRO_PERSPECTIVE 0
#define PRO_ORTHOGONAL  1

static int press_x, press_y; 
static float x_angle = 0.0; 
static float y_angle = 0.0; 
static float scale_size = 1; 
static float trans_xy[2]={0,0};
static float trans_z=0.0;
static int obj_mode = OBJ_SMOOTH;
static int xform_mode = 0; 
static int pro_mode = 0;

#define CB_READFILE 0

GLUI_FileBrowser *fb;

char filename[20];
static int ground=1;
static int axes=1;
static int box=0;
static float red=1.0;
static float green=1.0;
static float blue=1.0;
static int diffuse=1;
static int specular=1;
double xmin,xmax,ymin,ymax,zmin,zmax;

//Shared Vertex Data Structure
class SV_vert{
public:
	float x,y,z;
	float nx,ny,nz;
};
class SV_face{
public:
	int v1,v2,v3;
};
vector<SV_vert> svv;
vector<SV_face> svf;



int readFile( char* filename ){
	svv.clear();
	svf.clear();
	int vi,v1,v2,v3;
	double x,y,z;	
	bool first=true;
	SV_face tempf;
	SV_vert tempv;
	fstream fin(filename, ios::in);
	if(!fin) return 0;
	char temp[100];
	while(!fin.eof()){
		fin>>temp;
		if(!strcmp(temp,"Vertex")){			
			fin>>vi>>x>>y>>z;
			tempv.x=x;
			tempv.y=y;
			tempv.z=z;
			if(first){
				xmax=xmin=x;
				ymax=ymin=y;
				zmax=zmin=z;
				first=false;
			}
			else{
				if(x>xmax) xmax=x;
				if(x<xmin) xmin=x;
				if(y>ymax) ymax=y;
				if(y<ymin) ymin=y;
				if(z>zmax) zmax=z;
				if(z<zmin) zmin=z;
			}
			fin>>temp;
			char *p;
			p= strtok(temp,"(");
			if(!strcmp(p,"{normal=")){	
				p=strtok(NULL,"{");
				x=atof(p);
				fin>>y>>z;
				tempv.nx=x;
				tempv.ny=y;
				tempv.nz=z;					
			}
			else{
				tempv.nx=0;
				tempv.ny=0;
				tempv.nz=0;
			}
			svv.push_back(tempv);
			fin>>temp;
		}		
		if(!strcmp(temp,"Face")){			
			fin>>vi>>v1>>v2>>v3;
			tempf.v1=v1-1;
			tempf.v2=v2-1;
			tempf.v3=v3-1;
			svf.push_back(tempf);
		}
	}
	fin.close();
	return 1;
}

void display(void){
	// Just clean the screen
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	// setup the perspective projection
	if(trans_xy[0]>10) trans_xy[0]=10;
	if(trans_xy[1]>10) trans_xy[1]=10;
	if(trans_xy[0]<-10) trans_xy[0]=-10;
	if(trans_xy[1]<-10) trans_xy[1]=-10;
	if(pro_mode==PRO_PERSPECTIVE){
		glMatrixMode(GL_PROJECTION); 
		glLoadIdentity(); 
		gluPerspective(60, 1, 0.1, 100); 
		glTranslatef(trans_xy[0],trans_xy[1],trans_z);
	}

	else if(pro_mode==PRO_ORTHOGONAL){
		glMatrixMode(GL_PROJECTION); 
		glLoadIdentity(); 
		glOrtho(-20, 20, -20, 20,-50,50);
		glTranslatef(trans_xy[0],trans_xy[1],trans_z);
	}	

	glMatrixMode(GL_MODELVIEW);    
	glLoadIdentity(); 
	gluLookAt(0,5,20,0,0,0,0,1,0); 
	//Material
	GLfloat mSpecular[4]={0.297254, 0.30829, 0.306678, 0.8};
	GLfloat mDiffuse[4]={0.396, 0.74151, 0.69102, 0.8};
	GLfloat mAmbient[4]={0.1, 0.18725, 0.1745, 0.8};
	GLfloat mSininess=12.8;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mSininess);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mAmbient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mSpecular);
	//Lighting
	GLfloat lSpecular[4]={0.297254, 0.30829, 0.306678, 0.8};
	GLfloat lDiffuse[4]={0.396, 0.74151, 0.69102, 0.8};
	GLfloat lAmbient[4]={0.1, 0.18725, 0.1745, 0.8};
	GLfloat lightPos[4] = {5,5,5,0}; 
	glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	//rotate
	glRotatef(x_angle,0,1,0); 
	glRotatef(y_angle,1,0,0); 
	//scale
	glScalef(scale_size,scale_size,scale_size);
	
	

	//ground, coordinate axes and bounding box display
	if(ground==1){
		glColor3f(1.0,1.0,1.0);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		for(int i=-20;i<=20;i++){
			glVertex3f(-20,0,i);
			glVertex3f(20,0,i);
		}
		for(int i=-20;i<=20;i++){
			glVertex3f(i,0,-20);
			glVertex3f(i,0,20);
		}	
		glEnd();
	}
	if(axes==1){	
		glLineWidth(3.0);
		glBegin(GL_LINES);
		glColor3f(0.0,1.0,0.0);
		glVertex3f(0,0,0);
		glVertex3f(0,5,0);
		glColor3f(1.0,0.0,0.0);
		glVertex3f(0,0,0);
		glVertex3f(5,0,0);
		glColor3f(0.0,0.0,1.0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,5);
		glEnd();
	}
	//Translate the model to the center and scale it to a proper size
	glScalef(10/(xmax-xmin),10/(xmax-xmin),10/(xmax-xmin));
	glTranslatef(-(xmax+xmin)/2,-(ymax+ymin)/2,-(zmax+zmin)/2);
	if(box==1){
		glColor3f(0,0,0);
		glLineWidth(1.0);
		glBegin(GL_LINES);	
		glVertex3f(xmax,ymax,zmax);
		glVertex3f(xmin,ymax,zmax);
		glVertex3f(xmax,ymin,zmax);
		glVertex3f(xmin,ymin,zmax);
		glVertex3f(xmax,ymax,zmin);
		glVertex3f(xmin,ymax,zmin);
		glVertex3f(xmax,ymin,zmin);
		glVertex3f(xmin,ymin,zmin);

		glVertex3f(xmax,ymin,zmax);
		glVertex3f(xmax,ymax,zmax);
		glVertex3f(xmin,ymin,zmax);
		glVertex3f(xmin,ymax,zmax);
		glVertex3f(xmax,ymin,zmin);
		glVertex3f(xmax,ymax,zmin);
		glVertex3f(xmin,ymin,zmin);
		glVertex3f(xmin,ymax,zmin);

		glVertex3f(xmax,ymax,zmin);
		glVertex3f(xmax,ymax,zmax);
		glVertex3f(xmin,ymax,zmin);
		glVertex3f(xmin,ymax,zmax);
		glVertex3f(xmax,ymin,zmin);
		glVertex3f(xmax,ymin,zmax);
		glVertex3f(xmin,ymin,zmin);
		glVertex3f(xmin,ymin,zmax);
		glEnd();
	}
	//object display
	glColor3f(red,green,blue);
	if(obj_mode==OBJ_WIREFRAME){	
		glLineWidth(1.0);	
		for(int i=0;i<svf.size();i++){
			glBegin(GL_LINE_LOOP);
			glVertex3f(svv[svf[i].v1].x,svv[svf[i].v1].y,svv[svf[i].v1].z);
			glVertex3f(svv[svf[i].v2].x,svv[svf[i].v2].y,svv[svf[i].v2].z);
			glVertex3f(svv[svf[i].v3].x,svv[svf[i].v3].y,svv[svf[i].v3].z);
			glEnd();	
		}	
	}
	if(obj_mode==OBJ_POINT){
		glPointSize(2.0);	
		glBegin(GL_POINTS);
		for(int i=0;i<svv.size();i++){
			glVertex3f(svv[i].x,svv[i].y,svv[i].z);
		}
		glEnd();
	}
	if(diffuse==1){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}
	if(specular==1){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT1);
	}
	if(obj_mode==OBJ_FLAT){	
		for(int i=0;i<svf.size();i++){
			glShadeModel(GL_FLAT);
			glBegin(GL_TRIANGLES);
			glNormal3f(svv[svf[i].v1].nx,svv[svf[i].v1].ny,svv[svf[i].v1].nz);
			glVertex3f(svv[svf[i].v1].x,svv[svf[i].v1].y,svv[svf[i].v1].z);
			glNormal3f(svv[svf[i].v2].nx,svv[svf[i].v2].ny,svv[svf[i].v2].nz);
			glVertex3f(svv[svf[i].v2].x,svv[svf[i].v2].y,svv[svf[i].v2].z);
			glNormal3f(svv[svf[i].v3].nx,svv[svf[i].v3].ny,svv[svf[i].v3].nz);
			glVertex3f(svv[svf[i].v3].x,svv[svf[i].v3].y,svv[svf[i].v3].z);
			glEnd();	
		}	
	}
	if(obj_mode==OBJ_SMOOTH){
		for(int i=0;i<svf.size();i++){
			glShadeModel(GL_SMOOTH);
			glBegin(GL_TRIANGLES);
			glNormal3f(svv[svf[i].v1].nx,svv[svf[i].v1].ny,svv[svf[i].v1].nz);
			glVertex3f(svv[svf[i].v1].x,svv[svf[i].v1].y,svv[svf[i].v1].z);
			glNormal3f(svv[svf[i].v2].nx,svv[svf[i].v2].ny,svv[svf[i].v2].nz);
			glVertex3f(svv[svf[i].v2].x,svv[svf[i].v2].y,svv[svf[i].v2].z);
			glNormal3f(svv[svf[i].v3].nx,svv[svf[i].v3].ny,svv[svf[i].v3].nz);
			glVertex3f(svv[svf[i].v3].x,svv[svf[i].v3].y,svv[svf[i].v3].z);			
			glEnd();	
		}		
	}
	glutSwapBuffers();
	glutPostRedisplay();
}
void mymouse(int button, int state, int x, int y)
{
	if (state==GLUT_DOWN){
		press_x=x; press_y=y; 
		if(button==GLUT_LEFT_BUTTON)
			xform_mode=TRANSFORM_ROTATE; 
		else if(button==GLUT_RIGHT_BUTTON) 
			xform_mode=TRANSFORM_SCALE; 
		else if(button==GLUT_MIDDLE_BUTTON)
			xform_mode=TRANSFORM_MODE;
	}
	else if(state==GLUT_UP){
		xform_mode=TRANSFORM_NONE; 
	}
}
void mymotion(int x, int y)
{
	if(xform_mode==TRANSFORM_ROTATE){
		x_angle+=(x-press_x)/5.0;
		if(x_angle>180) 
			x_angle-=360; 
		else if(x_angle<-180) 
			x_angle+=360; 
		press_x=x; 
		y_angle+=(y-press_y)/5.0; 
		if(y_angle>180) 
			y_angle-=360; 
		else if(y_angle<-180) 
			y_angle+=360; 
		press_y=y; 
	}
	else if(xform_mode==TRANSFORM_SCALE){
		float old_size=scale_size;
		scale_size*=(1+(y-press_y)/60.0); 
		if(scale_size<0) 
			scale_size=old_size; 
		press_y=y; 
	}
	else if(xform_mode==TRANSFORM_MODE){
		trans_xy[0]+=0.1*(x-press_x)/scale_size;
		trans_xy[1]-=0.1*(y-press_y)/scale_size;
		press_x=x;
		press_y=y;
	}
	// force the redraw function
	glutPostRedisplay(); 
}
void mykey(unsigned char key, int x, int y)
{
	switch(key) 
	{
	case 'w': 
		cout << "wireframe mode" << endl;
		obj_mode = OBJ_WIREFRAME;
		break; 
	case 'p':
		cout << "point mode" << endl;
		obj_mode = OBJ_POINT;
		break;
	case 'f':
		cout << "flat mode" << endl;
		obj_mode = OBJ_FLAT;
		break;
	case 's':
		cout << "smooth mode" << endl;
		obj_mode = OBJ_SMOOTH;
		break;
	case 'g':
		if(ground==false) ground=true;
		else ground=false;
		break;
	case 'a':
		if(axes==false) axes=true;
		else axes=false;
		break;
	case 'b':
		if(box==false) box=true;
		else box=false;
		break;
	}
	// force the redraw function
	glutPostRedisplay(); 
}
void control_cb(int control){
	switch(control){
	case CB_READFILE:
		strcpy(filename,fb->get_file());
		if(filename[strlen(filename)-1]!='m'||filename[strlen(filename)-2]!='.') return;
		readFile(filename);
		break;
	}
	glutPostRedisplay();
}
class SampleListener : public Listener {
public:
	virtual void onFrame(const Controller&);
};
void SampleListener::onFrame(const Controller& controller) {

	Leap::Frame frame = controller.frame();
	if(frame.id()%10) return;
	Leap::Frame preframe = controller.frame(10);	
	if(frame.hands().count()==1&&preframe.hands().count()==1)
	{	
		int curExtFin=0;
		int preExtFin=0;
		for(int i=0;i<frame.fingers().count();i++)
		{
			if(frame.fingers()[i].isExtended()) curExtFin++;
		}
		for(int i=0;i<preframe.fingers().count();i++)
		{
			if(preframe.fingers()[i].isExtended()) preExtFin++;
		}
		if(curExtFin>0&&preExtFin>0)
		{
			Leap::Vector mvector = frame.hands()[0].palmPosition()-preframe.hands()[0].palmPosition();
			x_angle+=mvector.x/5.0;
			if(x_angle>180) 
				x_angle-=360; 
			else if(x_angle<-180) 
				x_angle+=360;
			y_angle+=-mvector.y/5.0; 
			if(y_angle>180) 
				y_angle-=360; 
			else if(y_angle<-180) 
				y_angle+=360; 
		}		
	}
	else if(frame.hands().count()==2&&preframe.hands().count()==2)
	{
		int curExtFinR=0;
		int preExtFinR=0;
		int curExtFinL=0;
		int preExtFinL=0;
		int curR,preR,curL,preL;
		if(frame.hands()[1].isRight())
		{
			curR=1;
			curL=0;
		}
		else if(frame.hands()[1].isLeft())
		{
			curR=0;
			curL=1;
		}
		if(preframe.hands()[1].isRight())
		{
			preR=1;
			preL=0;
		}
		else if(preframe.hands()[1].isLeft())
		{
			preR=0;
			preL=1;
		}
		for(int i=0;i<frame.hands()[curR].fingers().count();i++)
		{
			if(frame.hands()[curR].fingers()[i].isExtended()) curExtFinR++;
		}
		
		for(int i=0;i<preframe.hands()[preR].fingers().count();i++)
		{
			if(preframe.hands()[preR].fingers()[i].isExtended()) preExtFinR++;
		}
		for(int i=0;i<frame.hands()[curL].fingers().count();i++)
		{
			if(frame.hands()[curL].fingers()[i].isExtended()) curExtFinL++;
		}
		for(int i=0;i<preframe.hands()[preL].fingers().count();i++)
		{
			if(preframe.hands()[preL].fingers()[i].isExtended()) preExtFinL++;
		}
		if(curExtFinL>0&&preExtFinL>0&&curExtFinR>0&&curExtFinR>0)
		{
			Leap::Vector mvector1=frame.hands()[curR].palmPosition()-frame.hands()[curL].palmPosition();
			Leap::Vector mvector2=preframe.hands()[preR].palmPosition()-preframe.hands()[preL].palmPosition();
			scale_size*=(1+((mvector1.x)-(mvector2.x))/100.0);
		}
		else if(curExtFinL==0&&curExtFinR>0&&preExtFinL==0&&preExtFinR>0)
		{
			Leap::Vector mvector=frame.hands()[curR].palmPosition()-preframe.hands()[preR].palmPosition();
			trans_xy[0]+=0.1*mvector.x/scale_size;
			trans_xy[1]+=0.1*mvector.y/scale_size;
		}
	}
}

int main(int argc, char ** argv){
	SampleListener listener;
	Controller controller;
	controller.addListener(listener);
	int window;
	GLUI * glui_window;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1000,700);
	glutInitWindowPosition(100,100);
	window=glutCreateWindow("3D mesh viewer(Yang Xiangting)");
	glClearColor(0.5,0.5,0.5,1);
	glutDisplayFunc(display);
	glutMouseFunc(mymouse);
	glutMotionFunc(mymotion);
	glutKeyboardFunc(mykey);

	glui_window = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_RIGHT);
	GLUI_Panel *op_panel=glui_window->add_panel ("Open file");
	fb = new GLUI_FileBrowser(op_panel,"", false, CB_READFILE, control_cb);

	GLUI_Panel *re_panel=glui_window->add_panel ("Rendering");	
	glui_window->add_checkbox_to_panel (re_panel, "show ground",&ground);
	glui_window->add_checkbox_to_panel (re_panel, "show axes", &axes);
	glui_window->add_checkbox_to_panel (re_panel, "show bounding box", &box );
	GLUI_Listbox *li_mode=glui_window->add_listbox_to_panel (re_panel, "display mode",&obj_mode);
	li_mode->add_item (OBJ_SMOOTH, "smooth");
	li_mode->add_item (OBJ_FLAT, "flat");
	li_mode->add_item (OBJ_POINT, "point");
	li_mode->add_item (OBJ_WIREFRAME, "wire frame");

	GLUI_Panel *co_panel=glui_window->add_panel_to_panel (re_panel,"Color");	
	GLUI_Spinner *sp_red=glui_window->add_spinner_to_panel(co_panel,"R",GLUI_SPINNER_FLOAT,&red);
	sp_red->set_float_limits (0,1);
	GLUI_Spinner *sp_green=glui_window->add_spinner_to_panel(co_panel,"G",GLUI_SPINNER_FLOAT,&green);
	sp_green->set_float_limits (0,1);
	GLUI_Spinner *sp_blue=glui_window->add_spinner_to_panel(co_panel,"B",GLUI_SPINNER_FLOAT,&blue);
	sp_blue->set_float_limits (0,1);

	GLUI_Panel *li_panel=glui_window->add_panel_to_panel (re_panel,"Lightning");
	glui_window->add_checkbox_to_panel (li_panel, "diffuse light", &diffuse);
	glui_window->add_checkbox_to_panel (li_panel,"specular light", &specular );

	GLUI_Panel *pr_panel=glui_window->add_panel_to_panel(re_panel,"Projection");
	GLUI_RadioGroup *ra_projection=glui_window->add_radiogroup_to_panel(pr_panel,&pro_mode);
	glui_window->add_radiobutton_to_group(ra_projection, "perspective" );
	glui_window->add_radiobutton_to_group(ra_projection, "orthogonal" );


	glutMainLoop();
	controller.removeListener(listener);
	return 1;
}