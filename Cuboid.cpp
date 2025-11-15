
#include <iostream>
#include <math.h>
#include <string.h>
#include "Cuboid.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ZERO 0.0000000001
#define COLLISION 0.0
#define CLOSEST(a,b,c) (a<b&&a<c)?a:(b<c)?b:c
#define PI 3.1415926535897932384626433832795

const double PI_OVER_180 = PI / 180;

typedef struct
{
   C_vector point;
   C_vector normal;
} tPlane;

/*****************************
 * PRIVATE UTILITY FUNCTIONS *
 *****************************/

void MatrixMultiply( double m1[3][3], double m2[3][3] )
{
   int i, j, k;
   double result[3][3] = { 0.0, 0.0, 0.0,
                           0.0, 0.0, 0.0,
                           0.0, 0.0, 0.0 };

   for( i = 0; i < 3; i++ )
      for( j = 0; j < 3; j++ )
         for( k = 0; k < 3; k++ )
            result[i][j] += m1[i][k] * m2[k][j];

   memcpy( &m1[0][0], &result[0][0], 9 * sizeof( double ) );
}

void Identity( double ppMatrix[3][3] )
{
   ppMatrix[0][0] = 1.0; ppMatrix[0][1] = 0.0; ppMatrix[0][2] = 0.0; // X component of axis vectors
   ppMatrix[1][0] = 0.0; ppMatrix[1][1] = 1.0; ppMatrix[1][2] = 0.0; // Y component of axis vectors
   ppMatrix[2][0] = 0.0; ppMatrix[2][1] = 0.0; ppMatrix[2][2] = 1.0; // Z component of axis vectors
}

double SphereToPlaneCollision( tPlane plane, C_vector sphere_pos, double sphere_rad, C_vector &poc )
{
   C_vector v_sp2pp; // Vector from Sphere Position to Plane Point
   double   d_sp2cp; // Distance from Sphere Position to Closest Point on plane
   C_vector v_sp2cp; // Vector from Sphere Position to Closest Point on plane

   v_sp2pp = plane.point - sphere_pos;
   d_sp2cp = plane.normal * v_sp2pp;
   v_sp2cp = d_sp2cp * plane.normal;
   poc     = sphere_pos + v_sp2cp;

   if( fabs( d_sp2cp ) <= sphere_rad )
      return COLLISION;

   return fabs( d_sp2cp ) - sphere_rad;
}

bool LinePlaneCollision( tPlane plane, C_vector a, C_vector b, C_vector &pp )
{
   C_vector ba;
   double   ndota;
   double   ndotba;
   double   d;
   double   t;

   ba = b - a;

   // check if line is parallel
   ndotba = plane.normal * ba;

   if (fabs(ndotba) < ZERO) return false;

   ndota = plane.normal * a;

   d = plane.normal * plane.point;

   t = (d - ndota)/ndotba;

   pp = a + (t * ba);

   // limit collision returns to line segment (center of sphere to
   // center of cuboid)
   if (t >= 0.0 and t <= 1.0) return true;

   return false;
}

bool PointInBounds( C_vector point, C_vector rect[4] )
{
   int i;
   double minX, maxX;
   double minY, maxY;
   double minZ, maxZ;

   // Set point 1 as the default values for min and max
   minX = maxX = rect[0].x();
   minY = maxY = rect[0].y();
   minZ = maxZ = rect[0].z();

   for( i = 1; i < 4; i++ )
   {
      // Min/Max X
      if( rect[i].x() < minX ) minX = rect[i].x(); else
      if( rect[i].x() > maxX ) maxX = rect[i].x();

      // Min/Max Y
      if( rect[i].y() < minY ) minY = rect[i].y(); else
      if( rect[i].y() > maxY ) maxY = rect[i].y();

      // Min/Max Z
      if( rect[i].z() < minZ ) minZ = rect[i].z(); else
      if( rect[i].z() > maxZ ) maxZ = rect[i].z();
   }

   // Check if the point is outside any of the bounds
   if( point.x() < minX || point.x() > maxX )
      return false;
   if( point.y() < minY || point.y() > maxY )
      return false;
   if( point.z() < minZ || point.z() > maxZ )
      return false;

   // Point is inside the bounds
   return true;
}

/****************
 * CONSTRUCTORS *
 ****************/

C_cuboid::C_cuboid( void )
{
   m_vPosition = C_vector( 0.0 );
   m_pSize[WIDTH] = m_pSize[HEIGHT] = m_pSize[DEPTH] = 1.0;
   Identity( m_pOrientation );
}

C_cuboid::C_cuboid( C_vector &c )
{
   m_vPosition = c;
   m_pSize[WIDTH] = m_pSize[HEIGHT] = m_pSize[DEPTH] = 1.0;
   Identity( m_pOrientation );
}

C_cuboid::C_cuboid( double s )
{
   m_vPosition = C_vector( 0.0 );
   m_pSize[WIDTH] = m_pSize[HEIGHT] = m_pSize[DEPTH] = s;
   Identity( m_pOrientation );
}

C_cuboid::C_cuboid( double w, double h, double d )
{
   m_vPosition     = C_vector( 0.0 );
   m_pSize[WIDTH]  = w;
   m_pSize[HEIGHT] = h;
   m_pSize[DEPTH]  = d;
   Identity( m_pOrientation );
}

C_cuboid::C_cuboid( C_vector &c, double s )
{
   m_vPosition = c;
   m_pSize[WIDTH] = m_pSize[HEIGHT] = m_pSize[DEPTH] = s;
   Identity( m_pOrientation );
}

C_cuboid::C_cuboid( C_vector c, double w, double h, double d )
{
   m_vPosition     = c;
   m_pSize[WIDTH]  = w;
   m_pSize[HEIGHT] = h;
   m_pSize[DEPTH]  = d;
   Identity( m_pOrientation );
}

/******************
 * INPUT / OUTPUT *
 ******************/

void C_cuboid::get( void )
{
}

void C_cuboid::put( void )
{
   int i;
   C_vector corners[8];
   C_vector v[3];

   for( i = 0; i < 3; i++ )
      v[i] = C_vector( m_pOrientation[i][X], m_pOrientation[i][Y], m_pOrientation[i][Z] ) * m_pSize[i] * 0.5;

   // Front face
   corners[0] = m_vPosition + v[X] + v[Y] + v[Z]; // Top Right
   v[X] *= -1;
   corners[1] = m_vPosition + v[X] + v[Y] + v[Z]; // Top Left
   v[Y] *= -1;
   corners[2] = m_vPosition + v[X] + v[Y] + v[Z]; // Bottom Left
   v[X] *= -1;
   corners[3] = m_vPosition + v[X] + v[Y] + v[Z]; // Bottom Right

   // Move to back face
   v[Y] *= -1;
   v[Z] *= -1;

   // Back face
   corners[4] = m_vPosition + v[X] + v[Y] + v[Z]; // Top Left
   v[X] *= -1;
   corners[5] = m_vPosition + v[X] + v[Y] + v[Z]; // Top Right
   v[Y] *= -1;
   corners[6] = m_vPosition + v[X] + v[Y] + v[Z]; // Bottom Right
   v[X] *= -1;
   corners[7] = m_vPosition + v[X] + v[Y] + v[Z]; // Bottom Left

   // Print
   for( i = 0; i < 8; i++ )
   {
      printf( "Corner %d = %8.4f, %8.4f, %8.4f\n", i, corners[i].x(), corners[i].y(), corners[i].z() );
   }
}

/*************
 * ACCESSORS *
 *************/

C_vector C_cuboid::Position( void )
{
   return m_vPosition;
}

double C_cuboid::Width( void )
{
   return m_pSize[WIDTH];
}

double C_cuboid::Height( void )
{
   return m_pSize[HEIGHT];
}

double C_cuboid::Depth( void )
{
   return m_pSize[DEPTH];
}

/*************
 * MODIFIERS *
 *************/

void C_cuboid::SetPosition( C_vector c )
{
   m_vPosition = c;
}

void C_cuboid::SetPosition( double x, double y, double z )
{
   m_vPosition = C_vector( x, y, z );
}

void C_cuboid::operator +=( C_vector &v )
{
   m_vPosition += v;
}

C_cuboid operator +( C_cuboid &c, C_vector &v )
{
   C_cuboid tmp = c;

   tmp.SetPosition( c.Position() + v );

   return tmp;
}

void C_cuboid::SetHeight( double h )
{
   m_pSize[HEIGHT] = h;
}

void C_cuboid::SetWidth( double w )
{
   m_pSize[WIDTH] = w;
}

void C_cuboid::SetDepth( double d )
{
   m_pSize[DEPTH] = d;
}

void C_cuboid::scale( double s )
{
   for( int i = 0; i < 3; i++ )
      m_pSize[i] *= s;
}

void C_cuboid::operator *=( double s )
{
   scale( s );
}

C_cuboid operator *( C_cuboid &c, double s )
{
   C_cuboid tmp = c;

   tmp.scale( s );

   return tmp;
}

void C_cuboid::Yaw_D( double z )
{
   Yaw( z * PI_OVER_180 );
}

void C_cuboid::Yaw( double z )
{
   double rm[3][3]; // Rotation Matrix
   double sn  = sin( z );
   double csn = cos( z );
   double nsn = -1 * sn;

   rm[0][0] = csn; rm[0][1] = nsn; rm[0][2] = 0.0;
   rm[1][0] = sn;  rm[1][1] = csn; rm[1][2] = 0.0;
   rm[2][0] = 0.0; rm[2][1] = 0.0; rm[2][2] = 1.0;
   

   MatrixMultiply( m_pOrientation, rm );
}

void C_cuboid::Pitch_D( double y )
{
   Pitch( y * PI_OVER_180 );
}

void C_cuboid::Pitch( double y )
{
   double rm[3][3]; // Rotation Matrix
   double sn  = sin( y );
   double csn = cos( y );
   double nsn = -1 * sn;

   rm[0][0] = csn; rm[0][1] = 0.0; rm[0][2] = sn;
   rm[1][0] = 0.0; rm[1][1] = 1.0; rm[1][2] = 0.0;
   rm[2][0] = nsn; rm[2][1] = 0.0; rm[2][2] = csn;

   MatrixMultiply( m_pOrientation, rm );
}

void C_cuboid::Roll_D( double x )
{
   Roll( x * PI_OVER_180 );
}

void C_cuboid::Roll( double x )
{
   double rm[3][3]; // Rotation Matrix
   double sn  = sin( x );
   double csn = cos( x );
   double nsn = -1 * sn;

   rm[0][0] = 1.0; rm[0][1] = 0.0; rm[0][2] = 0.0;
   rm[1][0] = 0.0; rm[1][1] = csn; rm[1][2] = nsn;
   rm[2][0] = 0.0; rm[2][1] = sn;  rm[2][2] = csn;

   MatrixMultiply( m_pOrientation, rm );
}

void C_cuboid::SetYaw_D( double z ) // Degrees
{
   m_Yaw = z;
   SetYaw( z * PI_OVER_180 );
}

void C_cuboid::SetYaw  ( double z ) // Radians
{
   m_pOrientation[0][0] = 1.0; m_pOrientation[0][1] = 0.0;
   m_pOrientation[1][0] = 0.0; m_pOrientation[1][1] = 1.0;
   Yaw( z );
}

void C_cuboid::SetPitch_D( double y ) // Degrees
{
   SetPitch( y * PI_OVER_180 );
}

void C_cuboid::SetPitch  ( double y ) // Radians
{
   m_pOrientation[0][0] = 1.0; m_pOrientation[0][2] = 0.0;
   m_pOrientation[2][0] = 0.0; m_pOrientation[2][2] = 1.0;
   Pitch( y );
}

void C_cuboid::SetRoll_D( double x ) // Degrees
{
   SetRoll( x * PI_OVER_180 );
}

void C_cuboid::SetRoll  ( double x ) // Radians
{
   m_pOrientation[1][1] = 1.0; m_pOrientation[1][2] = 0.0;
   m_pOrientation[2][1] = 0.0; m_pOrientation[2][2] = 1.0;
   Roll( x );
}

/***********************
 * COLLISION DETECTION *
 ***********************/

int C_cuboid::SphereCollision( const C_vector &pos, double rad, double& miss_distance, C_vector& poc)
{
   double   s2p_mag;
   C_vector s2p;
   C_vector p[3];
   C_vector v[2];
   C_vector cuboid_center;
   C_vector sphere;
   tPlane   plane;

   int face = -1;

   cuboid_center = m_vPosition;
   //cuboid_center.set_z(-cuboid_center.z());

   sphere = pos;
   //sphere.set_z(-sphere.z());

   // Use cuboid as center at origin
   C_vector trans_pos = sphere - cuboid_center;

   // Rotate translated position
   C_vector new_pos = C_vector( m_pOrientation[0][0] * trans_pos.x() + m_pOrientation[0][1] * trans_pos.y() + m_pOrientation[0][2] * trans_pos.z(),
                                m_pOrientation[1][0] * trans_pos.x() + m_pOrientation[1][1] * trans_pos.y() + m_pOrientation[1][2] * trans_pos.z(),
                                m_pOrientation[2][0] * trans_pos.x() + m_pOrientation[2][1] * trans_pos.y() + m_pOrientation[2][2] * trans_pos.z());

   /////////////////////////////////////////////////////////////////////
   // Front face (x-north, y-east, z-up)
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.y() >= p[2].y() && poc.y() <= p[0].y() &&
          poc.z() >= p[2].z() && poc.z() <= p[0].z())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 1;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;

         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Right face
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.x() >= p[2].x() && poc.x() <= p[0].x() &&
          poc.z() >= p[2].z() && poc.z() <= p[0].z())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 2;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;

         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Top face
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.x() >= p[2].x() && poc.x() <= p[0].x() &&
          poc.y() >= p[2].y() && poc.y() <= p[0].y())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 3;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;

         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Left face
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.x() >= p[2].x() && poc.x() <= p[0].x() &&
          poc.z() >= p[2].z() && poc.z() <= p[0].z())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 4;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;

         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Bottom face
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.x() >= p[2].x() && poc.x() <= p[0].x() &&
          poc.y() >= p[2].y() && poc.y() <= p[0].y())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 5;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;
      
         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   /////////////////////////////////////////////////////////////////////
   // Back face
   /////////////////////////////////////////////////////////////////////

   p[0] = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5);
   p[1] = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);
   p[2] = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5);

   // Build a plane out of the triangle
   v[0] = p[1] - p[0];
   v[1] = p[2] - p[0];
   plane.normal = unit( cross( v[0], v[1] ) );
   plane.point  = p[0];

   if (LinePlaneCollision(plane, new_pos, C_vector(0.0), poc))
   {
      // Is this point inside the bounds of the face?
      if (poc.y() >= p[2].y() && poc.y() <= p[0].y() &&
          poc.z() >= p[2].z() && poc.z() <= p[0].z())
      {
         // Calculate how far the sphere center is from the intersection
         // on the face
         s2p = poc - new_pos;

         // Check if sphere surface is colliding with the face
         s2p_mag = abs(s2p) - rad;

         face = 6;

         // only collide with one face, return collision or distance
         if( s2p_mag < ZERO )
            miss_distance = COLLISION;
         else
            miss_distance = s2p_mag;

         poc = poc + m_vPosition;

         glm::mat4 model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
         model = glm::inverse(model);

         poc = C_vector( model[0][0] * poc.x() + model[0][1] * poc.y() + model[0][2] * poc.z(),
                         model[1][0] * poc.x() + model[1][1] * poc.y() + model[1][2] * poc.z(),
                         model[2][0] * poc.x() + model[2][1] * poc.y() + model[2][2] * poc.z());

         return face;
      }
   }

   // If we get this far, then the sphere center didn't collide with any
   // face, so the sphere center is inside cuboid... return collision
   miss_distance = COLLISION;
   return -1;
}

int C_cuboid::SphereCollisionOld( const C_vector &pos, double rad, double& miss_distance, C_vector& poc)
{
   int i;
   double s2p_mag;
   C_vector s2p;
   C_vector v1;
   C_vector v2;
   C_vector faces[24];
   C_vector axis[3];
   tPlane plane;

   int face = -1;

   // Create the x, y and z axis
   for( i = 0; i < 3; i++ )
      axis[i] = C_vector( m_pOrientation[i][X], m_pOrientation[i][Y], m_pOrientation[i][Z] ) * m_pSize[i];

   // Find the position of the top right corner of the front face
   faces[0] = m_vPosition;
   for( i = 0; i < 3; i++ )
      faces[0] += (axis[i] * 0.5);

   // Build the rectangles that make up each face
#if XOUT_YLEFT_ZDOWN
   // Front face
   faces[1]  = faces[0]  + (axis[Y] * -1);
   faces[2]  = faces[1]  + (axis[Z] * -1);
   faces[3]  = faces[2]  + axis[Y];
   // Right face
   faces[4]  = faces[3];
   faces[5]  = faces[4]  + (axis[X]  * -1);
   faces[6]  = faces[5]  + axis[Z];
   faces[7]  = faces[6]  + axis[X];
   // Top face
   faces[8]  = faces[7];
   faces[9]  = faces[8]  + (axis[X]  * -1);
   faces[10] = faces[9]  + (axis[Y]  * -1);
   faces[11] = faces[10] + axis[X];
   // Left face
   faces[12] = faces[11];
   faces[13] = faces[12] + (axis[X]  * -1);
   faces[14] = faces[13] + (axis[Z] * -1);
   faces[15] = faces[14] + axis[X];
   // Bottom face
   faces[16] = faces[15];
   faces[17] = faces[16] + (axis[X]  * -1);
   faces[18] = faces[17] + axis[Y];
   faces[19] = faces[18] + axis[X];
   // Back face
   faces[20] = faces[0]  + (axis[X] * -1);
   faces[21] = faces[20] + (axis[Z] * -1);
   faces[22] = faces[21] + (axis[Y]  * -1);
   faces[23] = faces[22] + axis[Z];
#else
   // Front face
   faces[1]  = faces[0]  + (axis[X]  * -1);
   faces[2]  = faces[1]  + (axis[Y] * -1);
   faces[3]  = faces[2]  + axis[X];
   // Right face
   faces[4]  = faces[3];
   faces[5]  = faces[4]  + (axis[Z]  * -1);
   faces[6]  = faces[5]  + axis[Y];
   faces[7]  = faces[6]  + axis[Z];
   // Top face
   faces[8]  = faces[7];
   faces[9]  = faces[8]  + (axis[Z]  * -1);
   faces[10] = faces[9]  + (axis[X]  * -1);
   faces[11] = faces[10] + axis[Z];
   // Left face
   faces[12] = faces[11];
   faces[13] = faces[12] + (axis[Z]  * -1);
   faces[14] = faces[13] + (axis[Y] * -1);
   faces[15] = faces[14] + axis[Z];
   // Bottom face
   faces[16] = faces[15];
   faces[17] = faces[16] + (axis[Z]  * -1);
   faces[18] = faces[17] + axis[X];
   faces[19] = faces[18] + axis[Z];
   // Back face
   faces[20] = faces[0]  + (axis[Z] * -1);
   faces[21] = faces[20] + (axis[Y] * -1);
   faces[22] = faces[21] + (axis[X]  * -1);
   faces[23] = faces[22] + axis[Y];
#endif

   // For each face ...
   for( i = 0; i < 24; i+=4 )
   {
      // Build a plane out of the rectangle
      v1 = faces[i+1] - faces[i];
      v2 = faces[i+3] - faces[i];
      plane.normal = unit( cross( v1, v2 ) );
      plane.point  = faces[i];

      // Check if the line between the cuboid center and the sphere center
      // collides with the plane
      if (LinePlaneCollision( plane, pos, m_vPosition, poc ))
      {
         // Is this point inside the bounds of the face?
         if( PointInBounds( poc, faces+i ) )
         {
            // Calculate how far the sphere center is from the intersection
            // on the face
            s2p = poc - pos;

            // Check if sphere surface is colliding with the face
            s2p_mag = abs(s2p) - rad;

            // only collide with one face, return collision or distance
            if( s2p_mag < ZERO )
               miss_distance = COLLISION;
            else
               miss_distance = s2p_mag;

            face = (i/4) + 1;

            return face;
         }
      }
   }

   // If we get this far, then the sphere center didn't collide with any
   // face, so the sphere center is inside cuboid... return collision
   miss_distance = COLLISION;
   return face;
}

void C_cuboid::GetFaceCorners(int Face, C_vector& C1, C_vector& C2, C_vector& C3, C_vector& C4)
{
   if (Face == -1)
   {
      C1 = C_vector(0.0);
      C2 = C_vector(0.0);
      C3 = C_vector(0.0);
      C4 = C_vector(0.0);
      return;
   }

   C_vector c1;
   C_vector c2;
   C_vector c3;
   C_vector c4;

   if (Face == 1)
   {
      // Front face
      c1 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;

   }
   else if (Face == 2)
   {
      // Right face
      c1 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
   }
   else if (Face == 3)
   {
      // Top face
      c1 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
   }
   else if (Face == 4)
   {
      // Left face
      c1 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
   }
   else if (Face == 5)
   {
      // Bottom face
      c1 = C_vector(  m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector(  m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
   }
   else if (Face == 6)
   {
      // Back face
      c1 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c2 = C_vector( -m_pSize[DEPTH] * 0.5,  m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c3 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5, -m_pSize[HEIGHT] * 0.5) + m_vPosition;
      c4 = C_vector( -m_pSize[DEPTH] * 0.5, -m_pSize[WIDTH] * 0.5,  m_pSize[HEIGHT] * 0.5) + m_vPosition;
   }

   glm::mat4 model = glm::mat4(1.0f);
   model = glm::rotate(model, glm::radians((float)-m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
   model = glm::inverse(model);

   C1 = C_vector( model[0][0] * c1.x() + model[0][1] * c1.y() + model[0][2] * c1.z(),
                  model[1][0] * c1.x() + model[1][1] * c1.y() + model[1][2] * c1.z(),
                  model[2][0] * c1.x() + model[2][1] * c1.y() + model[2][2] * c1.z());

   C2 = C_vector( model[0][0] * c2.x() + model[0][1] * c2.y() + model[0][2] * c2.z(),
                  model[1][0] * c2.x() + model[1][1] * c2.y() + model[1][2] * c2.z(),
                  model[2][0] * c2.x() + model[2][1] * c2.y() + model[2][2] * c2.z());

   C3 = C_vector( model[0][0] * c3.x() + model[0][1] * c3.y() + model[0][2] * c3.z(),
                  model[1][0] * c3.x() + model[1][1] * c3.y() + model[1][2] * c3.z(),
                  model[2][0] * c3.x() + model[2][1] * c3.y() + model[2][2] * c3.z());

   C4 = C_vector( model[0][0] * c4.x() + model[0][1] * c4.y() + model[0][2] * c4.z(),
                  model[1][0] * c4.x() + model[1][1] * c4.y() + model[1][2] * c4.z(),
                  model[2][0] * c4.x() + model[2][1] * c4.y() + model[2][2] * c4.z());
}